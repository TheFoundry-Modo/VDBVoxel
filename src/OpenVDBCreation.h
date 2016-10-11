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

#ifndef OPENVDBCREATION_H
#define OPENVDBCREATION_H

#include <lx_item.hpp>
#include <lx_mesh.hpp>
#include "OpenVDBItem.h"
#include "VoxelItemCache.h"
#include "VDBVoxelItem.h"
#include "SourceItem.h"

/*
*	Creator classes for creating OpenVDBItem from input source or file.
*/

class OpenVDBCreator
{
    public:
		virtual
	~OpenVDBCreator (){}

		virtual void
	Init (VDBVoxelChannels	*creationInfo) {}

		virtual OpenVDBItem *
	Alloc (
		VDBVoxelChannels	*creationInfo,
		bool			&isValid) = 0;

		virtual VoxelItemKey
	GenKey (
		const VDBVoxelChannels	*creationInfo);
};

class OpenVDBCreateFromFile :
	public OpenVDBCreator
{
    public:
	OpenVDBCreateFromFile (CLxUser_Item		 selfItem);

		virtual OpenVDBItem *
	Alloc (
		VDBVoxelChannels	*creationInfo,
		bool			&isValid);

		virtual VoxelItemKey
	GenKey (
		const VDBVoxelChannels	*creationInfo);

    protected:

		bool
	GetCurrentFileName (
		unsigned	frame,
		unsigned	start,
		const string	&pattern,
		string		&filename);

	CLxUser_Item	m_item;

};

class OpenVDBCreateFromMesh :
	public OpenVDBCreator
{
    public:
	OpenVDBCreateFromMesh (CLxUser_Item		 sourceItem);

		virtual OpenVDBItem *
	Alloc (
		VDBVoxelChannels	*creationInfo,
		bool			&isValid);

		virtual VoxelItemKey
	GenKey (
		const VDBVoxelChannels	*creationInfo);

    protected:

	/*
	*	this is set when an item attachs the VoxelItem in the modifier.
	*	we need this to evaluated deformed mesh by lookup the "mesh" channel.
	*/

	CLxUser_Item	m_srcitem;
};

class OpenVDBCreateFromParticle :
	public OpenVDBCreator
{
    public:
		virtual void
	Init (VDBVoxelChannels	*creationInfo);

		virtual OpenVDBItem *
	Alloc (
		VDBVoxelChannels	*creationInfo,
		bool			&isValid);

		virtual VoxelItemKey
	GenKey (
		const VDBVoxelChannels	*creationInfo);

    protected:
		void
	SampleParticles (
		VDBVoxelChannels	*creationInfo,
		CParticleList		&pa);

	CParticleList			m_pa;
};


class OpenVDBCreationStrategy
{
    protected:
	OpenVDBCreator	*m_openVDBCreator;

    public:
	OpenVDBCreationStrategy () : m_openVDBCreator(0){}
	~OpenVDBCreationStrategy () {
		if (m_openVDBCreator != NULL)
		{
			/*
			* We can not delete the stuff here, since it is allocated in modifier and no idea how many times will be used.
			* The best solution is let the modifier attach a reference counter for it.
			*/
			//delete m_openVDBCreator;
			m_openVDBCreator = NULL;
		}
	}
		void
	SetCacheBudget (
		const size_t	&sizeInBytes);

		void
	SetCreator (
		OpenVDBCreator	*creator)
	{
		if (m_openVDBCreator != NULL)
			delete m_openVDBCreator;

		m_openVDBCreator = creator;
	}

		void
	Destroy (
		VDBVoxelChannels	*info)
	{
		VoxelItemCachePolicy	*cachePolicy = VoxelItemCachePolicy::getInstance ();
		VoxelItemKey	keys;

		// if key is not cached in info, we have to recompute it.
		if (info->hashKey.base == 0) {
			m_openVDBCreator->Init(info);
			keys = m_openVDBCreator->GenKey(info);
		} else {
			keys.base = info->hashKey.base;
			keys.mesh = info->hashKey.mesh;
		}
		cachePolicy->MarkCacheUnused (keys);
	}

		VoxelItemCache *
	Create (
		VDBVoxelChannels	*creationInfo,
		const VoxelItemInfo	&info)
	{
		VoxelItemCachePolicy	*cachePolicy = VoxelItemCachePolicy::getInstance ();
		VoxelItemCache		*cache = NULL;
		OpenVDBItem		*vdbItem;
		bool			 isValid;
		VoxelItemKey		keys;

		m_openVDBCreator->Init(creationInfo);
		keys = m_openVDBCreator->GenKey (creationInfo);
		creationInfo->hashKey = keys;

		unsigned cacheSize = VoxelItemPref::getInstance()->CacheSize();
		unsigned cacheMode = VoxelItemPref::getInstance()->CacheMode();
		
		if (cacheSize <= 0 || cacheMode == 0) {
			cachePolicy->CleanCache (true);
		}
		// It is safe to flush memory operations here, since modo is requesting for new data and thus old data has been deprecated.
		cachePolicy->Flush();

		if (!cachePolicy->cacheHits (keys, &cache)) {
			try {
				vdbItem = m_openVDBCreator->Alloc (creationInfo, isValid);
				// only cache when the vdbItem is valid
				if (vdbItem != NULL) {
					vdbItem->applyFilter (
						creationInfo->cv_filtIter, creationInfo->cv_filtRadius,
						creationInfo->cv_filtMinMask, creationInfo->cv_filtMaxMask,
						creationInfo->cv_filtInvertMask, NULL, creationInfo->cv_filtType);

					cache->cacheSave (vdbItem);
					cachePolicy->cacheUpdates ();
				}
				else {
					cachePolicy->MarkCacheUnused (keys);
					return NULL;
				}
			}
			catch (...) {
				cachePolicy->MarkCacheUnused (keys);
				return NULL;
			}

			if (!isValid) {
				cachePolicy->MarkCacheUnused (keys);
				return NULL;
			}
		}

		return cache;
	}
};

#endif