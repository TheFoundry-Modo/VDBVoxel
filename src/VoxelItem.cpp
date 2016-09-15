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

#include "VoxelItem.h"
#include "FileLoadCommand.h"
#include "ClearCacheCommand.h"
#include "PopupCommand.h"
#include "ParticleAttach.h"
#include "VoxelItemChannel.h"

CLxAttributeDesc	 VoxelItemChannel::desc;

/*
*	Packages implement item types, or simple item extensions. They are
*	like the metatype object for the item type. They define the common
*	set of channels for the item type and spawn new instances.
*
*/


/*
*	descInfo defines, what is a VoxelItem (aka. VDBVoxel Item), that it is a locator,
*	has a dedicated schematic link, has voxel source object that
*	contains voxel source interface, has also instanceble object that
*	implements instanceable interface. It can be masked as well.
*
*/
	LXtTagInfoDesc
VoxelItemPackage::descInfo[] = {
	{ LXsPKG_SUPERTYPE,		 "baseVolume" },
	{ LXsPKG_GRAPHS,		 GRAPH_VOXELITEM },
	{ LXsPKG_VOXEL_CHANNEL,		 CN_VDBOBJ },
	{ LXsPKG_INSTANCEABLE_CHANNEL,	 CN_VDBOBJ },
	{ LXsPKG_IS_MASK,		 "."},
	{ 0 }
};

/*
*	add all interfaces for VDBVoxel Item.
*/

VoxelItemPackage::VoxelItemPackage ()
{
	vi_factory.AddInterface (new CLxIfc_PackageInstance <VoxelItemInstance>);
	vi_factory.AddInterface (new CLxIfc_TableauSource <VoxelItemInstance>);
	vi_factory.AddInterface (new CLxIfc_StringTag <VoxelItemInstance>);
	vi_factory.AddInterface (new CLxIfc_ViewItem3D <VoxelItemInstance>);
	vi_factory.AddInterface (new CLxIfc_SurfaceItem <VoxelItemInstance>);
	vi_factory.AddInterface (new CLxIfc_SceneItemListener <VoxelItemInstance>);
	vi_factory.AddInterface (new CLxIfc_Voxel <VoxelItemInstance>);

	surf_factory.AddInterface (new CLxIfc_Surface <VoxelItemSurface>);

	bin_factory.AddInterface (new CLxIfc_SurfaceBin <VoxelItemBin>);
	bin_factory.AddInterface (new CLxIfc_StringTag <VoxelItemBin>);
	bin_factory.AddInterface (new CLxIfc_TableauSurface <VoxelItemBin>);

	elt_factory.AddInterface (new CLxIfc_TableauSurface <VoxelItemElement>);
	elt_factory.AddInterface (new CLxIfc_TableauInstance <VoxelItemElement>);
}

	LxResult
VoxelItemPackage::pkg_SetupChannels (
	ILxUnknownID	 addChan)
{
	CLxUser_AddChannel	 ac(addChan);
	CLxUser_Value		 val;

	// CN_VDBOBJ indicates the VDBVoxelItem object, which is created in the Modifier when the channel is evaluated.
	ac.NewChannel (CN_VDBOBJ, LXsTYPE_OBJREF);
	return VoxelItemChannel::desc.setup_channels (addChan);;
}

	LxResult
VoxelItemPackage::pkg_TestInterface (
	const LXtGUID	*guid)
{
	return (vi_factory.TestInterface (guid) ? LXe_TRUE : LXe_FALSE);
}

	LxResult
VoxelItemPackage::pkg_Attach (
	void		**ppvObj)
{
	VoxelItemInstance	*instance = vi_factory.Alloc (ppvObj);

	instance->src_pkg = this;
	instance->inst_ifc = (ILxUnknownID) ppvObj[0];
	return LXe_OK;
}

	void
initialize()
{
	VoxelItemElement::initialize();
	VoxelItemChannel::initialize();
	VoxelItemPackage::initialize ();
	VDBVoxelItem::initialize ();
	VDBVoxelModifier::initialize ();
	CCommandLoadFile::initialize ();
	CCommandClearCache::initialize ();
	CCommandSetFeature::initialize ();
	CParticleAttachCommand::initialize ();
	CParticleConnection::initialize ();
}
