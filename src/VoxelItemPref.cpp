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

#include "VoxelItemPref.h"
#include "common.h"
#include <lxu_prefvalue.hpp>

VoxelItemPref	*VoxelItemPref::m_single = NULL;

VoxelItemPref::VoxelItemPref () : m_cacheMode(0), m_cacheSize(0)
{

}

VoxelItemPref::~VoxelItemPref ()
{
	if (m_single) {
		delete m_single;
		m_single = NULL;
	}
}

void VoxelItemPref::init ()
{

	CLxReadUserValue rpv;
	if ( rpv.Query (PREF_CACHE_MODE))
		m_cacheMode = static_cast<unsigned> (rpv.GetInt());
	if ( rpv.Query (PREF_CACHE_SIZE))
		m_cacheSize = static_cast<unsigned> (rpv.GetInt());
}

const VoxelItemPref* VoxelItemPref::getInstance ()
{
	if (m_single == NULL) {

		m_single = new VoxelItemPref();
		m_single->init();
	}

	return m_single;
}