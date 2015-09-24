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

#include "VoxelItemSurface.h"
#include "VoxelItemBin.h"
#include "VoxelItem.h"
#include "VoxelItemCache.h"

/*
*	VoxelItem Surface.
*/

VoxelItemSurface::VoxelItemSurface () : useInternelReferenceCount (0)
{
#if LOG_ON
	printf("create VoxelItemSurface %p\n", this);
#endif
}

VoxelItemSurface::~VoxelItemSurface ()
{
#if LOG_ON
	printf("delete VoxelItemSurface %p\n", this);
#endif
	if (useInternelReferenceCount) {
		VoxelItemCachePolicy *policy = VoxelItemCachePolicy::getInstance();
		policy->MarkUnusedByAction (m_openVDBItem);
	}

}

	LxResult
VoxelItemSurface::Initialize (
	VoxelItemPackage		*pkg,
	unsigned			 gridNo,
	OpenVDBItem			*openVDBItem,
	bool				 internalRef)
{
	LxResult result = LXe_FAILED;

	useInternelReferenceCount = internalRef;

	src_pkg = pkg;
	VoxelItemPart::init (gridNo, openVDBItem);

	result = LXe_OK;

	if (useInternelReferenceCount) {

		VoxelItemCachePolicy *policy = VoxelItemCachePolicy::getInstance();
		policy->MarkUsedByAction (m_openVDBItem);
	}

	return result;
}

	LxResult
VoxelItemSurface::surf_GetBBox (
	LXtBBox			*bbox)
{
	LXtTableauBox		 tBox;
	LxResult		result = Bound (tBox);
	SetBBoxFromTBox (bbox, tBox);

	return result;
}

	LxResult
VoxelItemSurface::surf_FrontBBox (
	const LXtVector		 pos,
	const LXtVector		 dir,
	LXtBBox			*bbox)
{
	return LXe_NOTIMPL;
}

	LxResult
VoxelItemSurface::surf_RayCast (
	const LXtRayInfo	*ray,
	LXtRayHit		*hit)
{
	return LXe_NOTIMPL;
}

	LxResult
VoxelItemSurface::surf_BinCount (
	unsigned		*count)
{
	LxResult result = LXe_OK;
	// nubmer of bins
	*count = 1;

	return result;
}

	LxResult
VoxelItemSurface::surf_BinByIndex (
	unsigned		 index,
	void			**ppvObj)
{

	VoxelItemBin *bin = src_pkg->bin_factory.Alloc (ppvObj);
	bin->copy_channels(this);
	bin->init (m_gridNo, m_openVDBItem);
	return LXe_OK;
}

	LxResult
VoxelItemSurface::surf_TagCount (
	LXtID4			 type,
	unsigned		*count)
{
	// number of tags
	*count = 0;

	return LXe_OK;
}

	LxResult
VoxelItemSurface::surf_TagByIndex (
	LXtID4			 type,
	unsigned		 index,
	const char		**stag)
{
	return LXe_OK;
}