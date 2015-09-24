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

#ifndef	VOXELITEMPREF_H
#define	VOXELITEMPREF_H

/*
*	Preferences for VoxelItem are global contants for all
*	VoxelItems (VDBVoxel items) in a scene. A restart of modo
*	is needed to take effect of any changes.
*/

class VoxelItemPref
{
private:

	VoxelItemPref ();
	void init();

	unsigned	m_cacheMode;
	unsigned	m_cacheSize;

	static VoxelItemPref	*m_single;

public:
	~VoxelItemPref ();

	unsigned CacheMode() const {return m_cacheMode;}
	unsigned CacheSize() const {return m_cacheSize;}

	static const VoxelItemPref* getInstance();
};

#endif