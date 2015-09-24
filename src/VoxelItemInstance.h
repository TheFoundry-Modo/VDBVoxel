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

#ifndef VOXELITEMINSTANCE_H
#define	VOXELITEMINSTANCE_H

#include <lx_surface.hpp>
#include <lx_item.hpp>
#include <lx_tableau.hpp>
#include <lx_package.hpp>
#include <lx_vmodel.hpp>
#include <lx_vector.hpp>
#include <lx_listener.hpp>
#include "OpenVDBItem.h"

class VoxelItemPackage;

// Instance
class VoxelItemInstance :
	public CLxImpl_PackageInstance,
	public CLxImpl_StringTag,
	public CLxImpl_TableauSource,
	public CLxImpl_ViewItem3D,
	public CLxImpl_SurfaceItem,
	public CLxImpl_SceneItemListener,
	public CLxImpl_Voxel
{
public:
	VoxelItemPackage		*src_pkg;
	OpenVDBItem			*m_openVDBItem;
	CLxUser_Item			 m_item;
	ILxUnknownID			 inst_ifc;
	CLxUser_PacketService		 pkt_service;
	CLxUser_ListenerService		 listener_service;

public:
	// impl Voxel Source Interface
		LxResult
	voxel_FeatureCount (
		unsigned	*num)		LXx_OVERRIDE;

		LxResult
	voxel_FeatureByIndex (
		unsigned	 index,
		const char	**name)		LXx_OVERRIDE;

		LxResult
	voxel_BBox (
		LXtTableauBox	 bbox)		LXx_OVERRIDE;

		LxResult
	voxel_NextPos (
		const float	 currentPos,
		unsigned	 currentSegment,
		const float	 stride,
		float		*segmentList,
		unsigned	*nextSegment,
		float		*nextPos)	LXx_OVERRIDE;

		LxResult
	voxel_Sample (
		const LXtFVector pos,
		unsigned	 index,
		float		*val)		LXx_OVERRIDE;

		LxResult
	voxel_VDBData (
		void		**ppvObj)	LXx_OVERRIDE;

		LxResult
	voxel_RayIntersect (
		const LXtVector	 origin,
		const LXtFVector direction,
		unsigned	*numberOfSegments,
		float		**Segmentlist)	LXx_OVERRIDE;

		LxResult
	voxel_RayRelease (
		unsigned	 numberOfSegments,
		float		**Segmentlist)	LXx_OVERRIDE;

		LxResult
	pins_Initialize (
		ILxUnknownID	 item,
		ILxUnknownID	 super)		LXx_OVERRIDE;

		void
	pins_Cleanup (void)			LXx_OVERRIDE;

		void
	pins_Doomed (void)			LXx_OVERRIDE;

		LxResult
	pins_SynthName(
		char		*buf,
		unsigned	 len)		LXx_OVERRIDE;

		LxResult
	pins_Newborn (
		ILxUnknownID	 original,
		unsigned	 flags)		LXx_OVERRIDE;

		LxResult
	pins_AfterLoad ()			LXx_OVERRIDE;

		LxResult
	tsrc_Elements (
		ILxUnknownID	 tblx)		LXx_OVERRIDE;

		LxResult
	tsrc_PreviewUpdate (
		int		 chanIndex,
		int		*update)	LXx_OVERRIDE;

		LxResult
	vitm_Draw (
		ILxUnknownID	 itemChanRead,
		ILxUnknownID	 viewStrokeDraw,
		int		 selectionFlags,
		LXtVector	 itemColor)	LXx_OVERRIDE;
	// StringTag
		LxResult
	stag_Get (
		LXtID4		 type,
		const char	**tag)		LXx_OVERRIDE;


	// SurfaceItem interface.

		LxResult
	isurf_GetSurface (
		ILxUnknownID	 chanRead,
		unsigned	 morph,
		void		**ppvObj)	LXx_OVERRIDE;

		LxResult
	isurf_Prepare (
		ILxUnknownID	 eval,
		unsigned	*index)		LXx_OVERRIDE;

		LxResult
	isurf_Evaluate (
		ILxUnknownID	 attr,
		unsigned	 index,
		void		**ppvObj)	LXx_OVERRIDE;

	/*
	*	CLxImpl_SceneItemListener interface.
	*	Item creator will be inited inside sil_ChannelValue for each instance.
	*	sil_LinkAdd method notifies the change of the graph and update the UI
	*/
		void
	sil_ChannelValue (
		const char		*action,
		ILxUnknownID		 itemObj,
		unsigned		 channel);


		void
	GraphChange (
		const char		*graph,
		ILxUnknownID		 item1,
		ILxUnknownID		 item2);

		void
	sil_LinkAdd (
		const char		*graph,
		ILxUnknownID		 itemFrom,
		ILxUnknownID		 itemTo)	LXx_OVERRIDE
	{
		GraphChange (graph, itemFrom, itemTo);
	}
};


#endif