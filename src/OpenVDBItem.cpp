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

#include "common.h"
#include "OpenVDBItem.h"
#include <openvdb/tools/ParticlesToLevelSet.h>
#include <openvdb/math/Mat4.h>
#include <openvdb/tools/LevelSetFilter.h>
#include <openvdb/tools/Filter.h>
#include <fstream>
#include <boost/unordered_map.hpp>

// if you need other libs, it's better to remove this
using namespace openvdb;

typedef tools::ParticlesToLevelSet<FloatGrid, Index32>::AttGridType RasterGridType;
typedef boost::unordered_map<OpenVDBItem*, int> BoostMap;

// A global map to record all active OpenVDBItems, used for verification for multi-thread.
static BoostMap g_openVDBItems;

OpenVDBItemPreLoader::OpenVDBItemPreLoader (
	const std::string	&filename)
{
	io::File file (filename);

	// Open the file.  This reads the file header, but not any grids.
	try {
		if (!file.open ()) {
			return;
		}

	} catch (...) {

		return;
	}

	GridBase::Ptr gridBase;

	for (io::File::NameIterator nameIter = file.beginName ();
		nameIter != file.endName (); ++nameIter) {
		m_featureNames.push_back (nameIter.gridName ());
	}

	file.close ();
}

	std::string
OpenVDBItemPreLoader::getFeatureNameList (
	std::string		&name)
{
	name = "";

	if (m_featureNames.size() > 0) {
		for (unsigned i = 0; i < m_featureNames.size () - 1; i++) {
			name = name + m_featureNames[i] + "|";
		}

		name = name + m_featureNames[m_featureNames.size() - 1];
		return m_featureNames[0];
	} else {

		return name;
	}

}

OpenVDBItem::OpenVDBItem (
	const CTriangleList	&triangleList,
	float			 voxelSize,
	float			 halfWidth,
	bool			&isValid)
{
	g_openVDBItems.insert (BoostMap::value_type (this, 1));
	OpenVDBGrid	*vdbGridPtr = new OpenVDBGrid (triangleList, voxelSize, halfWidth);
	m_openVDBGrids.push_back (vdbGridPtr);
	m_featureNameCache.push_back (vdbGridPtr->m_floatGridPtr->getName ());
	m_optFeatures.clear();

	isValid = true;

	// for the voxel source interface
	m_refGrid = 0;
	m_useHDDA = 0;
}

OpenVDBItem::OpenVDBItem (
	const CParticleList	&particleList,
	float			 voxelSize,
	float			 halfWidth,
	bool			&isValid)
{
	g_openVDBItems.insert (BoostMap::value_type (this, 2));
	OpenVDBGrid* vdbGridPtr = new OpenVDBGrid (particleList, voxelSize, halfWidth);
	m_openVDBGrids.push_back (vdbGridPtr);
	m_featureNameCache.push_back (vdbGridPtr->m_floatGridPtr->getName ());
	m_optFeatures.clear();

	isValid = true;

	// for the voxel source interface
	m_refGrid = 0;
	m_useHDDA = 0;
}

OpenVDBItem::OpenVDBItem (
	const std::string	&filename,
	bool			&isValid)
{
	g_openVDBItems.insert (BoostMap::value_type (this, 3));
	m_optFeatures.clear();
	// for the voxel source interface
	m_refGrid = 0;
	m_useHDDA = 0;

	// early test if the file exists;
	std::ifstream f(filename.c_str());
	if (!f.good()) {
		f.close ();
		isValid = false;
		return;
	}

	io::File file (filename);
	// Open the file.  This reads the file header, but not any grids.
	if (!file.open()) {
		isValid = false;
		return;
	}

	GridBase::Ptr	 gridBase;
	OpenVDBGrid	*vdbGridPtr;

	for (io::File::NameIterator nameIter = file.beginName ();
		nameIter != file.endName (); ++nameIter) {
		gridBase = file.readGrid (nameIter.gridName ());
		vdbGridPtr = new OpenVDBGrid (gridBase);
		m_openVDBGrids.push_back (vdbGridPtr);
		m_featureNameCache.push_back (vdbGridPtr->m_floatGridPtr->getName ());
	}

	file.close ();

	isValid = true;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			m_sourceXFRM[i][j] = (i == j ? 1.0 : 0.0);
			m_sourceXFRMInv[i][j] = m_sourceXFRM[i][j];
			m_sourceNXFRM[i][j] = m_sourceXFRM[i][j];
		}
}

OpenVDBItem::~OpenVDBItem ()
{
	g_openVDBItems.erase(this);
	
	for (unsigned i = 0; i < m_openVDBGrids.size (); i++)
	{
		delete m_openVDBGrids[i];
	}
	m_openVDBGrids.clear ();
#if LOG_ON
	printf ("delete OpenVDB item %lx\n", this);
#endif
}

	size_t
OpenVDBItem::getMemoryUsage ()
{
	size_t count = 0;

	for (unsigned i = 0; i<m_openVDBGrids.size(); i++) {

		count += m_openVDBGrids[i]->getMemoryUsage();
	}
	return count;
}

	void
OpenVDBItem::saveToFile (
	const std::string	&filename)
{
	io::File file (filename);
	// Add the grid pointer to a container.
	GridPtrVec grids;
	for (unsigned i = 0; i < m_openVDBGrids.size (); i++)
		grids.push_back (m_openVDBGrids[i]->m_floatGridPtr);

	file.write (grids);
}

	LxResult
OpenVDBItem::voxel_FeatureCount (
	unsigned		*num)
{
	num[0] = m_openVDBGrids.size ();

	return LXe_OK;
}

	LxResult
OpenVDBItem::voxel_FeatureByIndex (
	unsigned	 index,
	const char	**name)
{
	name[0] = m_featureNameCache[index].c_str ();
	return LXe_OK;
}
	LxResult
OpenVDBItem::voxel_BBox (
	LXtTableauBox	 bbox)
{
	if (m_openVDBGrids.size() == 0) {
		for (unsigned i = 0; i < 6; i++) {
			bbox[i] = 0;
		}
		return LXe_OK;
	}
	FloatGrid::Ptr reFloatGrid = m_openVDBGrids[m_refGrid]->m_floatGridPtr;

	if (reFloatGrid == NULL || reFloatGrid->empty ()) {
		for (unsigned i = 0; i < 6; i++) {
			bbox[i] = 0;
		}
	}
	else {
		double dbbox[6];
		computeAABB(m_refGrid, dbbox);

		for (int i = 0; i < 6; i++)
			bbox[i] = dbbox[i];
	}

	return LXe_OK;
}

	LxResult
OpenVDBItem::voxel_NextPos (
	float		 currentPos,
	unsigned	 currentSegment,
	float		 stride,
	float		*segmentList,
	unsigned	*nextSegment,
	float		*nextPos)
{
	// non-uniform stride algorithm
	float start = segmentList[currentSegment * 2];
	float end = segmentList[currentSegment * 2 + 1];

	nextPos[0] = currentPos + stride;

	if(nextPos[0] < start) {
		nextPos[0] = start;
		return LXe_OK;
	}

	if(nextPos[0] > end) {
		nextPos[0] = end;
		nextSegment[0] = currentSegment + 1;
	}
	else {
		nextSegment[0] = currentSegment;
	}

	return LXe_OK;
}

	LxResult
OpenVDBItem::voxel_Sample (
	const LXtFVector	 pos,
	unsigned		 index,
	float			*val)
{
	LXtVector pos_vs;
	LXx_VCPY(pos_vs, pos);

	MulMatrix(pos_vs, m_sourceXFRMInv);
	m_openVDBGrids[index]->readData (pos_vs[0], pos_vs[1], pos_vs[2], *val);
	return LXe_OK;
}

	LxResult
OpenVDBItem::voxel_VDBData (
	void			**ppvObj)
{
	ppvObj[0] = (void*) (m_openVDBGrids[m_refGrid]->m_floatGridPtr.get ());
	return LXe_OK;
}
	bool
OpenVDBItem::IsCached(
	float			 pos)
{
	return (pos == m_previousPosition);
}

	unsigned
midPoint(unsigned left, unsigned right)
{
	return left + ( (right - left) * 0.5);	// avoid limited range
}

	unsigned
OpenVDBItem::InterectSegments(	// binary search
	float			 point,
	unsigned		 startID,
	unsigned		 endID,
	std::vector <float>	&t_hitList
)
{
	while (endID >= startID) {
		unsigned mid = midPoint (startID, endID);
		if (point < t_hitList[mid] && point >= t_hitList[mid - 1])
			return mid;
		else if (point < t_hitList[mid - 1])
			endID = mid - 1;
		else
			startID = mid + 1;
	}

	return endID;
}

	LxResult
OpenVDBItem::voxel_RayIntersect (
	const LXtVector		 origin,
	const LXtFVector	 direction,
	unsigned		*numberOfSegments,
	float			**hitlist)
{
	LXtFVector opos, dir;
	LXtVector tmpPos, tmpDir, newPos;

	LXx_VADD3(newPos, origin, direction);
	LXx_VSET3(tmpPos, origin[0], origin[1], origin[2]);

	MulMatrix(tmpPos, m_sourceXFRMInv);
	MulMatrix(newPos, m_sourceXFRMInv);

	LXx_VSUB3(tmpDir, newPos, tmpPos);

	LXx_VSET3(opos, tmpPos[0], tmpPos[1], tmpPos[2]);
	LXx_VSET3(dir, tmpDir[0], tmpDir[1], tmpDir[2]);

	if (m_useHDDA) {
		unsigned	 i;
		LXtFVector	*hitlist_vector = 0;

		numberOfSegments[0] = m_openVDBGrids[m_refGrid]->intersect (opos, dir, &hitlist_vector);

		*hitlist = new float[*numberOfSegments * 2];

		for (i = 0; i < numberOfSegments[0]; i++) {

			
			(*hitlist) [i * 2] = LXx_VDIST(opos, hitlist_vector[i * 2]);
			(*hitlist) [i * 2 + 1] = LXx_VDIST(opos, hitlist_vector[i * 2 + 1]);
		}
	} else {

		float entry, exist;

		if ( true == m_openVDBGrids[m_refGrid]->intersectBB (opos, dir, &entry, &exist)) {
			*hitlist = new float[2];
			numberOfSegments[0] = 1;
			(*hitlist) [0] = entry;
			(*hitlist) [1] = exist;
		}
		else
			numberOfSegments[0] = 0;
	}

	return LXe_OK;
}

	LxResult
OpenVDBItem::voxel_RayRelease (
	unsigned	 numberOfSegments,
	float		**Segmentlist)
{

	if (numberOfSegments > 0)
		delete *Segmentlist;

	return LXe_OK;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

	void
OpenVDBItem::setOptFeatureNames (
	const std::string	&namelist)
{
	m_optFeatures.clear();

	std::stringstream ss(namelist);
	std::string item;
	while (std::getline(ss, item, '|')) {
		m_optFeatures.push_back(item);
	}
}

	void
OpenVDBItem::MulMatrix (
	LXtVector		&vec,
	const LXtMatrix4	&mat)
{
	const float x = vec[0] * mat[0][0] + vec[1] * mat[1][0] + vec[2] * mat[2][0] + mat[3][0];
	const float y = vec[0] * mat[0][1] + vec[1] * mat[1][1] + vec[2] * mat[2][1] + mat[3][1];
	const float z = vec[0] * mat[0][2] + vec[1] * mat[1][2] + vec[2] * mat[2][2] + mat[3][2];
	const float w = vec[0] * mat[0][3] + vec[1] * mat[1][3] + vec[2] * mat[2][3] + mat[3][3];

	vec[0] = x/w;
	vec[1] = y/w;
	vec[2] = z/w;
}

// Reference from Mesa implementation: http://www.mesa3d.org
	void
InvMatrix (
	const LXtMatrix4	&m,
	LXtMatrix4		&inv)
{
	double det;

	inv[0][0] =
	m[1][1] * m[2][2] * m[3][3] -
	m[1][1] * m[2][3] * m[3][2] -
	m[2][1] * m[1][2] * m[3][3] +
	m[2][1] * m[1][3] * m[3][2] +
	m[3][1] * m[1][2] * m[2][3] -
	m[3][1] * m[1][3] * m[2][2];

	inv[1][0] =
	-m[1][0]* m[2][2] * m[3][3] +
	m[1][0] * m[2][3] * m[3][2] +
	m[2][0] * m[1][2] * m[3][3] -
	m[2][0] * m[1][3] * m[3][2] -
	m[3][0] * m[1][2] * m[2][3] +
	m[3][0] * m[1][3] * m[2][2];

	inv[2][0] =
	m[1][0] * m[2][1] * m[3][3] -
	m[1][0] * m[2][3] * m[3][1] -
	m[2][0] * m[1][1] * m[3][3] +
	m[2][0] * m[1][3] * m[3][1] +
	m[3][0] * m[1][1] * m[2][3] -
	m[3][0] * m[1][3] * m[2][1];

	inv[3][0] =
	-m[1][0]* m[2][1] * m[3][2] +
	m[1][0] * m[2][2] * m[3][1] +
	m[2][0] * m[1][1] * m[3][2] -
	m[2][0] * m[1][2] * m[3][1] -
	m[3][0] * m[1][1] * m[2][2] +
	m[3][0] * m[1][2] * m[2][1];

	inv[0][1] =
	-m[0][1]* m[2][2] * m[3][3] +
	m[0][1] * m[2][3] * m[3][2] +
	m[2][1] * m[0][2] * m[3][3] -
	m[2][1] * m[0][3] * m[3][2] -
	m[3][1] * m[0][2] * m[2][3] +
	m[3][1] * m[0][3] * m[2][2];

	inv[1][1] =
	m[0][0] * m[2][2] * m[3][3] -
	m[0][0] * m[2][3] * m[3][2] -
	m[2][0] * m[0][2] * m[3][3] +
	m[2][0] * m[0][3] * m[3][2] +
	m[3][0] * m[0][2] * m[2][3] -
	m[3][0] * m[0][3] * m[2][2];

	inv[2][1] =
	-m[0][0]* m[2][1] * m[3][3] +
	m[0][0] * m[2][3] * m[3][1] +
	m[2][0] * m[0][1] * m[3][3] -
	m[2][0] * m[0][3] * m[3][1] -
	m[3][0] * m[0][1] * m[2][3] +
	m[3][0] * m[0][3] * m[2][1];

	inv[3][1] =
	m[0][0] * m[2][1] * m[3][2] -
	m[0][0] * m[2][2] * m[3][1] -
	m[2][0] * m[0][1] * m[3][2] +
	m[2][0] * m[0][2] * m[3][1] +
	m[3][0] * m[0][1] * m[2][2] -
	m[3][0] * m[0][2] * m[2][1];

	inv[0][2] =
	m[0][1] * m[1][2] * m[3][3] -
	m[0][1] * m[1][3] * m[3][2] -
	m[1][1] * m[0][2] * m[3][3] +
	m[1][1] * m[0][3] * m[3][2] +
	m[3][1] * m[0][2] * m[1][3] -
	m[3][1] * m[0][3] * m[1][2];

	inv[1][2] =
	-m[0][0]* m[1][2] * m[3][3] +
	m[0][0] * m[1][3] * m[3][2] +
	m[1][0] * m[0][2] * m[3][3] -
	m[1][0] * m[0][3] * m[3][2] -
	m[3][0] * m[0][2] * m[1][3] +
	m[3][0] * m[0][3] * m[1][2];

	inv[2][2] =
	m[0][0] * m[1][1] * m[3][3] -
	m[0][0] * m[1][3] * m[3][1] -
	m[1][0] * m[0][1] * m[3][3] +
	m[1][0] * m[0][3] * m[3][1] +
	m[3][0] * m[0][1] * m[1][3] -
	m[3][0] * m[0][3] * m[1][1];

	inv[3][2] =
	-m[0][0]* m[1][1] * m[3][2] +
	m[0][0] * m[1][2] * m[3][1] +
	m[1][0] * m[0][1] * m[3][2] -
	m[1][0] * m[0][2] * m[3][1] -
	m[3][0] * m[0][1] * m[1][2] +
	m[3][0] * m[0][2] * m[1][1];

	inv[0][3] =
	-m[0][1]* m[1][2] * m[2][3] +
	m[0][1] * m[1][3] * m[2][2] +
	m[1][1] * m[0][2] * m[2][3] -
	m[1][1] * m[0][3] * m[2][2] -
	m[2][1] * m[0][2] * m[1][3] +
	m[2][1] * m[0][3] * m[1][2];

	inv[1][3] =
	m[0][0] * m[1][2] * m[2][3] -
	m[0][0] * m[1][3] * m[2][2] -
	m[1][0] * m[0][2] * m[2][3] +
	m[1][0] * m[0][3] * m[2][2] +
	m[2][0] * m[0][2] * m[1][3] -
	m[2][0] * m[0][3] * m[1][2];

	inv[2][3] =
	-m[0][0]* m[1][1] * m[2][3] +
	m[0][0] * m[1][3] * m[2][1] +
	m[1][0] * m[0][1] * m[2][3] -
	m[1][0] * m[0][3] * m[2][1] -
	m[2][0] * m[0][1] * m[1][3] +
	m[2][0] * m[0][3] * m[1][1];

	inv[3][3] =
	m[0][0] * m[1][1] * m[2][2] -
	m[0][0] * m[1][2] * m[2][1] -
	m[1][0] * m[0][1] * m[2][2] +
	m[1][0] * m[0][2] * m[2][1] +
	m[2][0] * m[0][1] * m[1][2] -
	m[2][0] * m[0][2] * m[1][1];

	det = m[0][0] * inv[0][0] + m[0][1] * inv[1][0] + m[0][2] * inv[2][0] + m[0][3] * inv[3][0];

	if (det != 0) {

		det = 1.0 / det;

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++)
				inv[i][j] = inv[i][j] * det;
		}
	}

}

	void
NormalMatrix (
	const LXtMatrix4	&view,
	LXtMatrix4		&n)
{
	LXtMatrix	m;

	// Get the rotation part of the view matrix
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			m[i][j] = view[i][j];
	}

	// compute normal matrix

	double det =
	+m[0][0]*(m[1][1]*m[2][2]-m[2][1]*m[1][2])
	-m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
	+m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);


	double invdet = 1/det;

	n[0][0] =  (m[1][1]*m[2][2]-m[2][1]*m[1][2])*invdet;
	n[1][0] = -(m[0][1]*m[2][2]-m[0][2]*m[2][1])*invdet;
	n[2][0] =  (m[0][1]*m[1][2]-m[0][2]*m[1][1])*invdet;
	n[0][1] = -(m[1][0]*m[2][2]-m[1][2]*m[2][0])*invdet;
	n[1][1] =  (m[0][0]*m[2][2]-m[0][2]*m[2][0])*invdet;
	n[2][1] = -(m[0][0]*m[1][2]-m[1][0]*m[0][2])*invdet;
	n[0][2] =  (m[1][0]*m[2][1]-m[2][0]*m[1][1])*invdet;
	n[1][2] = -(m[0][0]*m[2][1]-m[2][0]*m[0][1])*invdet;
	n[2][2] =  (m[0][0]*m[1][1]-m[1][0]*m[0][1])*invdet;

	n[3][0] = n[3][1] = n[3][2] = n[0][3] = n[1][3] = n[2][3] = 0;
	n[3][3] = 1;
}
	void
OpenVDBItem::applyTransfrom (
	const LXtMatrix4	&mat4)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m_sourceXFRM[i][j] = mat4[i][j];

	InvMatrix (m_sourceXFRM, m_sourceXFRMInv);
	NormalMatrix (m_sourceXFRM, m_sourceNXFRM);
}


	void
OpenVDBItem::applyFilter (
	int			 iterations,
	int			 radius,
	float			 minMask,
	float			 maxMask,
	bool			 invertMask,
	const FloatGrid		*mask,
	int			 filterType)
{
	// apply filter to all grids.
	for (int i = 0; i < m_openVDBGrids.size(); i++) {

		const bool isLevelSetGrid = m_openVDBGrids[i]->m_floatGridPtr->getGridClass () == GRID_LEVEL_SET;

		if (!isLevelSetGrid) {

			tools::Filter<FloatGrid, FloatGrid, util::NullInterrupter> filter(*m_openVDBGrids[i]->m_floatGridPtr);

			filter.setMaskRange (minMask, maxMask);
			filter.invertMask (invertMask);

			switch (filterType) {
				case FILT_MEAN:
					filter.mean (radius, iterations, mask);
					break;
				case FILT_GAUSSIAN:
					filter.gaussian (radius, iterations, mask);
					break;
				case FILT_MEDIAN:
					filter.median (radius, iterations, mask);
					break;
				default:
					break;
			}
		}
		else {

			tools::LevelSetFilter<FloatGrid, FloatGrid, util::NullInterrupter> filter(*m_openVDBGrids[i]->m_floatGridPtr);

			filter.setMaskRange (minMask, maxMask);
			filter.invertMask (invertMask);

			switch (filterType) {
				case FILT_MEAN:
					filter.mean (radius, mask);
					break;
				case FILT_GAUSSIAN:
					filter.gaussian (radius, mask);
					break;
				case FILT_MEDIAN:
					filter.median (radius, mask);
					break;
				default:
					break;
			}
		}
	}
}

	bool
OpenVDBItem::isValid (
	OpenVDBItem		*vdbItem)
{
	if (g_openVDBItems.find (vdbItem) != g_openVDBItems.end ()) {
		return true;
	}
	else {
#if LOG_ON
		printf ("Current OpenVDBItem %p is invalid!\n", vdbItem);
#endif
		return false;
	}
}
