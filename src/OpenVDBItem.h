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

#ifndef OPENVDBITEM_H
#define OPENVDBITEM_H

#include <openvdb/openvdb.h>
#include <openvdb/tools/RayIntersector.h>
#include <lxvmath.h>
#include <lxw_volume.hpp>
#include <string>
#include <limits>
#include "SourceItem.h"

using openvdb::FloatGrid;

void OpenVDBInit();
class OpenVDBItem;

/*
*	OpenVDBGrid stores the Grid data of VDB voxel.
*/

// TODO: some of interfaces are not needed in current release
class OpenVDBGrid
{
    public:
	OpenVDBGrid (
		openvdb::GridBase::Ptr gridBase);

	OpenVDBGrid (
		const CParticleList	&particleList,
		float			 voxelSize,
		float			 halfWidth);

	OpenVDBGrid (
		const CTriangleList	&triList,
		float			 voxelSize,
		float			 halfwidth);

	~OpenVDBGrid ();

		void
	computAABB (
		double			*bbox);

		float
	getValue (
		const float		 wcoords[3]);

		void
	readData (
		double			 x,
		double			 y,
		double			 z,
		float			&val);

		void
	setData (
		double			 x,
		double			 y,
		double			 z,
		float			 val);

	// return number of pairs ( entry point + exist point) 
		unsigned
	intersect (
		const LXtFVector	&origin,
		const LXtFVector	&dir,
		LXtFVector		**hitlist);
	// return intersect test result
		bool
	intersectBB (
		const LXtFVector	&origin,
		const LXtFVector	&dir,
		float			*entry,
		float			*exist);
	// generate samples for scale value rendering
		void
	genVoxelSamples ();
	// generate mesh (volume2mesh)
		void
	genMesh ();
	// visualize grid values
		void
	beginTraversal ();
	// return if there is still a node left
		bool
	iterNextNode (
		LXtVector	&BBMin,
		LXtVector	&BBMax,
		float		&density);

	// access vertex data
		inline void
	readSamplePoints (
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_samplePoints[3 * index];
		avec[1] = m_samplePoints[3 * index + 1];
		avec[2] = m_samplePoints[3 * index + 2];
	}
		inline void
	readSampleColors (
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_sampleColors[3 * index];
		avec[1] = m_sampleColors[3 * index + 1];
		avec[2] = m_sampleColors[3 * index + 2];
	}
		inline void
	readMeshPoints (
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_meshPoints[3 * index];
		avec[1] = m_meshPoints[3 * index + 1];
		avec[2] = m_meshPoints[3 * index + 2];
	}
		inline void
	readPointNormals (
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_pointNormals[3 * index];
		avec[1] = m_pointNormals[3 * index + 1];
		avec[2] = m_pointNormals[3 * index + 2];
	}
		inline void
	readMeshQuaIndices (
		unsigned		 index,
		unsigned		 aint4[4])
	{
		aint4[0] = m_meshQuaIndices[index * 4];
		aint4[1] = m_meshQuaIndices[index * 4 + 1];
		aint4[2] = m_meshQuaIndices[index * 4 + 2];
		aint4[3] = m_meshQuaIndices[index * 4 + 3];
	}
		inline unsigned
	getMeshQuadNum ()
	{
		return m_meshQuaIndices.size ();
	}
		inline unsigned
	getSampleNum ()
	{
		return m_sampleIndices.size ();
	}
		inline size_t
	getMemoryUsage ()
	{
		return m_floatGridPtr->memUsage();
	}
		void
	resample (
		const LXtMatrix4	&mat4);

	friend class OpenVDBItem;

    protected:
	// vdb data
	FloatGrid::Ptr			 m_floatGridPtr;
	// iterator for the grid
	FloatGrid::TreeType::NodeIter	 m_nodePtr;

	// CPU buffers for voxel samples
	std::vector<float>		 m_samplePoints;
	std::vector<float>		 m_sampleColors;
	std::vector<unsigned>		 m_sampleIndices;

	// CPU buffers for mesh quads
	std::vector<float>		 m_meshPoints;
	std::vector<double>		 m_pointNormals;	// point normals;
	std::vector<int>		 m_pointAcc;		// number of referenced points
	std::vector<unsigned>		 m_meshQuaIndices;	// quads
	std::vector<unsigned>		 m_meshTriIndices;	// triangles
	double				 m_meshAABB[6];		// bounding BOX;
	bool				 m_meshAABBValid;	// mesh bounding box valid;

	// world space bounding box
	openvdb::math::BBox<openvdb::Vec3d> m_wbb;		// world space

	// Generation states
	bool				 m_isSurfaceReady;
	bool				 m_isSampleReady;

};

/*
*	OpenVDBItem holds all VDB voxel data created from a file,
*	a particle source, or a mesh item. It is a class for array
*	of OpenVDBGrid.
*/

class OpenVDBItem
{
	// impl Voxel Source Interface
    public:
		LxResult
	voxel_FeatureCount (
		unsigned	*num);

		LxResult
	voxel_FeatureByIndex (
		unsigned	  index,
		const char	**name);

		LxResult
	voxel_BBox (
		LXtTableauBox bbox);

		LxResult
	voxel_NextPos (
		float		 currentPos,
		unsigned	 currentSegment,
		float		 stride,
		float		*segmentList,
		unsigned	*nextSegment,
		float		*nextPos);

		LxResult
	voxel_Sample (
		const LXtFVector pos,
		unsigned	 index,
		float		*val);

		LxResult
	voxel_VDBData (
		void		**ppvObj);

	// Voxel Source Interface : candidate 1
		LxResult
	voxel_RayIntersect (
		const LXtVector		 origin,
		const LXtFVector	 direction,
		unsigned		*numberOfSegments,
		float			**Segmentlist);

		LxResult
	voxel_RayRelease (
		unsigned		 numberOfSegments,
		float			**Segmentlist);

	 // Voxel Source Interface : candidate 2

    public:
	static	bool isValid (
		OpenVDBItem		*vdbItem);

	OpenVDBItem (
		const std::string	&filename,
		bool			&isValid);

	OpenVDBItem (
		const CParticleList	&particleList,
		float			 voxelSize,
		float			 halfWidth,
		bool			&isValid);

	OpenVDBItem (
		const CTriangleList	&triangleList,
		float			 voxelSize,
		float			 halfWidth,
		bool			&isValid);

	~OpenVDBItem ();

		void
	applyFilter (
		int			 iterations,
		int			 radius,
		float			 minMask,
		float			 maxMask,
		bool			 invertMask,
		const FloatGrid		*mask,
		int			 filterType);

		void
	saveToFile (
		const std::string	&filename);

		size_t
	getMemoryUsage ();

		void
	setOptFeatureNames (
		const std::string	&namelist);

		inline void
	getFeatureCount (
		unsigned	&number)
	{
		number = m_openVDBGrids.size ();
	}
		inline void
	getFeatureName (
		unsigned	 gridID,
		std::string	&name)
	{
		name = m_openVDBGrids[gridID]->m_floatGridPtr->getName ();
	}
		inline void
	getFeatureID (
		const std::string	&name,
		unsigned		&gridID)
	{
		for (unsigned i = 0; i < m_openVDBGrids.size(); i++) {
			if (m_openVDBGrids[i]->m_floatGridPtr->getName() == name) {
				gridID = getValidGridNo (i);
				return;
			}
		}
		for (unsigned i = 0; i < m_optFeatures.size(); i++) {
			if (m_optFeatures[i] == name) {
				gridID = getValidGridNo (i);
				return;
			}
		}
		// load from an old fbx, we translate featurename
		if (name == DEFAULT_FEATURE_OLD && DEFAULT_FEATURE_OLD != DEFAULT_FEATURE)
			return getFeatureID (DEFAULT_FEATURE, gridID);

		// feature not found
		gridID = (unsigned)-1;
	}
	// general interfaces
		inline void
	sample(
		unsigned gridID,
		float	 x,
		float	 y,
		float	 z,
		float	&val)
	{
		float  wcoords[3];
		wcoords[0] = x;
		wcoords[1] = y;
		wcoords[2] = z;
		val = m_openVDBGrids[gridID]->getValue (wcoords);
	}
		inline void
	computeAABB(
		unsigned	 gridID,
		double		*bbox)
	{
		m_openVDBGrids[gridID]->computAABB (bbox);

		LXtVector V[8];

		LXx_VSET3(V[0], bbox[0], bbox[1], bbox[2]);
		LXx_VSET3(V[1], bbox[3], bbox[1], bbox[2]);
		LXx_VSET3(V[2], bbox[0], bbox[4], bbox[5]);
		LXx_VSET3(V[3], bbox[3], bbox[4], bbox[5]);
		LXx_VSET3(V[4], bbox[0], bbox[1], bbox[5]);
		LXx_VSET3(V[5], bbox[3], bbox[4], bbox[2]);
		LXx_VSET3(V[6], bbox[0], bbox[4], bbox[2]);
		LXx_VSET3(V[7], bbox[3], bbox[1], bbox[5]);

		MulMatrix (V[0], m_sourceXFRM);

		bbox[0] = V[0][0];
		bbox[1] = V[0][1];
		bbox[2] = V[0][2];
		bbox[3] = V[0][0];
		bbox[4] = V[0][1];
		bbox[5] = V[0][2];


		for (int i = 1; i < 8; i++) {

			MulMatrix (V[i], m_sourceXFRM);

			for (int j = 0; j < 3; j++) {

				if (bbox[j] > V[i][j])
					bbox[j] = V[i][j];

				if (bbox[3 + j] < V[i][j])
					bbox[3 + j] = V[i][j];
			}
		}
	}

		inline unsigned
	intersect(
		unsigned		 gridID,
		const LXtFVector	&origin,
		const LXtFVector	&dir,
		LXtFVector		**hitlist)
	{
		return m_openVDBGrids[gridID]->intersect (origin, dir, hitlist);
	}
	// dedicated interfaces
		inline unsigned
	getValidGridNo (
		unsigned	gridID)
	{
		if (gridID < m_openVDBGrids.size())
			return gridID;
		else {
#if LOG_ON
			printf("ASSERT: gridID is out of bound, set to 0 instead.\n");
#endif
			return 0;
		}
	}
		inline void
	genMesh(
		unsigned	gridID)
	{
		m_openVDBGrids[gridID]->genMesh ();
	}
		inline void
	genVoxelSamples(
		unsigned	 gridID)
	{
		m_openVDBGrids[gridID]->genVoxelSamples ();
	}
		inline void
	beginTraversal(
		unsigned	 gridID)
	{
		m_openVDBGrids[gridID]->beginTraversal ();
	}
		inline bool
	iterNextNode(
		unsigned	 gridID,
		LXtVector	&BBMin,
		LXtVector	&BBMax,
		float		&density)
	{
		bool result = m_openVDBGrids[gridID]->iterNextNode (BBMin, BBMax, density);
		MulMatrix (BBMin, m_sourceXFRM);
		MulMatrix (BBMax, m_sourceXFRM);

		return result;
	}
	// accessor
		inline void
	readSamplePoints(
		unsigned	 gridID,
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_openVDBGrids[gridID]->m_samplePoints[3 * index];
		avec[1] = m_openVDBGrids[gridID]->m_samplePoints[3 * index + 1];
		avec[2] = m_openVDBGrids[gridID]->m_samplePoints[3 * index + 2];

		MulMatrix (avec, m_sourceXFRM);
	}
		inline void
	readSampleColors(
		unsigned	 gridID,
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_openVDBGrids[gridID]->m_sampleColors[3 * index];
		avec[1] = m_openVDBGrids[gridID]->m_sampleColors[3 * index + 1];
		avec[2] = m_openVDBGrids[gridID]->m_sampleColors[3 * index + 2];
	}
		inline void
	readPointNormals (
		unsigned	 gridID,
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_openVDBGrids[gridID]->m_pointNormals[3 * index];
		avec[1] = m_openVDBGrids[gridID]->m_pointNormals[3 * index + 1];
		avec[2] = m_openVDBGrids[gridID]->m_pointNormals[3 * index + 2];

		MulMatrix (avec, m_sourceNXFRM);
	}
		inline void
	readMeshPoints(
		unsigned	 gridID,
		unsigned	 index,
		LXtVector	&avec)
	{
		avec[0] = m_openVDBGrids[gridID]->m_meshPoints[3 * index];
		avec[1] = m_openVDBGrids[gridID]->m_meshPoints[3 * index + 1];
		avec[2] = m_openVDBGrids[gridID]->m_meshPoints[3 * index + 2];

		MulMatrix (avec, m_sourceXFRM);
	}
		inline void
	readMeshQuaIndices(
		unsigned	 gridID,
		unsigned	 index,
		unsigned	 aint4[4])
	{
		aint4[0] = m_openVDBGrids[gridID]->m_meshQuaIndices[index * 4];
		aint4[1] = m_openVDBGrids[gridID]->m_meshQuaIndices[index * 4 + 1];
		aint4[2] = m_openVDBGrids[gridID]->m_meshQuaIndices[index * 4 + 2];
		aint4[3] = m_openVDBGrids[gridID]->m_meshQuaIndices[index * 4 + 3];
	}
		inline void
	readValue(
		unsigned	 gridID,
		float		 x,
		float		 y,
		float		 z,
		float		&value)
	{
		m_openVDBGrids[0]->readData (x, y, z, value);
	}
		inline unsigned
	getMeshQuadNum(
		unsigned	 gridID)
	{
		return m_openVDBGrids[gridID]->m_meshQuaIndices.size ();
	}
		inline unsigned
	getSampleNum(
		unsigned	 gridID)
	{
		return m_openVDBGrids[gridID]->m_sampleIndices.size ();
	}

		void
	setUseHDDA (
		bool		use)
	{
		m_useHDDA = use;
	}
		void
	setCurrentGrid (
		unsigned		 gridNo)
	{
		m_refGrid = gridNo;
	}
	// source xfrm is applied to OpenVDBItem space to get 3D View space result.
		void
	applyTransfrom (
		const LXtMatrix4	&mat4);


    protected:
		bool
	IsCached (
		const LXtFVector	 eye,
		const LXtFVector	 dir);

		bool
	IsCached (
		float			 pos);

		unsigned
	InterectSegments (
		float			 point,
		unsigned		 startID,
		unsigned		 endID,
		std::vector<float>	&hitlist);

		void
	MulMatrix (
		LXtVector		&vec,
		const LXtMatrix4	&mat);

	std::vector<OpenVDBGrid* >	 m_openVDBGrids;
	std::vector<std::string>	 m_featureNameCache;
	std::vector<std::string>	 m_optFeatures;
	unsigned			 m_refGrid;
	float				 m_previousPosition;
	bool				 m_useHDDA;
	LXtMatrix4			 m_sourceXFRM;
	LXtMatrix4			 m_sourceNXFRM;
	LXtMatrix4			 m_sourceXFRMInv;
	int				 m_slotTemperFuel[2]; // gridIDs for temperature and fuel, density is customizable.
};

/*
*	Voxel data features are loaded before creating a OpenVDBItem.
*	OpenVDBItemPreLoader only reads the file header of a VDB file, not any grids.
*/

class OpenVDBItemPreLoader
{
    public:
	OpenVDBItemPreLoader(
		const std::string	&filename);

	~OpenVDBItemPreLoader(){};

		std::string
	getFeatureNameList(
		std::string		&name);

    protected:
	std::vector<std::string>	 m_featureNames;
};
#endif

