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

#ifndef VOXELITEMBIN_H
#define	VOXELITEMBIN_H

#include <lx_surface.hpp>
#include "VoxelItemPart.h"

/*
*	Bin for Voxel Item for displaying vertices.
*	It implements necessary interfaces for displaying
*	in 3D viewport, preview and offline render.
*
*/

class VoxelItemBin :
	public CLxImpl_SurfaceBin,
	public CLxImpl_TableauSurface,
	public CLxImpl_StringTag,
	public VoxelItemPart,
	public VoxelItemChannel

{
    public:

		LxResult
	surfbin_GetBBox (
		LXtBBox			*bbox)	LXx_OVERRIDE;

		LxResult
	surfbin_FrontBBox(
		const LXtVector		 pos,
		const LXtVector		 dir,
		LXtBBox			*bbox)	LXx_OVERRIDE;

		LxResult
	stag_Get(
		LXtID4			 type,
		const char		**tag)	LXx_OVERRIDE;

		LxResult
	tsrf_Bound(
		LXtTableauBox		 bbox)	LXx_OVERRIDE;

		unsigned
	tsrf_FeatureCount(
		LXtID4			 type)	LXx_OVERRIDE;

		LxResult
	tsrf_FeatureByIndex(
		LXtID4			 type,
		unsigned		 index,
		const char		**name)	LXx_OVERRIDE;

		LxResult
	tsrf_SetVertex(
		ILxUnknownID		 vdesc)	LXx_OVERRIDE;

		LxResult
	tsrf_Sample(
		const LXtTableauBox	 bbox,
		float			 scale,
		ILxUnknownID		 trisoup)LXx_OVERRIDE;
};

#endif