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

#ifndef POPUPCOMMAND_H
#define POPUPCOMMAND_H

#include <lx_plugin.hpp>
#include <lxu_command.hpp>
#include <lx_command.hpp>
#include <lx_io.hpp>
#include <lx_seltypes.hpp>
#include <lxu_select.hpp>
#include <lxu_message.hpp>
#include <lx_channelui.hpp>

#include "common.h"

#define FEATURE_POPUP_SERVER_NAME				"feature.popup"
#define ARG_FEATURENAME						"featurename"
#define ARGi_FEATURENAME					0
#define POPUP_COMMAND_NOTIFIER					"popup.command.notifier"

using std::string;
using std::vector;

/*
*	Pop up menu command for selecting a valid feature of VDB voxel data.
*	A feature maps to a grid in OpenVDBItem.
*/

class CCommandSetFeature
	: public CLxBasicCommand
{
    public:
		static void
	initialize ()
	{
		CLxGenericPolymorph		*srv;

		srv =new CLxPolymorph					<CCommandSetFeature>;
		srv->AddInterface (new CLxIfc_Command			<CCommandSetFeature>);
		srv->AddInterface (new CLxIfc_Attributes		<CCommandSetFeature>);
		srv->AddInterface (new CLxIfc_AttributesUI		<CCommandSetFeature>);
		thisModule.AddServer (FEATURE_POPUP_SERVER_NAME, srv);
	}

	CCommandSetFeature ();

		int
	basic_CmdFlags ()		LXx_OVERRIDE;

		bool
	basic_Enable (
		CLxUser_Message	&msg)	LXx_OVERRIDE;

		bool
	basic_Notifier (
		int		index,
		std::string	&name,
		std::string	&args)	LXx_OVERRIDE;

		CLxDynamicUIValue *
	atrui_UIValue (
		unsigned int	 index)	LXx_OVERRIDE;

		LxResult
	cmd_Query (
		unsigned int	 index,
		ILxUnknownID	 vaQuery)LXx_OVERRIDE;

		void
	cmd_Execute (
		unsigned	 flags)	LXx_OVERRIDE;

    protected:

	class CFeatureNamePopup
		: public CLxUIListPopup
	{
	    public:
		CCommandSetFeature	*cmd;
		CFeatureNamePopup (
			CCommandSetFeature* val) : cmd (val){}

			void
		UpdateLists () { cmd->GetNameList (user_list);}
	};

		void
	GetNameList (
		vector<string>	&list);

		void
	InitRead (
		CLxUser_Item	&item);

		bool
	isCurrentType ();

	/*
	*	The item selection type we're interested in is our own.
	*/

	CLxItemSelectionType	 m_sel_geni;

	/*
	*	We use a common channel read, and assume that all the items processed
	*	in the selection are from the same scene. When we set the read object
	*	we record the channel indicies.
	*/

	CLxUser_ChannelRead	 m_read_chan;
	int			 m_idx_name;
};

#endif