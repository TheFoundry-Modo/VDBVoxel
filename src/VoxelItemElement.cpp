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

#include "VoxelItemElement.h"
#include <lxvmath.h>

	LxResult
VoxelItemElement::tsrf_Bound (
	LXtTableauBox		 bbox)
{
	return Bound (bbox);
}

	unsigned
VoxelItemElement::tsrf_FeatureCount (
	LXtID4			 type)
{
	return FeatureCount (type);
}

	LxResult
VoxelItemElement::tsrf_FeatureByIndex (
	LXtID4			 type,
	unsigned		 index,
	const char		**name)
{
	return FeatureByIndex (type, index, name);
}

	LxResult
VoxelItemElement::tsrf_SetVertex (
	ILxUnknownID		 vdesc)
{
	return SetVertex (vdesc);
}

	LxResult
VoxelItemElement::tsrf_Sample (
	const LXtTableauBox	 bbox,
	float			 scale,
	ILxUnknownID		 trisoup)
{
	if (cv_drawSurf)
		Sample (bbox, scale, trisoup);
	return LXe_OK;
}

	LxResult
VoxelItemElement::tins_Properties (
	ILxUnknownID	 vecstack)
{
	return LXe_OK;
}

	LxResult
VoxelItemElement::tins_GetTransform (
	unsigned	 endPoint,
	LXtVector	 offset,
	LXtMatrix	 xfrm)
{
	LXx_VCPY(offset, m_offset);

	for (unsigned i = 0; i < 3; ++i)
		LXx_VCPY(xfrm[i], m_xfrm[i]);

	return LXe_OK;
}

	LxResult
VoxelItemElement::stag_Get (
	LXtID4		 type,
	const char	**tag)
{
	tag[0] = "Default";
	return LXe_OK;
}
