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

#include "VoxelItemPart.h"
#include "VDBVoxelItem.h"
#include "VoxelItemCache.h"
#include "OpenVDBCreation.h"
#include "Verification.h"
#include "VoxelItemPref.h"
#include <lxmesh.h>
#include <lxu_attributes.hpp>

VoxelItemPart::VoxelItemPart ()
	: m_gridNo (0), m_openVDBItem (0), m_extraFeatureNum (0)
{
}

VoxelItemPart::~VoxelItemPart ()
{

}
	LxResult
VoxelItemPart::FeatureByIndex (
	LXtID4		 type,
	unsigned	 index,
	const char	**name)
{
	unsigned	i;
	LxResult	result = LXe_NOTFOUND;

	if (type == LXiTBLX_BASEFEATURE) {
		switch (index) {
		case FEATURE_POSITION:
			name[0] = LXsTBLX_FEATURE_POS;
			return LXe_OK;

		case FEATURE_OBJECT_POSITION:
			name[0] = LXsTBLX_FEATURE_OBJPOS;
			return LXe_OK;

		case FEATURE_NORMAL:
			name[0] = LXsTBLX_FEATURE_NORMAL;
			return LXe_OK;

		case FEATURE_VELOCITY:
			name[0] = LXsTBLX_FEATURE_VEL;
			return LXe_OK;

		default:
			result = LXe_OUTOFBOUNDS;
			break;
		}
	}
	else if (type == LXi_VMAP_WEIGHT)
	{
		string	 nameStr;
		char	*namePtr;

		if (!VoxelItem::IsValid(m_openVDBItem))
			return LXe_OK;

		m_openVDBItem->getFeatureName (index, nameStr);
		namePtr = new char[nameStr.size () + 1];
		
		for (i = 0; i < nameStr.size (); i++) {
			namePtr[i] = nameStr[i];
		}

		namePtr[nameStr.size ()] = '\0';

		name[0] = namePtr;

		return LXe_OK;
	}

	return result;
}

	unsigned
VoxelItemPart::FeatureCount (
	LXtID4		 type)
{
	int count = 0;

	if (type == LXiTBLX_BASEFEATURE) {
		count = BASE_FEATURE_COUNT;
	}
	else if (type == LXi_VMAP_WEIGHT)
	{
		count = m_extraFeatureNum;
	}

	return count;
}
	LxResult
VoxelItemPart::Bound (
	LXtTableauBox	 bbox)
{
	// return when it is not initialized yet.
	if (m_bboxValid.size () == 0)
		return LXe_OK;
	int i;

	m_gridNo = m_openVDBItem->getValidGridNo (m_gridNo);

	if (m_bboxValid[m_gridNo] == false && VoxelItem::IsValid (m_openVDBItem)) {
			m_openVDBItem->computeAABB (m_gridNo, &m_bbox[m_gridNo][0]);

		m_bboxValid[m_gridNo] = true;
	}

	for (i = 0; i < 6; i++)
		bbox[i] = m_bbox[m_gridNo][i];

	return LXe_OK;
}

	LxResult
VoxelItemPart::loadOpenVDBModel (
	VDBVoxelChannels			*creationInfo,
	OpenVDBCreationStrategy			*strategy)
{
	VoxelItemCache	*thisItemCache;
	VoxelItemInfo	 currentInfo;
	currentInfo.vdbDataname = creationInfo->cv_dataName;
	currentInfo.halfWidth = creationInfo->cv_halfWidth;
	currentInfo.voxelSize = creationInfo->voxelSize;
	currentInfo.radius = creationInfo->cv_radius;
	currentInfo.time = creationInfo->time;
	currentInfo.mesh = creationInfo->mesh;

	unsigned cacheSize = VoxelItemPref::getInstance()->CacheSize();
	strategy->SetCacheBudget (cacheSize * 1024 * 1024);
	thisItemCache = strategy->Create (creationInfo, currentInfo);
	/*
	* generate required data into cache
	*/
	if (thisItemCache!=NULL) {
		/*
		 * cache hits
		 */
		if (thisItemCache->IsCached ()) {
			LoadFromCache (thisItemCache);
		}
		else {

			OpenVDBItem* vdb = thisItemCache->cacheLoad ();
			vdb->getFeatureCount (thisItemCache->extraFeatureNum);

			LoadFromCache (thisItemCache);
		}

		// Apply the transformation of the source item after the cache pass.
		// So that, we do not have to invalidate OpenVDBItem object for each xfrm change.
		thisItemCache->ApplyXFRM (creationInfo->xfrm);
	}
	else {
		return LXe_FALSE;
	}

	return LXe_OK;
}

	LxResult
VoxelItemPart::Sample (
	const LXtTableauBox	 bbox,
	float			 scale,
	ILxUnknownID		 trisoup)
{
	CLxUser_TriangleSoup	 soup (trisoup);
	LXtTableauBox		 box;
	LxResult		 rc;
	unsigned		 index, i, j, k;
	float			 vec[3 * 4];
	LXtVector		 position, normal;

	if (!VoxelItem::IsValid(m_openVDBItem))
		return LXe_FALSE;

	if (m_openVDBItem == NULL) return LXe_FALSE;

	// We have to find out why m_gridNo can be invalid in this function. Use the LOG_ON to find it.
	m_gridNo = m_openVDBItem->getValidGridNo (m_gridNo);

	m_openVDBItem->genMesh (m_gridNo);

	if (!soup.TestBox (box))
		return LXe_OK;

	rc = soup.Segment (1, LXiTBLX_SEG_TRIANGLE);
	if (rc == LXe_FALSE)
		return LXe_OK;
	else if (LXx_FAIL(rc))
		return rc;

	unsigned numQuads = m_openVDBItem->getMeshQuadNum (m_gridNo) / 4;

	for (i = 0; i < numQuads; i++) {
		unsigned quadIndex[4];
		m_openVDBItem->readMeshQuaIndices (m_gridNo, i, quadIndex);

		for (k = 0; k < 4; k++) {
			m_openVDBItem->readMeshPoints (m_gridNo, quadIndex[k], position);
			m_openVDBItem->readPointNormals (m_gridNo, quadIndex[k], normal);
			LXx_Normalize(normal);

			for (j = 0; j < 3; j++) {
				vec[j + m_f_pos[0]] = position[j];
				vec[j + m_f_pos[1]] = 0.0f;
				vec[j + m_f_pos[2]] = -normal[j];
				vec[j + m_f_pos[3]] = 0.0f;
			}
			// scalar extra features
			for (j = 0; j < m_extraFeatureNum; j++)
			{
				float temp;
				m_openVDBItem->readValue (j, position[0], position[1], position[2], temp);
				vec[m_f_pos[4 + j]] = temp;
			}
			soup.Vertex (vec, &index);
		}
	}

	for (j = 0; j< numQuads; j++) {
		soup.Quad (4 * j, 4 * j + 1, 4 * j + 2, 4 * j + 3);
	}

	return LXe_OK;
}

	LxResult
VoxelItemPart::SetVertex (
	ILxUnknownID		 vdesc)
{
	LxResult	 rc;
	const char	 *name;
	unsigned	 offset, i;

	m_f_pos.resize (TOTAL_FEATURE_COUNT + m_extraFeatureNum);


	if (!vrt_desc.set(vdesc))
		return LXe_NOINTERFACE;

	for (i = 0; i < TOTAL_FEATURE_COUNT; i++) {
		FeatureByIndex (LXiTBLX_BASEFEATURE, i, &name);
		rc = vrt_desc.Lookup (LXiTBLX_BASEFEATURE, name, &offset);
		m_f_pos[i] = (rc == LXe_OK ? offset : -1);
	}

	if (rc == LXe_OK) {
		for (i = 0; i < m_extraFeatureNum; i++){
			FeatureByIndex (LXi_VMAP_WEIGHT, i, &name);
			rc = vrt_desc.Lookup (LXi_VMAP_WEIGHT, name, &offset);
			m_f_pos[i + TOTAL_FEATURE_COUNT] = offset;
		}
	}
	return LXe_OK;
}

	void 
VoxelItemPart::SetBBoxFromTBox (
	LXtBBox		*bbox,
	LXtTableauBox	 tBox)
{
	LXx_V3SET(bbox->min, tBox[0], tBox[1], tBox[2]);
	LXx_V3SET(bbox->max, tBox[3], tBox[4], tBox[5]);
	LXx_V3SET(bbox->extent, tBox[3] - tBox[0], tBox[4] - tBox[1], tBox[5] - tBox[2]);
	LXx_VCLR(bbox->center);
}
	void
VoxelItemPart::LoadFromCache (const VoxelItemCache* itemCache)
{
	m_openVDBItem = itemCache->cacheLoad ();
	m_extraFeatureNum = itemCache->extraFeatureNum;
	m_bboxValid.resize (m_extraFeatureNum);
	m_bbox.resize (m_extraFeatureNum);
}
	void
VoxelItemPart::init (
	unsigned		 gridNo,
	OpenVDBItem		*openVDBItem)
{
	unsigned i, featureNum;
	m_openVDBItem = openVDBItem;
	
	if (m_openVDBItem == NULL || !VoxelItem::IsValid(m_openVDBItem)) {
		featureNum = 0;
	}
	else {
		m_openVDBItem->getFeatureCount(featureNum);
		m_gridNo = openVDBItem->getValidGridNo (gridNo);
	}

	m_extraFeatureNum = featureNum;
	m_bboxValid.resize (featureNum);
	m_bbox.resize (featureNum);

	for (i = 0; i < featureNum; i++) {
		m_bbox[i].resize (6);
		m_bboxValid[i] = false;
	}
}