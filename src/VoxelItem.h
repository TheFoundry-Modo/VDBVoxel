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

#ifndef VOXELITEM_H
#define VOXELITEM_H

#include <lx_plugin.hpp>
#include <lx_package.hpp>
#include "common.h"
#include "VoxelItemElement.h"
#include "VoxelItemBin.h"
#include "VoxelItemSurface.h"
#include "VoxelItemInstance.h"
#include "VDBVoxelItem.h"
#include "VDBVoxelModifier.h"

class VoxelItemPackage :
	public CLxImpl_Package
{
    public:
		static void
	initialize()
		{
			CLxGenericPolymorph	*srv;

			srv = new CLxPolymorph <VoxelItemPackage>;
			srv->AddInterface (new CLxIfc_Package	 <VoxelItemPackage>);
			srv->AddInterface (new CLxIfc_StaticDesc <VoxelItemPackage>);
			thisModule.AddServer (ITEM_SERVER_NAME, srv);
		}
	static LXtTagInfoDesc			descInfo[];
	CLxPolymorph <VoxelItemInstance>	vi_factory;
	CLxPolymorph <VoxelItemBin>		bin_factory;
	CLxPolymorph <VoxelItemSurface>		surf_factory;
	CLxPolymorph <VoxelItemElement>		elt_factory;

	VoxelItemPackage();

		LxResult
	pkg_SetupChannels(
		ILxUnknownID	 addChan)	LXx_OVERRIDE;

		LxResult
	pkg_TestInterface(
		const LXtGUID	*guid)		LXx_OVERRIDE;

		LxResult
	pkg_Attach(
		void		**ppvObj)	LXx_OVERRIDE;
};

#endif 

