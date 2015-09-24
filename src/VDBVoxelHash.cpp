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

#include "VDBVoxelHash.h"

template <typename Type>
	unsigned
HashBitBlob (
	unsigned hash,
	Type	 value)
{
	unsigned tmp = *reinterpret_cast<unsigned const *>(&value);

	hash = hash * FNV_PRIME;
	hash = hash ^ tmp;

	return hash;
}

/*
*	Compute Hash for triangles.
*	This is used for creation from meshes, if cache mode "full"
*/

	unsigned
VDBVoxelFNV::ComputeHashCode(
	CLxLoc_Point	 pointAccessor,
	size_t		 pointNum)
{
	unsigned hash = FNV_BASE;

	for (unsigned i = 0; i < pointNum; i++) {
		LXtFVector xyz;
		pointAccessor.SelectByIndex (i);
		pointAccessor.Pos (xyz);

		hash = HashBitBlob<float> (hash, xyz[0]);
		hash = HashBitBlob<float> (hash, xyz[1]);
		hash = HashBitBlob<float> (hash, xyz[2]);
	}

	return( hash);
}

/*
*	Compute Hash for current channel states.
*	This is used for all creation types, if cache mode "fast",
*	and for creation from files and particles, if cache mode "full".
*/

	unsigned
VDBVoxelFNV::ComputeHashCode(
	VDBVoxelChannels	 info,
	bool			 isFile)
{
	unsigned hash = FNV_BASE;

	if (isFile) {

		for (unsigned i = 0; i < info.cv_fileName.size(); i++)
			hash = HashBitBlob<char> (hash, info.cv_fileName[i]);

	} else {

		for (unsigned i = 0; i < info.cv_dataName.size(); i++)
			hash = HashBitBlob<char> (hash, info.cv_dataName[i]);
	}

	hash = HashBitBlob<float> (hash, (float)info.voxelSize);
	hash = HashBitBlob<float> (hash, (float)info.cv_halfWidth);
	hash = HashBitBlob<float> (hash, (float)info.cv_radius);
	hash = HashBitBlob<float> (hash, (float)info.time);

	if (info.cv_filtType != 0) {

		hash = HashBitBlob <int>(hash, (int)info.cv_filtType);
		hash = HashBitBlob <int>(hash, (int)info.cv_filtRadius);
		hash = HashBitBlob <int>(hash, (int)info.cv_filtIter);
		hash = HashBitBlob <int>(hash, (int)info.cv_filtInvertMask);

		hash = HashBitBlob <float>(hash, (float)info.cv_filtMaxMask);
		hash = HashBitBlob <float>(hash, (float)info.cv_filtMinMask);
	}

	return( hash);
}