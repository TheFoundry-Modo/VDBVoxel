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

#include "VoxelItemBin.h"

/*
*	Bin for Voxel Item for displaying vertices.
*/

	LxResult
VoxelItemBin::surfbin_GetBBox (LXtBBox *bbox)
{
	LXtTableauBox	tBox;
	LxResult	result = Bound (tBox);

	SetBBoxFromTBox (bbox, tBox);

	return result;
}

	LxResult
VoxelItemBin::surfbin_FrontBBox (
	const LXtVector		 pos,
	const LXtVector		 dir,
	LXtBBox			*bbox)
{
	return LXe_NOTIMPL;
}

	LxResult
VoxelItemBin::stag_Get (
	LXtID4			 type,
	const char		**tag)
{
	tag[0] = "Default";
	return LXe_OK;
}
	LxResult
VoxelItemBin::tsrf_Bound (
	LXtTableauBox		 bbox)
{
	if (m_gridNo == (unsigned)-1)
		return LXe_OK;
	else
		return Bound (bbox);
}

	unsigned
VoxelItemBin::tsrf_FeatureCount (
	LXtID4			 type)
{
	return FeatureCount (type);
}

	LxResult
VoxelItemBin::tsrf_FeatureByIndex (
	LXtID4			 type,
	unsigned		 index,
	const char		**name)
{
	return FeatureByIndex (type, index, name);
}

	LxResult
VoxelItemBin::tsrf_SetVertex (
	ILxUnknownID		 vdesc)
{
	return SetVertex (vdesc);
}

	LxResult
VoxelItemBin::tsrf_Sample (
	const LXtTableauBox	 bbox,
	float			 scale,
	ILxUnknownID		 trisoup)
{
	if (cv_drawSurf && m_gridNo != (unsigned)-1)
		Sample (bbox, scale, trisoup);

	return LXe_OK;
}