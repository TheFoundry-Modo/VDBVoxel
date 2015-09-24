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

#ifndef VDBVOXELITEM_H
#define VDBVOXELITEM_H

#include "VoxelItemPart.h"
#include "VoxelItemChannel.h"
#include "VoxelItemCache.h"

class OpenVDBCreationStrategy;
class OpenVDBCreator;

/*
*	VDBVoxelChannels is used for containing all channel
*	values of a VDBVoxelItem.
*/

class VDBVoxelChannels :
	public CLxRefCounted,
	public VoxelItemChannel
{
public:
	CLxUser_TableauSurface	 ptSrc;
	LXtMatrix4		 xfrm;
	double			 time, voxelSize;
	CLxUser_Mesh		 mesh;
	VoxelItemKey		 hashKey;

	VDBVoxelChannels();

		bool
	src_Feature(
		LXtID4			 type,
		const char		*name);
};

/*
*	VDBVoxelItem implements Voxel Source Interface 
*	for rendering with volume items in offline render
*	and preview. It also has a method to get the voxel data
*	that is stored in OpenVDBItem.
*/

class VDBVoxelItem :
	public CLxImpl_Visitor,
	public CLxImpl_Voxel,
	public CLxImpl_Instanceable,
	public VoxelItemPart
{
public:
		static void
	initialize()
	{
		CLxGenericPolymorph	*srv;

		srv = new CLxPolymorph <VDBVoxelItem>;
		srv->AddInterface (new CLxIfc_Visitor <VDBVoxelItem>);
		srv->AddInterface (new CLxIfc_Voxel <VDBVoxelItem>);
		srv->AddInterface (new CLxIfc_Instanceable<VDBVoxelItem>);
		lx::AddSpawner (SPNNAME_ITEM, srv);
	}

	CLxSpawner<VDBVoxelItem>	 pp_spawn;
	CLxHolder<VDBVoxelChannels>	 c_eval;
	CLxPolymorph<VDBVoxelItem>	*sinst;

	VDBVoxelItem ();

	~VDBVoxelItem ();

		bool
	CreateVDBVoxel (
		OpenVDBCreator	*creator);
	
		inline OpenVDBItem*
	GetOpenVDBItem ()
	{
		return m_openVDBItem;
	}

	// impl voxel source interface
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

	// impl instanceable interface.
		int
	instable_Compare (
		ILxUnknownID	 other)		LXx_OVERRIDE;

		LxResult
	instable_AddElements (
		ILxUnknownID	 tableau,
		ILxUnknownID	 instT0,
		ILxUnknownID	 instT1)	LXx_OVERRIDE;

protected:
	OpenVDBCreationStrategy		*m_creationStrategy;
};

#endif
