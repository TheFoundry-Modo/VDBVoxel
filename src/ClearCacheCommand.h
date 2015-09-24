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

#ifndef CLEARCACHECOMMAND_H
#define CLEARCACHECOMMAND_H

#include <lx_plugin.hpp>
#include <lxu_command.hpp>
#include <lx_command.hpp>
#include <lx_io.hpp>
#include <lx_seltypes.hpp>
#include <lxu_select.hpp>
#include "common.h"

#define CACHE_CLEAR_SERVER_NAME		"cache.clear"

/*
*	CCommandClearCache is a command for clearing data in VDB voxel cache.
*	The command is usually not needed, but an option to force the recreation of next voxel
*	for special cases.
*/

class CCommandClearCache : public CLxBasicCommand
{
public:
	static
	void initialize ()
	{
		CLxGenericPolymorph		*srv;

		srv = new CLxPolymorph <CCommandClearCache>;
		srv->AddInterface(new CLxIfc_Command			<CCommandClearCache>);
		srv->AddInterface(new CLxIfc_Attributes			<CCommandClearCache>);
		srv->AddInterface(new CLxIfc_AttributesUI		<CCommandClearCache>);
		srv->AddInterface(new CLxIfc_StaticDesc			<CCommandClearCache>);

		thisModule.AddServer (CACHE_CLEAR_SERVER_NAME, srv);
	}

		int
	basic_CmdFlags ()		LXx_OVERRIDE;

		bool
	basic_Enable (
		CLxUser_Message	&msg)	LXx_OVERRIDE;

		void
	cmd_Execute (
		unsigned	 flags)	LXx_OVERRIDE;


	static LXtTagInfoDesc	 descInfo[];
};


#endif
