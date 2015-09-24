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

#include "VDBVoxelItem.h"
#include "OpenVDBCreation.h"
#include "VoxelItemSurface.h"
#include "VoxelItemElement.h"
#include "Verification.h"

VDBVoxelChannels::VDBVoxelChannels() :
	ptSrc (NULL),time(0.0), voxelSize(0.0)
{
	hashKey.base = 0;
	hashKey.mesh = 0;
}

	bool
VDBVoxelChannels::src_Feature(
	LXtID4			 type,
	const char		*name)
{
	const char		*fname;
	unsigned		 i, n;

	n = ptSrc.FeatureCount(type);
	for (i = 0; i < n; i++) {
		ptSrc.FeatureByIndex(type, i, &fname);
		if (strcmp(name, fname) == 0)
			return true;
	}
	return false;
}

// impl Voxel Source Interface
	LxResult
VDBVoxelItem::voxel_FeatureCount (
	unsigned	*num)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_FeatureCount (num);
	else {
		num = 0;
		return LXe_OK;
	}
}

	LxResult
VDBVoxelItem::voxel_FeatureByIndex (
	unsigned	 index,
	const char	**name)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_FeatureByIndex (index, name);
	else {
		name[0] = NULL;
		return LXe_OK;
	}
}

	LxResult
VDBVoxelItem::voxel_BBox (
	LXtTableauBox bbox)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_BBox (bbox);
	else {
		return LXe_OK;
	}
}

	LxResult
VDBVoxelItem::voxel_NextPos (
	float		 currentPos,
	unsigned	 currentSegment,
	float		 stride,
	float		*segmentList,
	unsigned	*nextSegment,
	float		*nextPos)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_NextPos (currentPos, currentSegment, stride, segmentList, nextSegment, nextPos);
	else {
		return LXe_OK;
	}
}

	LxResult
VDBVoxelItem::voxel_Sample (
	const LXtFVector pos,
	unsigned	 index,
	float		*val)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_Sample (pos, index, val);
	else {
		*val = 0.0;
		return LXe_OK;
	}
}

	LxResult
VDBVoxelItem::voxel_VDBData (
	void		**ppvObj)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_VDBData (ppvObj);
	else {
		if (ppvObj != NULL)
			*ppvObj = NULL;
		return LXe_OK;
	}
}

	LxResult
VDBVoxelItem::voxel_RayIntersect (
	const LXtVector		 origin,
	const LXtFVector	 direction,
	unsigned		*numberOfSegments,
	float			**Segmentlist)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_RayIntersect (origin, direction, numberOfSegments, Segmentlist);
	else {
		*numberOfSegments = 0;
		return LXe_OK;
	}
}

	LxResult
VDBVoxelItem::voxel_RayRelease (
	unsigned	 numberOfSegments,
	float		**Segmentlist)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_RayRelease (numberOfSegments, Segmentlist);
	else {
		return LXe_OK;
	}
}

VDBVoxelItem::VDBVoxelItem () :
	pp_spawn (SPNNAME_ITEM)
{
#if LOG_ON
	printf ("create VDBVoxelItem %p\n", this);
#endif
	m_creationStrategy = new OpenVDBCreationStrategy();
}

VDBVoxelItem::~VDBVoxelItem () {
#if LOG_ON
	printf ("delete VDBVoxelItem %p\n", this);
#endif
	// Reduce the reference of VDB voxel data marked with currentInfo
	m_creationStrategy->Destroy (c_eval);

	 delete m_creationStrategy;
}

	bool
VDBVoxelItem::CreateVDBVoxel (
	OpenVDBCreator	*creator)
{
	m_creationStrategy->SetCreator(creator);
	if (loadOpenVDBModel(c_eval, m_creationStrategy) == LXe_OK)
		return true;
	else
		return false;
}

	int
VDBVoxelItem::instable_Compare (
	ILxUnknownID	 other)
{
	VDBVoxelItem	*that;

	that = sinst->Cast (other);

	if (that->GetOpenVDBItem() != this->GetOpenVDBItem())
		return 1;

	return VDBVoxelChannels::desc.struct_compare ((VoxelItemChannel* ) this->c_eval, (VoxelItemChannel*) that->c_eval);
}

	LxResult
VDBVoxelItem::instable_AddElements (
	ILxUnknownID	 tableau,
	ILxUnknownID	 instT0,
	ILxUnknownID	 instT1)
{
	CLxUser_Tableau			 tbx (tableau);
	CLxSpawner<VoxelItemElement>	 selt (SPNNAME_ELEMENT);
	ILxUnknownID			 obj;
	VoxelItemElement		*elt;

	if (!VoxelItem::IsValid(m_openVDBItem))
		return LXe_OK;
	
	// When it is nothing to draw, we do not add any elements.
	if (m_openVDBItem == NULL)
		return LXe_OK;

	elt = selt.Alloc (obj);
	CLxArmUnknownRef	 tmp (obj);

	sinst->Cast (instT0);
	sinst->Cast (instT1);

	unsigned		 gridNo;
	m_openVDBItem->getFeatureID (this->c_eval->cv_featureName, gridNo);

	elt->copy_channels (this->c_eval);
	elt->init (gridNo, m_openVDBItem);

	lx_err::check (tbx.AddInstanceableElement (obj, obj));
	return LXe_OK;
}