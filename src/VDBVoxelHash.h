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

#ifndef VDBVOXELHASH_H
#define VDBVOXELHASH_H

#include <lx_mesh.hpp>
#include "VDBVoxelItem.h"

#define FNV_PRIME 16777619u
#define FNV_BASE 2166136261u

/*
*	FNV-1 hash key generator function for generating VDBVoxel hash keys,
*	which are used for cache hit test.
*/

class VDBVoxelFNV
{

public:
		static unsigned
	ComputeHashCode (
		CLxLoc_Point	 pointAccessor,
		size_t		 pointNum);

		static unsigned
	ComputeHashCode (
		VDBVoxelChannels info,
		bool		 isFile);

};

#endif
