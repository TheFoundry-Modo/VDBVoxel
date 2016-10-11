/*
*	Copyright 2015 The Foundry Visionmongers Ltd.
*
*	Licensed under the Apache License, Version 2.0 (the "License");
*	you may not use this file except in compliance with the License.
* 	You may obtain a copy of the License at
*
*		http://www.apache.org/licenses/LICENSE-2.0
*
*  	Neither the name of The Foundry nor the names of
* 	its contributors may be used to endorse or promote products derived
*	from this software without specific prior written permission.
*
* 	Unless required by applicable law or agreed to in writing, software
* 	distributed under the License is distributed on an "AS IS" BASIS,
*	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*	See the License for the specific language governing permissions and
*	limitations under the License.
*/

#include "OpenVDBCreation.h"
#include <lx_action.hpp>
#include <lx_mesh.hpp>
#include <lxidef.h>
#include <fstream>
#include "ParticleAttach.h"
#include "VDBVoxelHash.h"

/*
*	64M min mount of cache, if caching is used.
*/

#define CacheBudgetMin	67108864

	VoxelItemKey
OpenVDBCreator::GenKey (
	const VDBVoxelChannels	*creationInfo)
{
	VoxelItemKey keys;
	unsigned key = VDBVoxelFNV::ComputeHashCode (*creationInfo, false);
	keys.base = key;

	return keys;
}

OpenVDBCreateFromMesh::OpenVDBCreateFromMesh (
	CLxUser_Item		 sourceItem)
{
	this->m_srcitem.set (sourceItem);
}

OpenVDBCreateFromFile::OpenVDBCreateFromFile (
	CLxUser_Item		 item)
{
	this->m_item.set (item);
}

/*
*	Get a filename from a VDB file sequence.
*	It only supports filename as #####000XXX.
*/
	bool
OpenVDBCreateFromFile::GetCurrentFileName (
	unsigned	 frame,
	unsigned	 start,
	const string	&pattern,
	string		&filename)
{
	using namespace std;

	string currentNum = static_cast<ostringstream*>( &(ostringstream() << (frame + start)) )->str();
	string startNum = static_cast<ostringstream*>( &(ostringstream() << start) )->str();

	unsigned long diff = currentNum.length() - startNum.length();

	unsigned long offset = 0;
	unsigned long len = pattern.length();
	for (offset = 0; offset < diff; offset++) {
		if (pattern[len - 1 - offset] != '0')
			break;
	}
	
	filename = pattern.substr (0, len - offset) + currentNum + string(".vdb");

	return true;
}

/*
*	VDBSeqFindInFilename gets the start number and naming pattern from the input VDB filename.
*	This function must be called, before using GetCurrentFileName.
*/

	bool
VDBSeqFindInFilename (
	unsigned		&startDigit,
	string			&prefix,
	const string		&fileFullName)
{

	if( fileFullName.empty())
		return false;

	size_t	 pos = fileFullName.find_last_of ('/');
	string	 filenameWithExt (fileFullName.substr (pos + 1));
	string	 path (fileFullName.substr (0, pos + 1));

	pos = filenameWithExt.find_last_of('.');
	string	 filename (filenameWithExt.substr (0, pos));

	// record all digits
	pos = 4096;
	for (int i = (int)filename.length() - 1 ; i >= 0; i--)
	{
		if ( filename[i] <= '9' && filename[i] >= '0')
			pos = i;
		else
			break;
	}

	if (pos == 4096)
		return false;
	else {
		string pattern (filename.substr (pos));

		// remove 0 at front
		for (size_t i = 0; i < pattern.length(); i++)
		{
			if (pattern[i] == '0')
				pos ++;
			else
				break;
		}

		pos = min (filename.length()-1, pos);
		startDigit = atoi(pattern.c_str());
		prefix = string (filename.substr (0, pos));
		prefix = path + prefix;
		return true;
	}
}

OpenVDBItem* OpenVDBCreateFromFile::Alloc (
	VDBVoxelChannels	*creationInfo,
	bool			&isValid)
{
	CLxUser_Scene		 scene;
	CLxUser_ChannelRead	 readChan;
	CLxUser_Item		 fxItem;
	CLxItemType		 iType_Scene(LXsITYPE_SCENE);

	scene.from (this->m_item);
	scene.GetChannels (readChan, creationInfo->time); // Time is needed to get deformed meshes
	scene.GetItem(iType_Scene, fxItem);

	string filename, pattern;
	unsigned startDigit;
	// FPS is no need to cache, since its modification requires a restart of modo anyway.
	unsigned frame = readChan.FValue(fxItem, LXsICHAN_SCENE_FPS) * creationInfo->time;

	// Pattern has to be detected no earlier than here, since file names can be changed in different ways.
	if (VDBSeqFindInFilename (startDigit, pattern, creationInfo->cv_fileName)) {

		bool fileExist = false;

		// Loop used for skipping missed sequence files, if a frame is lacking, use the latest frame instead.
		for (int i = frame; i >= 0; i--) {

			GetCurrentFileName (i, startDigit, pattern, filename);

			// test if the file exists;
			ifstream f(filename.c_str ());
			if (f.good ()) {
				f.close ();
				fileExist = true;
				break;
			}
		}
		// when detection fails, we change back to use non-sequential file name;
		if (!fileExist)
			filename = creationInfo->cv_fileName;
	}
	else
		filename = creationInfo->cv_fileName;

	// never create invalid OpenVDBItems
	OpenVDBItem *vdb = NULL;
	ifstream f(filename.c_str ());
	if (f.good ()) {
		f.close ();
		vdb = new OpenVDBItem (filename, isValid);
		vdb->setOptFeatureNames (creationInfo->cv_featureNameList);
	}

	return vdb;
}

	VoxelItemKey
OpenVDBCreateFromFile::GenKey (
	const VDBVoxelChannels	*creationInfo)
{
	// generate hash key with FNV-1
	VoxelItemKey keys;
	unsigned key = VDBVoxelFNV::ComputeHashCode (*creationInfo, true);
	keys.base = key;

	return keys;
}

OpenVDBItem* OpenVDBCreateFromMesh::Alloc (
	VDBVoxelChannels	*creationInfo,
	bool			&isValid)
{
	CLxUser_Mesh		 mesh;
	CLxUser_MeshFilter	 mfilt;
	CLxUser_Scene		 scene;
	CLxUser_ChannelRead	 readChan;
	CLxLoc_Point		 pointAccessor;
	CLxLoc_Polygon		 polygonAccessor;
	CTriangleList		 tri;
	CSourceFinder		 src;
	unsigned		 index;

	if (this->m_srcitem.ChannelLookup (LXsICHAN_MESH_MESH, &index) == LXe_OK) {
		scene.from (this->m_srcitem);
		scene.GetChannels (readChan, creationInfo->time); // Time is needed to get deformed meshes
		if (readChan.Object (this->m_srcitem, index, mfilt)) {
			if (mfilt.GetMesh (mesh)) {

			mesh.GetPoints (pointAccessor);
			mesh.GetPolygons (polygonAccessor);
			unsigned numPolygons = mesh.NPolygons ();
			unsigned numPoints = mesh.NPoints ();

			/*
			*	read polygons, then generate triangles for each of them.
			*	push back to the triangles buffer.
			*/

			for (unsigned i = 0; i < numPolygons; i++) {
				unsigned numTriangles;
				polygonAccessor.SelectByIndex (i);
				polygonAccessor.GenerateTriangles (&numTriangles);

				for (unsigned k = 0; k < numTriangles; k++) {
					LXtPointID pointID [3];
					unsigned   index [3];
					polygonAccessor.TriangleByIndex (k, &pointID[0], &pointID[1], &pointID[2]);

					for (unsigned j = 0; j < 3; j++) {
						pointAccessor.Select (pointID[j]);
						pointAccessor.Index (&index[j]);
					}
					tri.triangles.push_back (Vec3I (index[0], index[1], index[2]));
				}
			}

			for (unsigned i = 0; i < numPoints; i++) {
				LXtFVector xyz;
				pointAccessor.SelectByIndex (i);
				pointAccessor.Pos (xyz);

				tri.points.push_back (Vec3s(xyz[0], xyz[1], xyz[2]));
			}

			return new OpenVDBItem (tri, creationInfo->voxelSize, creationInfo->cv_halfWidth, isValid);
			}
		}
	}

	return NULL;
}

	VoxelItemKey
OpenVDBCreateFromMesh::GenKey (
	const VDBVoxelChannels	*creationInfo)
{
	VoxelItemKey keys;
	unsigned key = VDBVoxelFNV::ComputeHashCode (*creationInfo, false);
	keys.base = key;

	/*
	*	Compute hash key by considering all triangles in the mesh.
	*	This is quite expensive, so it is only enabled with cache mode: Full.
	*/

	if (creationInfo->mesh != NULL) {

		CLxUser_Mesh		 mesh;
		CLxUser_MeshFilter	 mfilt;
		CLxUser_Scene		 scene;
		CLxUser_ChannelRead	 readChan;
		CLxLoc_Point		 pointAccessor;
		unsigned		 index;

		if (this->m_srcitem.ChannelLookup (LXsICHAN_MESH_MESH, &index) == LXe_OK) {
			scene.from (this->m_srcitem);
			scene.GetChannels (readChan, creationInfo->time); // Time is needed to get deformed meshes
			if (readChan.Object (this->m_srcitem, index, mfilt)) {
				if (mfilt.GetMesh (mesh)) {

					mesh.GetPoints (pointAccessor);
					unsigned numPoints = mesh.NPoints ();
					keys.mesh = VDBVoxelFNV::ComputeHashCode (pointAccessor, numPoints);
				}
			}

		}

	}
	return keys;
}

	OpenVDBItem *
OpenVDBCreateFromParticle::Alloc (
	VDBVoxelChannels	*creationInfo,
	bool			&isValid)
{
	return new OpenVDBItem (m_pa, creationInfo->voxelSize, creationInfo->cv_halfWidth, isValid);
}
	void
OpenVDBCreateFromParticle::SampleParticles (
	VDBVoxelChannels	*creationInfo,
	CParticleList		&pa)
{

	CLxUser_TableauVertex	 vdesc;
	CLxUser_TableauService	 tSrv;
	unsigned		 i;

	// Use triangle soup to process the particle source.
	CTriangleSoup		 soup (&pa, creationInfo->xfrm, creationInfo->cv_radius);
	tSrv.NewVertex (vdesc);
	vdesc.AddFeature (LXiTBLX_PARTICLES, LXsTBLX_PARTICLE_POS, &i);
	if (creationInfo->src_Feature (LXiTBLX_PARTICLES, LXsTBLX_PARTICLE_VEL)) {
		vdesc.AddFeature (LXiTBLX_PARTICLES, LXsTBLX_PARTICLE_VEL, &i);
		vdesc.Lookup (LXiTBLX_PARTICLES, LXsTBLX_PARTICLE_VEL, &soup.off_vel);
	}
	if (creationInfo->src_Feature (LXiTBLX_PARTICLES, LXsTBLX_PARTICLE_SIZE)) {
		vdesc.AddFeature (LXiTBLX_PARTICLES, LXsTBLX_PARTICLE_SIZE, &i);
		vdesc.Lookup (LXiTBLX_PARTICLES, LXsTBLX_PARTICLE_SIZE, &soup.off_size);
	}
	creationInfo->ptSrc.SetVertex (vdesc);
	creationInfo->ptSrc.Sample (NULL, -1.0, soup); // call the triangleSoup
}

	VoxelItemKey
OpenVDBCreateFromParticle::GenKey (
	const VDBVoxelChannels	*creationInfo)
{
	VoxelItemKey keys;
	unsigned key = VDBVoxelFNV::ComputeHashCode (*creationInfo, false);
	keys.base = key;

	unsigned pak = VDBVoxelFNV::ComputeHashCode (m_pa);
	keys.mesh = pak;

	return keys;
}

	void
OpenVDBCreateFromParticle::Init(
	VDBVoxelChannels *creationInfo)
{
	m_pa.reset();
	SampleParticles (creationInfo, m_pa);
}

	void
OpenVDBCreationStrategy::SetCacheBudget (
	const size_t	&sizeInByte)
{
	VoxelItemCachePolicy	*cachePolicy = VoxelItemCachePolicy::getInstance ();

	if (sizeInByte < CacheBudgetMin) {

		cachePolicy->setCacheSize(CacheBudgetMin);
	}
	else {
		cachePolicy->setCacheSize(sizeInByte);
	}
}