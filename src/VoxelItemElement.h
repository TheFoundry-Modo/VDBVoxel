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

#ifndef VOXELITEMELEMENT_H
#define VOXELITEMELEMENT_H

#include "VoxelItemPart.h"

/*
*	This is needed for drawing triangles 
*	if mesh generated from VDB Voxel data,
*	in offline render and preview.
*/

class VoxelItemElement:
	public CLxImpl_TableauSurface,
	public CLxImpl_TableauInstance,
	public CLxImpl_StringTag,
	public VoxelItemPart,
	public VoxelItemChannel
{
    public:
	LXtMatrix	 m_xfrm;
	LXtVector	 m_offset;

		static void
	initialize ()
	{
		CLxGenericPolymorph	*srv;
		// VoxelItemElement is registed to both pkg and generic spawner.
		// for pkg, it is used in VoxelItemInstance, for the later, it is for instanceable interface.
		srv = new CLxPolymorph<VoxelItemElement>;
		srv->AddInterface (new CLxIfc_TableauSurface<VoxelItemElement>);
		srv->AddInterface (new CLxIfc_StringTag     <VoxelItemElement>);
		lx::AddSpawner (SPNNAME_ELEMENT, srv);
	}
		LxResult
	tsrf_Bound (
		LXtTableauBox	 bbox)			LXx_OVERRIDE;

		unsigned
	tsrf_FeatureCount (
		LXtID4		 type)			LXx_OVERRIDE;

		LxResult
	tsrf_FeatureByIndex (
		LXtID4		 type,
		unsigned	 index,
		const char	**name)			LXx_OVERRIDE;

		LxResult
	tsrf_SetVertex(
		ILxUnknownID	 vdesc)			LXx_OVERRIDE;

		LxResult
	tsrf_Sample(
		const LXtTableauBox	 bbox,
		float			 scale,
		ILxUnknownID		 trisoup)	LXx_OVERRIDE;

		LxResult
	tins_Properties(
		ILxUnknownID	 vecstack)		LXx_OVERRIDE;

		LxResult
	tins_GetTransform(
		unsigned	 endPoint,
		LXtVector	 offset,
		LXtMatrix	 xfrm)			LXx_OVERRIDE;

		LxResult
	stag_Get (
		LXtID4		 type,
		const char	**tag)			LXx_OVERRIDE;
};

#endif