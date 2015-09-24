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

#ifndef VOXELITEMSURFACE_H
#define VOXELITEMSURFACE_H


#include <lx_surface.hpp>
#include <lx_action.hpp>
#include <lx_item.hpp>
#include "VoxelItemPart.h"

class VoxelItemPackage;

// Surface
class VoxelItemSurface :
	public CLxImpl_Surface,
	public VoxelItemPart,
	public VoxelItemChannel
{

	VoxelItemPackage	*src_pkg;

public:
	VoxelItemSurface ();

	~VoxelItemSurface ();
	
		LxResult
	surf_GetBBox (
		LXtBBox		*bbox)	LXx_OVERRIDE;

		LxResult
	surf_FrontBBox (
		const LXtVector	 pos,
		const LXtVector	 dir,
		LXtBBox		*bbox)	LXx_OVERRIDE;

		LxResult
	surf_RayCast (
		const LXtRayInfo	*ray,
		LXtRayHit		*hit)	LXx_OVERRIDE;

		LxResult
	surf_BinCount (
		unsigned int		*count)	LXx_OVERRIDE;

		LxResult
	surf_BinByIndex (
		unsigned int	 index,
		void		**ppvObj)	LXx_OVERRIDE;

		LxResult
	surf_TagCount (
		LXtID4		 type,
		unsigned int	*count)		LXx_OVERRIDE;

		LxResult
	surf_TagByIndex (
		LXtID4		 type,
		unsigned int	 index,
		const char	**stag)		LXx_OVERRIDE;

		LxResult
	Initialize (
		VoxelItemPackage		*pkg,
		unsigned			 gridNo,
		OpenVDBItem			*openVDBitem,
		bool				 internalRef);

protected:

	// useInternelReferenceCount should be only set to true, if m_OpenVDBItem used in action context is from evaluation context, so that action context can hold a reference which will be checked by the cache system to prevent from deleting too early expecially for multiple threads.
	// if m_OpenVDBItem is used in evaluation context, we do not use internel ref count since externel ref system lx::ObjXXXX is used.
	bool		 useInternelReferenceCount;
};


#endif