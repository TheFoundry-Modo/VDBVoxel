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

#ifndef VERIFICATION_H
#define VERIFICATION_H

#include "OpenVDBItem.h"

/*
*	This is a temp fix for some unknown multi-thread crashing issues. 
*	Even if the verification is not expensive, we have to remove it ASAP.
*	only needs to test on m_openVDBItem, when it is being read.
*/

namespace VoxelItem {

	inline bool IsValid (OpenVDBItem *vdbItem)
	{
		// it should never returns false, or there are bugs
		return OpenVDBItem::isValid (vdbItem);
	}
}


#endif