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

#ifndef VOXELITEMPART_H
#define	VOXELITEMPART_H

#include <lx_tableau.hpp>
#include <lx_vertex.hpp>
#include <lx_item.hpp>
#include <vector>
#include <map>
#include <list>
#include "OpenVDBItem.h"
#include "VoxelItemChannel.h"

// this number is fixed in current version of SDK
// this feature is the feature of vertex not vdb voxel.
#define TOTAL_FEATURE_COUNT 4

class OpenVDBCreationStrategy;
class VDBVoxelChannels;
class VoxelItemInfo;
class VoxelItemCache;

using std::string;
using std::vector;
using std::map;
using std::list;

/*
*	VoxelItemPart encapsulates shared functions for VoxelItemBin/Element/Surface
*/

class VoxelItemPart
{
    public:
	// data transfer
		void
	init (
		unsigned		 gridNo,
		OpenVDBItem		*openVDBItem);

    protected:

		void
	MarkCacheUnused (
		string			 name);

    protected:
	VoxelItemPart ();
	~VoxelItemPart ();
		LxResult
	Bound (
		LXtTableauBox		 bbox);

		void
	SetBBoxFromTBox (
		LXtBBox			*bbox,
		LXtTableauBox		 tBox);

		LxResult
	Sample (
		const LXtTableauBox	 bbox,
		float			 scale,
		ILxUnknownID		 trisoup);

		LxResult
	FeatureByIndex (
		LXtID4			 type,
		unsigned		 index,
		const char		**name);

		unsigned
	FeatureCount (
		LXtID4			 type);

		LxResult
	SetVertex (
		ILxUnknownID		 vdesc);
	/*
	*	return whether there is a valid vdb model.
	*	include:
	*	1, symbol detection, need create or not, how to create
	*	2, cache policy
	*/
		LxResult
	loadOpenVDBModel (
		VDBVoxelChannels		*creationInfo,
		OpenVDBCreationStrategy		*strategy);

		void
	UpdatePriority (
		const string			&currentSourceName);

		void
	LoadFromCache (
		const VoxelItemCache		*itemCache);

	/*
	*	indexes for vertex feature subscripts.
	*/
	enum
	{
		FEATURE_POSITION,
		FEATURE_OBJECT_POSITION,
		FEATURE_NORMAL,
		FEATURE_VELOCITY,
		BASE_FEATURE_COUNT
	};
	CLxUser_TableauVertex		 vrt_desc;
	vector<int >			 m_f_pos;
	int				 m_gridNo;
	unsigned			 m_f_size;

	// from the cache
	OpenVDBItem			*m_openVDBItem;
	vector<vector<double> >		 m_bbox;
	vector<bool >			 m_bboxValid;
	unsigned			 m_extraFeatureNum;
};

#endif