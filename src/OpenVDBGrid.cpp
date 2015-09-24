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
#include <openvdb/tools/GridTransformer.h>
#include <openvdb/tools/ParticlesToLevelSet.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/LevelSetFilter.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/util/NullInterrupter.h>
#include <openvdb/math/Mat4.h>
#include <openvdb/tools/ChangeBackground.h>
#include "openvdb/RenderModules.h"

// if you need other libs, it's better to remove this
using namespace openvdb;

	void
OpenVDBInit ()
{
	initialize ();
}

OpenVDBGrid::OpenVDBGrid (
	const CTriangleList	&triangleList,
	float			 voxelSize,
	float			 halfwidth)
{
	m_floatGridPtr = tools::meshToLevelSet<FloatGrid> (
		*math::Transform::createLinearTransform (voxelSize), triangleList.points, triangleList.triangles, halfwidth);
#if 0
	printf ("v%f, h%f\n", voxelSize, halfwidth);
	for (int i = 0; i < triangleList.triangles.size(); i++)
	{
		printf ("%i %i %i |", triangleList.triangles[i][0], triangleList.triangles[i][1], triangleList.triangles[i][2]);
	}
	for (int i = 0; i < triangleList.points.size(); i++)
	{
		printf ("%f %f %f |", triangleList.points[i][0], triangleList.points[i][1], triangleList.points[i][2]);
	}

	tools::VolumeToMesh mesher (m_floatGridPtr->getGridClass () == GRID_LEVEL_SET ? 0.0 : 0.01);
	mesher (*m_floatGridPtr);
	printf ("Num: %d\n", mesher.pointListSize ());
#endif
	/*
	* Convert to FogVolume
	*/
	m_floatGridPtr->setName (DEFAULT_FEATURE);
	tools::sdfToFogVolume (*m_floatGridPtr);
	m_floatGridPtr->setGridClass (GRID_FOG_VOLUME);
	tools::changeBackground (m_floatGridPtr->tree(), 0.0);

	m_isSurfaceReady = false;
	m_isSampleReady = false;
	
}

OpenVDBGrid::OpenVDBGrid (
	const CParticleList	&particleList,
	float			 voxelSize,
	float			 halfWidth)
{
	m_floatGridPtr = createLevelSet<FloatGrid> (voxelSize, halfWidth);
	tools::ParticlesToLevelSet <FloatGrid, Index32> raster (*m_floatGridPtr);
	m_floatGridPtr->setName (DEFAULT_FEATURE);

	raster.setGrainSize (1);//a value of zero disables threading
	raster.rasterizeSpheres (particleList);
	//[TODO] Velocity looks very buggy, so we do not use it.
	//raster.rasterizeTrails (particleList);
	raster.finalize ();

	/*
	* Convert to FogVolume
	*/
	tools::sdfToFogVolume (*m_floatGridPtr);
	m_floatGridPtr->setGridClass (GRID_FOG_VOLUME);
	tools::changeBackground (m_floatGridPtr->tree(), 0.0);

	m_isSurfaceReady = false;
	m_isSampleReady = false;
}

OpenVDBGrid::OpenVDBGrid (
	GridBase::Ptr	 gridBase)
{

	m_floatGridPtr = gridPtrCast<FloatGrid> (gridBase);
	if (m_floatGridPtr == 0) {
		m_floatGridPtr = openvdb::FloatGrid::create ();
		return;
	}
	
	tools::changeBackground (m_floatGridPtr->tree(), 0.0);

	openvdb::math::CoordBBox gbb;	// grid space
	Vec3R mmin, mmax;


	// early clip
	m_floatGridPtr->tree ().evalActiveVoxelBoundingBox (gbb);
	mmin = m_floatGridPtr->indexToWorld (gbb.min ());
	mmax = m_floatGridPtr->indexToWorld (gbb.max ());
	openvdb::math::BBox<openvdb::Vec3d> wbb (mmin, mmax); // world space
	m_wbb = openvdb::math::BBox <openvdb::Vec3d> (mmin, mmax);

	m_isSurfaceReady = false;
	m_isSampleReady = false;
}
OpenVDBGrid::~OpenVDBGrid ()
{
	m_samplePoints.clear ();
	m_sampleColors.clear ();
	m_sampleIndices.clear ();

	m_meshPoints.clear ();
	m_pointNormals.clear ();
	m_meshQuaIndices.clear ();
	m_meshTriIndices.clear ();

	m_isSurfaceReady = false;
	m_isSampleReady = false;
}

	void
OpenVDBGrid::computAABB (
	double		*bbox)
{
	if (m_floatGridPtr->empty ()) {
		for (unsigned i = 0; i < 6; i++) {
			bbox[i] = 0;
		}
	}
	else {
		CoordBBox coordBBox = m_floatGridPtr->evalActiveVoxelBoundingBox ();
		Vec3d min = m_floatGridPtr->indexToWorld (coordBBox.min ());
		Vec3d max = m_floatGridPtr->indexToWorld (coordBBox.max ());

		bbox[0] = min.x ();
		bbox[1] = min.y ();
		bbox[2] = min.z ();
		bbox[3] = max.x ();
		bbox[4] = max.y ();
		bbox[5] = max.z ();
	}
}

	float
OpenVDBGrid::getValue (
	const float	 wcoords[3])
{
	FloatGrid::Accessor accessor = m_floatGridPtr->getAccessor ();

	Vec3d hitpoint;
	hitpoint[0] = wcoords[0];
	hitpoint[1] = wcoords[1];
	hitpoint[2] = wcoords[2];

	Vec3d hitcoord = m_floatGridPtr->worldToIndex (hitpoint);
	Coord coordPos (hitcoord[0], hitcoord[1], hitcoord[2]);

	return accessor.getValue (coordPos);
}

	void
OpenVDBGrid::genVoxelSamples ()
{
	if (m_isSampleReady)
		return;

	const Index64 maxVoxelPoints = 26000000;

	Vec3s colorMap[4];
	colorMap[0] = Vec3s(0.3, 0.9, 0.3); // green
	colorMap[1] = Vec3s(0.9, 0.3, 0.3); // red
	colorMap[2] = Vec3s(0.9, 0.9, 0.3); // yellow
	colorMap[3] = Vec3s(0.3, 0.3, 0.9); // blue

	const FloatGrid::TreeType& tree = m_floatGridPtr->tree ();
	const bool isLevelSetGrid = m_floatGridPtr->getGridClass () == GRID_LEVEL_SET;

	FloatGrid::ValueType minValue, maxValue;
	tree::LeafManager<const FloatGrid::TreeType> leafs (tree);

	{
		tools::MinMaxVoxel<const FloatGrid::TreeType> minmax (leafs);
		minmax.runParallel ();
		minValue = minmax.minVoxel ();
		maxValue = minmax.maxVoxel ();
	}

	Index64 voxelsPerLeaf = FloatGrid::TreeType::LeafNodeType::NUM_VOXELS;

	if (!isLevelSetGrid) {
		FloatGrid::TreeType::ValueConverter <bool>::Type::Ptr interiorMask (new FloatGrid::TreeType::ValueConverter <bool>::Type (false));

		{	// Generate Interior Points
			interiorMask->topologyUnion (tree);
			interiorMask->voxelizeActiveTiles ();

			if (interiorMask->activeLeafVoxelCount () > maxVoxelPoints) {
				voxelsPerLeaf = std::max <Index64>(1,
					(maxVoxelPoints / interiorMask->leafCount ()));
			}

			tools::erodeVoxels (*interiorMask, 2);

			tree::LeafManager <FloatGrid::TreeType::ValueConverter<bool>::Type> maskleafs (*interiorMask);
			std::vector<size_t> indexMap (maskleafs.leafCount ());
			size_t voxelCount = 0;
			for (Index64 l = 0, L = maskleafs.leafCount(); l < L; ++l) {
				indexMap[l] = voxelCount;
				voxelCount += std::min (maskleafs.leaf (l).onVoxelCount (), voxelsPerLeaf);
			}

			m_samplePoints.resize (voxelCount * 3);
			m_sampleColors.resize (voxelCount * 3);
			m_sampleIndices.resize (voxelCount);

			openvdb_viewer::util::PointGenerator <FloatGrid::TreeType::ValueConverter<bool>::Type> pointGen (
				m_samplePoints, m_sampleIndices, maskleafs, indexMap, m_floatGridPtr->transform (), voxelsPerLeaf);
			pointGen.runParallel ();

			openvdb_viewer::util::PointAttributeGenerator <FloatGrid> attributeGen (
				m_samplePoints, m_sampleColors, *m_floatGridPtr, minValue, maxValue, colorMap);
			attributeGen.runParallel ();
		}
	}
	else {
		// set vectors to zero (size = 0)
		m_samplePoints.clear ();
		m_sampleColors.clear ();
		m_sampleIndices.clear ();
	}

	m_isSampleReady = true;
}

	void
OpenVDBGrid::genMesh ()
{
	if ( m_isSurfaceReady)
		return;

	tools::VolumeToMesh mesher (
		m_floatGridPtr->getGridClass () == GRID_LEVEL_SET ? 0.0 : 0.01);
	mesher (*m_floatGridPtr);

	// Copy points and generate point normals.
	m_meshPoints.resize (mesher.pointListSize () * 3);
	m_pointNormals.resize (mesher.pointListSize () * 3);

	tree::ValueAccessor <const FloatGrid::TreeType> acc (m_floatGridPtr->tree ());
	typedef math::Gradient <math::GenericMap, math::CD_2ND> Gradient;
	math::GenericMap map (m_floatGridPtr->transform ());
	Coord ijk;

	for (Index64 n = 0, i = 0, N = mesher.pointListSize (); n < N; ++n) {
		const Vec3s& p = mesher.pointList ()[n];
		m_meshPoints[i++] = p[0];
		m_meshPoints[i++] = p[1];
		m_meshPoints[i++] = p[2];
	}

	// Copy primitives
	tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList ();
	Index64 numQuads = 0;
	for (Index64 n = 0, N = mesher.polygonPoolListSize (); n < N; ++n) {
		numQuads += polygonPoolList[n].numQuads ();
	}

	m_meshQuaIndices.reserve (numQuads * 4);
	Vec3d normal0, normal1, e1, e2;

	m_pointAcc.resize (mesher.pointListSize ());

	for (Index64 i = 0; i < mesher.pointListSize (); i++) {
		m_pointAcc[i] = 0;
		m_pointNormals[i * 3 + 0] = 0.0;
		m_pointNormals[i * 3 + 1] = 0.0;
		m_pointNormals[i * 3 + 2] = 0.0;
	}

	for (Index64 n = 0, N = mesher.polygonPoolListSize (); n < N; ++n) {
		const tools::PolygonPool& polygons = polygonPoolList[n];
		for (Index64 i = 0, I = polygons.numQuads (); i < I; ++i) {
			const Vec4I& quad = polygons.quad(i);
			m_meshQuaIndices.push_back (quad[0]);
			m_meshQuaIndices.push_back (quad[1]);
			m_meshQuaIndices.push_back (quad[2]);
			m_meshQuaIndices.push_back (quad[3]);

			double length;

			e1 = mesher.pointList ()[quad[1]];
			e1 -= mesher.pointList ()[quad[0]];
			e2 = mesher.pointList ()[quad[2]];
			e2 -= mesher.pointList ()[quad[1]];
			normal0 = e1.cross (e2);

			length = normal0.length ();
			if (length > 1.0e-7) normal0 *= (1.0 / length);

			e1 = mesher.pointList ()[quad[3]];
			e1 -= mesher.pointList ()[quad[2]];
			e2 = mesher.pointList ()[quad[0]];
			e2 -= mesher.pointList ()[quad[3]];
			normal1 = e1.cross (e2);

			length = normal1.length ();
			if (length > 1.0e-7) normal1 *= (1.0 / length);

			normal0 += normal1;

			for (Index64 v = 0; v < 4; ++v) {

				m_pointAcc[quad[v]] +=2;
				m_pointNormals[quad[v] * 3 + 0] += -normal0[0];
				m_pointNormals[quad[v] * 3 + 1] += -normal0[1];
				m_pointNormals[quad[v] * 3 + 2] += -normal0[2];
			}
		}
	}

	for (Index64 i = 0; i < mesher.pointListSize (); i++) {
		if (m_pointAcc[i] == 0)
			continue;
		m_pointNormals[i * 3 + 0] /= m_pointAcc[i];
		m_pointNormals[i * 3 + 1] /= m_pointAcc[i];
		m_pointNormals[i * 3 + 2] /= m_pointAcc[i];
	}

	m_isSurfaceReady = true;
}

	void
OpenVDBGrid::beginTraversal ()
{
	m_nodePtr = m_floatGridPtr->tree ().beginNode ();

}

	bool
OpenVDBGrid::iterNextNode (
	LXtVector	&BBMin,
	LXtVector	&BBMax,
	float		&density)
{
	CoordBBox bbox = m_nodePtr.getBoundingBox ();
	// Nodes are rendered as cell-centered
	const Vec3d min(bbox.min ().x () - 0.5, bbox.min ().y () - 0.5, bbox.min ().z () - 0.5);
	const Vec3d max(bbox.max ().x () + 0.5, bbox.max ().y () + 0.5, bbox.max ().z () + 0.5);

	Vec3d fn = m_floatGridPtr->indexToWorld (min);
	BBMin[0] = fn[0];
	BBMin[1] = fn[1];
	BBMin[2] = fn[2];

	fn = m_floatGridPtr->indexToWorld (max);
	BBMax[0] = fn[0];
	BBMax[1] = fn[1];
	BBMax[2] = fn[2];

	Coord nodeCoord = m_nodePtr.getCoord ();
	// read value (density) from a node;
	density = m_floatGridPtr->tree ().getValue (nodeCoord);

	m_nodePtr.increment ();
	
	return (m_nodePtr);
}

	void
OpenVDBGrid::readData(
	double	 x,
	double	 y,
	double	 z,
	float	&val)
{
	FloatGrid::Accessor accessor = m_floatGridPtr->getAccessor ();

	Vec3d	 hitpoint;
	hitpoint[0] = x;
	hitpoint[1] = y;
	hitpoint[2] = z;

	Vec3d hitcoord = m_floatGridPtr->worldToIndex (hitpoint);
	Coord coordPos (hitcoord[0], hitcoord[1], hitcoord[2]);

	val = accessor.getValue (coordPos);
}

	void
OpenVDBGrid::setData(
	double	 x,
	double	 y,
	double	 z,
	float	 val)
{
	FloatGrid::Accessor accessor = m_floatGridPtr->getAccessor ();

	Vec3d	 hitpoint;
	hitpoint[0] = x;
	hitpoint[1] = y;
	hitpoint[2] = z;

	Vec3d hitcoord = m_floatGridPtr->worldToIndex (hitpoint);
	Coord coordPos (hitcoord[0], hitcoord[1], hitcoord[2]);

	accessor.setValue (coordPos, val);
}

	bool
OpenVDBGrid::intersectBB (
	const LXtFVector	&origin,
	const LXtFVector	&dir,
	float			*entry,
	float			*exist)
{

	Vec3d camera (origin[0], origin[1], origin[2]);
	double tlen = LXx_VLEN (dir);
	openvdb::Vec3d viewDir (dir[0] / tlen, dir[1] / tlen, dir[2] / tlen);
	std::vector <openvdb::math::Ray <double>::TimeSpan> pTS;
	math::Ray <double> ray (camera, viewDir);

	// early clip
	if (!ray.clip(m_wbb))
		return false;

	entry[0] = ray.t0 ();
	exist[0] = ray.t1 ();
	return true;

}
	unsigned
OpenVDBGrid::intersect (
	const LXtFVector	 &origin,
	const LXtFVector	 &dir,
	LXtFVector		**hitlist)
{
	Vec3d camera (origin[0], origin[1], origin[2]);
	double tlen = LXx_VLEN (dir);
	openvdb::Vec3d viewDir (dir[0] / tlen, dir[1] / tlen, dir[2] / tlen);
	std::vector <openvdb::math::Ray <double>::TimeSpan> pTS;
	math::Ray <double> ray (camera, viewDir);
	unsigned segmentNum;

	openvdb::tools::VolumeRayIntersector<FloatGrid > rayIntersector (*m_floatGridPtr);

	rayIntersector.setWorldRay (ray);
	rayIntersector.hits (pTS);
	segmentNum = pTS.size ();

	if (segmentNum == 0)
		return 0;
	else {
		// entry point + exist point
		*hitlist = new LXtFVector [segmentNum * 2];

		for (unsigned i = 0; i < segmentNum; i++)
		{
			Real pT = ceil (pTS[i].t0), pT1 = pTS[i].t1;

			Vec3R entryPos = rayIntersector.getWorldPos (pT);
			Vec3R existPos = rayIntersector.getWorldPos (pT1);
			LXx_VSET3((*hitlist) [2 * i], entryPos[0], entryPos[1], entryPos[2]);
			LXx_VSET3((*hitlist) [2 * i + 1], existPos[0], existPos[1], existPos[2]);
		}

		return segmentNum;

	}
}

	void
OpenVDBGrid::resample (
	const LXtMatrix4	&mat4)
{
	Mat4R vdbMatrix (
		mat4[0][0],mat4[0][1],mat4[0][2],mat4[0][3],
		mat4[1][0],mat4[1][1],mat4[1][2],mat4[1][3],
		mat4[2][0],mat4[2][1],mat4[2][2],mat4[2][3],
		mat4[3][0],mat4[3][1],mat4[3][2],mat4[3][3]);

	FloatGrid::Ptr targetGrid = m_floatGridPtr->copy (CP_NEW);
	tools::GridTransformer transformer(vdbMatrix);

	transformer.transformGrid<tools::BoxSampler, FloatGrid> (*m_floatGridPtr, *targetGrid);

	targetGrid->pruneGrid();
	m_floatGridPtr = targetGrid;

}
