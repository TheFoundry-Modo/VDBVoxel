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

#ifndef PARTICLEATTACH_H
#define PARTICLEATTACH_H

#include <lx_item.hpp>
#include <lx_plugin.hpp>
#include <lx_particle.hpp>
#include <lx_schematic.hpp>
#include <lx_server.hpp>
#include <lx_action.hpp>
#include <lxu_command.hpp>
#include <lxu_math.hpp>
#include <lxidef.h>
#include <string>
#include <vector>
#include "common.h"
#include "SourceItem.h"
#include "VoxelItemPart.h"

#define SPNNAME_INSTANCE	"particle.inst"
#define SRVNAME_SRCCOMMAND	"particle.popup"
#define SRVNAME_CONNECTION	"particle.connection"
#define STYPE_SOURCE		0

static LXtTextValueHint hint_Type[] = {
	STYPE_SOURCE, "source",
	-1, 0
};

static CLxItemType		 cit_source(ITEM_SERVER_NAME);

/*
*	For detecting if the attaching target is a VDBVoxel.
*	This is used for making schematic links.
*/

class CSourceItems : public CLxItemSelection
{
    public:
	int			 type;

	CSourceItems(
		int t) : type(t) {}

		bool
	Include(
		CLxUser_Item	&item)
	{

		if (type == STYPE_SOURCE)
			return item.IsA (cit_source);

		return false;
	}
};

/*
*	Pop up menu command for attaching a particle source to VDBVoxel item.
*	After it is attached, the particle source can be used to create
*	VDB voxel data.
*/

class CParticleAttachCommand : public CLxBasicCommand
{
    public:
		static void
	initialize ()
	{
		CLxGenericPolymorph	*srv;

		srv = new CLxPolymorph <CParticleAttachCommand>;
		srv->AddInterface (new CLxIfc_Command     <CParticleAttachCommand>);
		srv->AddInterface (new CLxIfc_Attributes  <CParticleAttachCommand>);
		srv->AddInterface (new CLxIfc_AttributesUI<CParticleAttachCommand>);
		thisModule.AddServer(SRVNAME_SRCCOMMAND, srv);
	}

		static bool
	IsSource (
		CLxUser_SceneService	&srv,
		CLxUser_Item		&item,
		int			 type);

	CParticleAttachCommand ();

	~CParticleAttachCommand () {}

		int
	basic_CmdFlags ()		LXx_OVERRIDE;

		bool
	basic_Notifier (
		int			 index,
		std::string		&name,
		std::string		&args)
					LXx_OVERRIDE;
		bool
	basic_Enable(
		CLxUser_Message	&msg)	LXx_OVERRIDE;

		LxResult
	cmd_Query(
		unsigned	 index,
		ILxUnknownID	 vaQuery)
					LXx_OVERRIDE;

		CLxDynamicUIValue *
	atrui_UIValue(
		unsigned	 index)	LXx_OVERRIDE;

		void
	cmd_Execute(
		unsigned	 flags)	LXx_OVERRIDE;

    protected:

	/*
	*	Execute -- for each selected item this unlinks any source in the
	*	graph and sets the one we want.
	*/
		bool
	SetSource(
		CLxUser_Item	&item,
		CLxUser_Item	&source);

		bool
	UpDateUI(
		CLxUser_Item	&item,
		CLxUser_Item	&source);
	/*
	*	Customize the item popup. We'll only show particle sources and 'none'.
	*/
	class CItemPopup :
		public CLxDynamicUIValue
	{
	public:
		CLxUser_SceneService	 scene_S;
		int			 type;

		CItemPopup(int t) : type(t) {}

			unsigned
		Flags()
			LXx_OVERRIDE
		{
			return LXfVALHINT_ITEMS | LXfVALHINT_ITEMS_NONE;
		}

			bool
		ItemTest(CLxUser_Item &item)
			LXx_OVERRIDE
		{
			return IsSource(scene_S, item, type);
		}
	};
};


/*
*	Schematic version of the attachment.
*	This graph is one that we define, so we'll allow it to connect
*	from appropriate particle source items to voxel item.
*/

class CParticleConnection :
	public CLxImpl_SchematicConnection
{
    public:
	static LXtTagInfoDesc	 descInfo[];

		static void
	initialize()
	{
			CLxGenericPolymorph	*srv;

			srv = new CLxPolymorph<CParticleConnection>;
			srv->AddInterface(new CLxIfc_SchematicConnection<CParticleConnection>);
			srv->AddInterface(new CLxIfc_StaticDesc         <CParticleConnection>);
			lx::AddServer(SRVNAME_CONNECTION, srv);
	}

		static int
	type_Code(
		CLxUser_Item	&item);

	CLxUser_SceneService	scene_S;

		LxResult
	schm_ItemFlags(
		ILxUnknownID	 item,
		unsigned	*flags)	LXx_OVERRIDE;

		LxResult
	schm_AllowConnect(
		ILxUnknownID	 from,
		ILxUnknownID	 to)	LXx_OVERRIDE;

		LxResult
	schm_GraphName(
		const char	**name)	LXx_OVERRIDE;
};
#endif