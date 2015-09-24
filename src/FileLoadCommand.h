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

#ifndef FILELOADCOMMAND_H
#define FILELOADCOMMAND_H

#include <lx_plugin.hpp>
#include <lxu_command.hpp>
#include <lx_command.hpp>
#include <lx_io.hpp>
#include <lx_seltypes.hpp>
#include <lxu_select.hpp>
#include "common.h"

#define FILE_LOAD_SERVER_NAME		"file.browse"
#define ARG_FILENAME			"filename"
#define ARGi_FILENAME			0


/*
*	Implement the command class. This inherits from CLxBasicCommand,
*	which is a wrapper class that does a lot of the heavy lifting
*	when implementing a command.
*
*	CCommandLoadFile is a command for loading a VDB file or a sequence of VDB files.
*	It contains a sub command for openning a file dialog.
*	The feature list of a VDB file is also loaded here.
*/

class CCommandLoadFile : public CLxBasicCommand
{
    public:
	static 
		void initialize ()
	{
		CLxGenericPolymorph		*srv;

		srv = new CLxPolymorph <CCommandLoadFile>;
		srv->AddInterface(new CLxIfc_Command			<CCommandLoadFile>);
		srv->AddInterface(new CLxIfc_Attributes			<CCommandLoadFile>);
		srv->AddInterface(new CLxIfc_AttributesUI		<CCommandLoadFile>);
		srv->AddInterface(new CLxIfc_StaticDesc			<CCommandLoadFile>);

		thisModule.AddServer (FILE_LOAD_SERVER_NAME, srv);
	}

	CCommandLoadFile ();

	int		
		basic_CmdFlags ()	LXx_OVERRIDE;
	bool		
		basic_Enable (
		CLxUser_Message	&msg)	LXx_OVERRIDE;
	void		
		cmd_Interact ()		LXx_OVERRIDE;
	void		
		cmd_Execute (
		unsigned	 flags)	LXx_OVERRIDE;

	static LXtTagInfoDesc	 descInfo[];
};

#endif