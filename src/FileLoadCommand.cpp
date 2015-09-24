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

#include "FileLoadCommand.h"
#include "OpenVDBItem.h"

CCommandLoadFile::CCommandLoadFile ()
{
	/*
	*	The constructor is a good place to add arguments and
	*	set flags for those arguments. This command will have
	*	a single argument for the filename. The flags will mark
	*	the argument as optional. This will prevent modo from
	*	launching a dialog to ask for a filename string and
	*	instead allow the command to handle the missing argument
	*	value by spawning a file dialog.
	*/

	dyna_Add(ARG_FILENAME, LXsTYPE_FILEPATH);
	basic_SetFlags(ARGi_FILENAME, LXfCMDARG_OPTIONAL);
}

	int 
CCommandLoadFile::basic_CmdFlags ()
{
	/*
	*	The flags define if this should be an undoable command
	*	or not. If the command changes the state of the scene,
	*	then it should implement both the MODEL and UNDO flags.
	*/

	return LXfCMD_MODEL | LXfCMD_UNDO;
}

	bool 
CCommandLoadFile::basic_Enable (
	CLxUser_Message &msg)
{
	/*
	*	In practice, a command usually has an enabled state and
	*	a disabled state. This function allows you to return True
	*	or False to enable/disable the command. A message interface
	*	is provided to allow you to pass detailed information about
	*	the enable state back to the user. This command is always
	*	enabled.
	*/

	return true;
}

	void 
CCommandLoadFile::cmd_Interact ()
{
	/*
	*	When the command is executed, this function is first called.
	*	It's the ideal place to open a file dialog and ask for user
	*	interaction. The resulting path can then be passed to the
	*	commands execute method using the filename argument.
	*
	*	No scene or state altering code should be performed here.
	*/

	CLxUser_CommandService	 cmd_svc;
	CLxUser_Command		 command;
	CLxUser_ValueArray	 val_array;

	std::string		 filename = "";
	unsigned		 n = 0;


	/*
	*	Check if the filename argument is set, if it is, return early.
	*/

	if (dyna_IsSet (ARGi_FILENAME))
		return;

	/*
	*	Fire the commands to open the file dialog.
	*/

	cmd_svc.ExecuteArgString (-1, LXiCTAG_NULL, "dialog.setup fileOpen");
	cmd_svc.ExecuteArgString (-1, LXiCTAG_NULL, "dialog.title {Browse For Files...}");
	cmd_svc.ExecuteArgString (-1, LXiCTAG_NULL, "dialog.fileTypeCustom model {OpenVDB model} {*.vdb} vdb");
	cmd_svc.ExecuteArgString (-1, LXiCTAG_NULL, "dialog.open");

	/*
	*	Spawn a command that can be queried for the result.
	*/

	if (LXx_FAIL(cmd_svc.NewCommand (command, "dialog.result")))
		return;

	/*
	*	Query the first argument on the dialog.result command
	*	and store the results in a value array.
	*/

	if (LXx_FAIL(cmd_svc.QueryIndex (command, 0, val_array)))
		return;

	/*
	*	Get the number of elements in the array. There should
	*	only be one. If there are none, return early.
	*/

	n = val_array.Count ();

	if (!n)
		return;

	/*
	*	Read the string from the value array and set the filename
	*	argument to the value of the string.
	*/

	val_array.String (0, filename);
	attr_SetString (ARGi_FILENAME, filename.c_str ());
}

	void 
CCommandLoadFile::cmd_Execute (
	unsigned	 flags)
{
	CLxUser_ItemPacketTranslation	 itemPkt;
	CLxUser_SelectionService	 srv_sel;
	CLxUser_ChannelWrite		 chanWrite;
	CLxUser_ChannelWrite		 chanWriteEdit;
	CLxSceneSelection		 scene_sel;
	CLxUser_Scene			 scene;
	CLxUser_Item			 item;
	CLxUser_ItemGraph		 graph;
	LXtID4				 selID_item;
	std::string			 filename = "";
	std::string			 currentFeature;
	void				*pkt;

	/*
	*	Use selection system to get current VDBVoxelItem
	*	for executing the command.
	*/

	selID_item = srv_sel.LookupType (LXsSELTYP_ITEM);
	pkt = srv_sel.Recent (selID_item);
	scene_sel.Get (scene);
	itemPkt.autoInit ();
	itemPkt.GetItem (pkt, item);

	if (pkt != NULL) {
		attr_GetString (ARGi_FILENAME, filename);
		chanWrite.setupFrom (scene);
		if (filename != "") {

			/*	
			*	Update schematic graph link
			*/

			CLxUser_Scene		 scene;
			CLxUser_ItemGraph	 graph;
			CLxUser_Item		 old;

			if (scene.from (item) && graph.from (scene, GRAPH_VOXELITEM))
				while (graph.Reverse (item)) {
					graph.Reverse (item, 0, old);
					graph.DeleteLink (old, item);
				}

			/*
			*	Read only a small part of data from a VDB file for loading feature list.
			*	no voxel data is stored.
			*/

			OpenVDBItemPreLoader itemPreLoader (filename);
			chanWrite.Set (item, CN_DATA_FILE_NAME, filename.c_str());
			chanWrite.Set (item, CN_FILE_NAME, filename.c_str());
			
			std::string featureList;
			currentFeature = itemPreLoader.getFeatureNameList (featureList);
			chanWrite.Set (item, FEATURE_NAME_LIST, featureList.c_str());

			chanWriteEdit.from (item);
			chanWriteEdit.Set (item, CURRENT_FEATURE, currentFeature.c_str());
		}
			
	}
}

	LXtTagInfoDesc 
CCommandLoadFile::descInfo[] = {
	{ 0 }
};
