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

#include "ParticleAttach.h"

LXtTagInfoDesc	 CParticleConnection::descInfo[] = {
	{ LXsSRV_USERNAME, "@SchemaGraphs@Connection Source@" },
	{ 0 }
};

	int
CParticleConnection::type_Code (
	CLxUser_Item	&item)
{
	if (item.IsA(cit_source))
		return STYPE_SOURCE;

	return -1;
}

	LxResult
CParticleConnection::schm_ItemFlags (
	ILxUnknownID	 item,
	unsigned	*flags)
{
	CLxUser_Item	 it(item);

	if (type_Code(it) < 0)
		return LXe_NOTFOUND;

	flags[0] = LXfSCON_SINGLE;
	return LXe_OK;
}

	LxResult
CParticleConnection::schm_AllowConnect (
	ILxUnknownID		 from,
	ILxUnknownID		 to)
{
	CLxUser_Item		 it(to);
	CLxUser_Item		 fm(from);
	int			 type = type_Code (it);

	return CParticleAttachCommand::IsSource (scene_S, fm, type) ? LXe_TRUE : LXe_FALSE;
}
	LxResult
CParticleConnection::schm_GraphName (
	const char		**name)
{
	name[0] = GRAPH_VOXELITEM;
	return LXe_OK;
}


CParticleAttachCommand::CParticleAttachCommand ()
{
	dyna_Add ("type", LXsTYPE_INTEGER);
	dyna_SetHint (0, hint_Type);

	dyna_Add ("source", "&item");
	basic_SetFlags (1, LXfCMDARG_QUERY);
}

		

	int
CParticleAttachCommand::basic_CmdFlags ()
{
	return LXfCMD_MODEL | LXfCMD_UNDO;
}

	bool
CParticleAttachCommand::basic_Notifier (
	int			 index,
	std::string		&name,
	std::string		&args)
{
	if (index == 0) {
		name = "select.event";
		args = "item +v";		// VALUE on item selection

	}
	else if (index == 1) {
		name = "select.event";
		args = "item exist+d";		// DISABLE on item selection existence

	}
	else if (index == 2)
	{
		name = LXsNOTIFIER_CHANNEL;
		args = "+vd " CN_DATA_FILE_NAME " " ITEM_SERVER_NAME;
	}
	else if (index == 3)
	{
		name = LXsNOTIFIER_GRAPHS;
		args = GRAPH_VOXELITEM " " "+v";  
	}
	else 
		return false;

	return true;
}

	bool
CParticleAttachCommand::basic_Enable (
	CLxUser_Message		&msg)
{
	CLxUser_Item		 item;
	CSourceItems		 sel(dyna_Int(0));

	return sel.GetFirst (item);
}

	LxResult
CParticleAttachCommand::cmd_Query (
	unsigned		 index,
	ILxUnknownID		 vaQuery)
{
	CLxUser_ValueReference	 ref;
	CLxUser_ValueArray	 va(vaQuery);
	CSourceItems		 sel(dyna_Int(0));
	CLxUser_Item		 item;
	CLxUser_Value		 val;
	CSourceFinder		 src;

	for (sel.LoopInit (); sel.LoopNext (item);)
	{
		va.AddEmpty (val);
		ref.set (val);

		if (src.Find (item)) {
			ref.SetObject ( (ILxUnknownID)src.source_item);
		}
	}

	return LXe_OK;
}

	 bool
CParticleAttachCommand::IsSource (
	CLxUser_SceneService	&srv,
	CLxUser_Item		&item,
	int			 type)
{
		if (type == STYPE_SOURCE)
		{
			CLxUser_ParticleItem	 pi;
			return pi.set (item);
		}

		return false;
}

	bool
CParticleAttachCommand::UpDateUI (
	CLxUser_Item		&item,
	CLxUser_Item		&source)
{
	CLxUser_ChannelWrite	 chanWrite, chanWriteEdit;

	chanWrite.setupFrom (item);
	chanWrite.Set (item, CN_DATA_FILE_NAME, source.GetIdentity ().c_str ());

	const char	*feature = DEFAULT_FEATURE;
	chanWrite.Set (item, FEATURE_NAME_LIST, feature);

	chanWriteEdit.from (item);
	chanWriteEdit.Set (item, CURRENT_FEATURE, feature);

	return true;
}

	CLxDynamicUIValue *
CParticleAttachCommand::atrui_UIValue (
	unsigned		 index)
{
	return new CItemPopup (dyna_Int (0));
}

	bool
CParticleAttachCommand::SetSource (
	CLxUser_Item		&item,
	CLxUser_Item		&source)
{
		CLxUser_Scene		 scene;
		CLxUser_ItemGraph	 graph;
		CLxUser_Item		 old;

		if (!scene.from (item) || !graph.from (scene, GRAPH_VOXELITEM))
			return false;

		while (graph.Reverse (item)) {
			graph.Reverse (item, 0, old);
			graph.DeleteLink (old, item);
		}

		if (source.test ()) {
			graph.AddLink (source, item);
			return true;
		}

		return false;
	}

	void
CParticleAttachCommand::cmd_Execute (
	unsigned		 flags)
	{
	
	CSourceItems		 sel(dyna_Int(0));
	CLxUser_Item		 source, item;
	CLxUser_ValueReference	 ref;

	if (dyna_Object (1, ref))
		ref.Get (source);

	for (sel.LoopInit (); sel.LoopNext (item);) {
		if (SetSource (item, source)) {
			UpDateUI (item, source);
		} else {
			// if there is no source attached, we try to load from file, thus we init features first.
			// restore settings from file.

			CLxUser_ChannelRead	 chanRead;
			CLxUser_ChannelWrite	 chanWrite, chanWriteEdit;
			std::string		 currentFeature;
			std::string		 filename;

			chanRead.from (item);
			int index = item.ChannelIndex (CN_FILE_NAME);
			chanRead.GetString (item, index, filename);

			OpenVDBItemPreLoader itemPreLoader (filename);
			chanWrite.setupFrom (item);
			chanWrite.Set (item, CN_DATA_FILE_NAME, filename.c_str());
			std::string featureList;
			currentFeature = itemPreLoader.getFeatureNameList (featureList);
			chanWrite.Set (item, FEATURE_NAME_LIST, featureList.c_str());

			chanWriteEdit.from (item);
			chanWriteEdit.Set (item, CURRENT_FEATURE, currentFeature.c_str());
		}
	}
		
}