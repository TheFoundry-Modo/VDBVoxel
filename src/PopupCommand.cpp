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

#include "PopupCommand.h"
#include <string>
#include <sstream>

CCommandSetFeature::CCommandSetFeature ()
	:m_sel_geni (ITEM_SERVER_NAME)
{
	dyna_Add (ARG_FEATURENAME, LXsTYPE_STRING);
	basic_SetFlags (ARGi_FEATURENAME, LXfCMDARG_QUERY);

	m_idx_name = -1;
}

	int
CCommandSetFeature::basic_CmdFlags ()
{
	return LXfCMD_MODEL | LXfCMD_UNDO;
}

	void
CCommandSetFeature::InitRead (
	CLxUser_Item	&item)
{
	if (!m_read_chan.test())
		m_read_chan.from (item);

	m_idx_name = item.ChannelIndex (FEATURE_NAME_LIST);
}

/*
*	We have to limit the command to operate on items that have the same
*	type setting in order for the popup and editing the value to make
*	sense. This comes from the first selected item of our type.
*/
	bool
CCommandSetFeature::isCurrentType()
{
	CLxUser_Item		 item;
	if (!m_sel_geni.GetFirst (item))
		return false;
	else
		return true;
}

/*
*	Current type has to be something that can set the name in order to
*	enable the command.
*/
	bool
CCommandSetFeature::basic_Enable (
	CLxUser_Message		&msg)
{
	bool isCurrent = isCurrentType ();
	return isCurrent;
}

	bool 
CCommandSetFeature::basic_Notifier (
	int			 index,
	std::string		&name,
	std::string		&args)
{

	if (index == 0) {
		name = LXsNOTIFIER_CHANNEL;
		args = "+vd " FEATURE_NAME_LIST " " ITEM_SERVER_NAME;
		return true;
	}
	return false;
}

	void 
CCommandSetFeature::GetNameList(
	std::vector<std::string>	&list)
{
	CLxUser_Item		 item;
	CLxUser_ChannelWrite	 wchan;
	std::stringstream	 strStream;
	std::string		 featureList;
	std::string		 featureName;

	m_sel_geni.LoopInit ();
	while (m_sel_geni.LoopNext (item)) {
		InitRead (item);

		m_read_chan.GetString (item, m_idx_name, featureList);
		strStream.str (featureList);
		// split
		while (std::getline (strStream, featureName, '|')) {
			list.push_back (featureName);
		}
	}
}

/*
*	Our popup object uses the command object to update the list.
*/

	CLxDynamicUIValue * 
CCommandSetFeature::atrui_UIValue (
	unsigned		 index)
{
	return new CFeatureNamePopup (this);
}

	LxResult
CCommandSetFeature::cmd_Query (
	unsigned int		 index,
	ILxUnknownID		 vaQuery)
{
	CLxUser_ValueArray	 va(vaQuery);
	const char*		 name;
	CLxUser_Item		 item;
	
	m_sel_geni.LoopInit ();
	while (m_sel_geni.LoopNext (item)) {
		if (!m_read_chan.test ())
			m_read_chan.from (item);

		m_idx_name = item.ChannelIndex (CURRENT_FEATURE);
		m_read_chan.String (item, m_idx_name, &name);
		va.AddString (name);
	}
	
	return LXe_OK;
}

/*
*	Execute is super-simple. Just loop over the valid items and
*	set their name channel.
*/
	void 
CCommandSetFeature::cmd_Execute (
	unsigned		 flags)
{
	std::string		 name;
	CLxUser_Item		 item;
	CLxUser_ChannelWrite	 wchan;
	
	if (!dyna_String (ARGi_FEATURENAME, name)) {
		basic_Message ().SetCode (LXe_INVALIDARG);
		return;
	}

	m_sel_geni.LoopInit ();
	while (m_sel_geni.LoopNext (item)) {
		if (!m_read_chan.test ())
			m_read_chan.from (item);

		m_idx_name = item.ChannelIndex (CURRENT_FEATURE);
		if (!wchan.test ())
			wchan.from (item);

		wchan.Set (item, m_idx_name, name.c_str());
	}
	
}
