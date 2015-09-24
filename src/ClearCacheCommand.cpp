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

#include "ClearCacheCommand.h"
#include "VoxelItemCache.h"

	int
CCommandClearCache::basic_CmdFlags ()
{
	return LXfCMD_MODEL;
}
	bool
CCommandClearCache::basic_Enable (
	CLxUser_Message &msg)
{
	return true;
}

	void
CCommandClearCache::cmd_Execute (
	unsigned	 flags)
{
	LXtID4				 selID_item;
	CLxUser_SelectionService	 srv_sel;
	void				*pkt;

	/*
	*	Use selection system to get current VDBVoxel Item
	*	for executing the command.
	*/

	selID_item = srv_sel.LookupType (LXsSELTYP_ITEM);
	pkt = srv_sel.Recent (selID_item);

	/*
	*	The cmd_Execute of the clear cache command only sets a cache clear flag to the cachePolicy,
	*	instead of cleaning the cache immediately. When the context and work flow are suitable, the 
	*	cache clear will be performed.
	*/

	if (pkt != NULL) {
		VoxelItemCachePolicy	*cachePolicy = VoxelItemCachePolicy::getInstance ();
		cachePolicy->LazyCleanCache (false);
	}
}

	LXtTagInfoDesc
CCommandClearCache::descInfo[] = {
	{ 0 }
};
