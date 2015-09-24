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

#ifndef VOXELITEMCACHE_H_
#define VOXELITEMCACHE_H_

#include "OpenVDBItem.h"
#include <map>
#include <list>
#include <string>
#include <lx_mesh.hpp>
#include <boost/unordered_map.hpp>

using namespace std;

/*
*	A global map to record all active OpenVDBItems, used for detecting
*	if a OpenVDBItem is still valide on cache layer and for VoxelItemSurface only. 
*	This boostMap is only for construction/desctruction of VoxelItemSurface.
*
*	The big picture is, action context also needs creation for bounding box
*	hit test in 3D viewport. However, we only create OpenVDBItem in evaluation context
*	in modifier for efficiency.
*/

typedef boost::unordered_map<const OpenVDBItem*, int> BoostMap;

/*
*	Key for caching VDBVoxel data
*/

class VoxelItemKey
{
    public:
	VoxelItemKey ():
		base (0), mesh (0){};

	unsigned base;
	unsigned mesh;

		friend bool
	operator == (
		const VoxelItemKey	&first,
		const VoxelItemKey	&other)
	{
		return (first.base == other.base) && (first.mesh == other.mesh);
	}

		bool
	operator < (
		const VoxelItemKey	&key2) const
	{
		if (this->base < key2.base)
			return true;
		else if (key2.base == this->base) {

			if (this->mesh < key2.mesh)
				return true;
		}
		
		return false;
	}
};

/*
*	Values/states for caching VDB voxel data,
*	some of them will be computed into VoxelItemKey
*	for cache hitting test.
*
*/

class VoxelItemInfo
{
    public:
	VoxelItemInfo ():
		vdbDataname (""), voxelSize (0), halfWidth (0), radius (0), time (0), mesh (NULL){};
	string		 vdbDataname;
	float		 voxelSize;
	float		 halfWidth;
	float		 radius;
	float		 time;
	CLxUser_Mesh	 mesh;

		friend bool
	operator == (
		const VoxelItemInfo	&first,
		const VoxelItemInfo	&other)
	{
		return (first.vdbDataname == other.vdbDataname) && (first.voxelSize == other.voxelSize) && (first.halfWidth == other.halfWidth) && (first.time == other.time) && (first.radius == other.radius);
	}
		bool
	operator < (
		const VoxelItemInfo	&key2) const
	{
		if (this->vdbDataname < key2.vdbDataname)
			return true;
		else if (key2.vdbDataname == this->vdbDataname) {

			if (this->voxelSize < key2.voxelSize)
				return true;
			else if (key2.voxelSize == this->voxelSize) {

				if (this->halfWidth < key2.halfWidth)
					return true;
				else if (key2.halfWidth == this->halfWidth) {

					if (this->radius < key2.radius)
						return true;
					else if (key2.radius == this->radius) {

						if (this->time < key2.time)
							return true;
					}
				}
			}
		}

		return false;
	}
};

class VoxelItemCache
{
    public:
	VoxelItemCache ():
		extraFeatureNum(0), m_openVDBItem(0), m_memUsage(0), m_refCount(0), m_cached(0){}

		inline bool
	Release ()
	{
		if (this->m_refCount <= 0 && this->m_openVDBItem != NULL) {
#if LOG_ON
			printf ("Release VoxelItem Cache %p\n", m_openVDBItem);
#endif
			delete this->m_openVDBItem;
			this->m_openVDBItem = 0;
			return true;
		}
		else
			return false;
	}

		inline void
	ForceRelease ()
	{
		this->m_refCount = 0;
		Release ();
	}

		inline bool
	IsCached (){ return m_cached;}

		bool
	cacheSave (OpenVDBItem	*item)
	{
		m_openVDBItem = item;
		m_memUsage = m_openVDBItem->getMemoryUsage ();
#if LOG_ON
		printf ("cacheSave %p ref %d\n", m_openVDBItem, m_refCount);
#endif
		return true;
	}
		OpenVDBItem*
	cacheLoad () const
	{
#if LOG_ON
		printf ("cacheLoad %p ref %d\n", m_openVDBItem, m_refCount);
#endif
		return m_openVDBItem;
	}

		void
	ApplyXFRM (
		const LXtMatrix4	&XFRM);
	/*
	*	Additional features
	*	Currently, we only support scalar type (e.g., temperature, density)
	*	Store as LXi_VMAP_WEIGHT
	*/
	unsigned			 extraFeatureNum;
	
    protected:
	// can be only called by the policy
		inline void
	Unused()
	{
		this->m_refCount--;
#if LOG_ON
		printf ("Unused %p ref %d\n", m_openVDBItem, m_refCount);
#endif
	}
		inline void
	Used(
		int times = 1)
	{
		this->m_refCount = this->m_refCount + times;
#if LOG_ON
		if (m_openVDBItem != NULL)
			printf ("Used %p ref %d\n", m_openVDBItem, m_refCount);
		else
			printf ("A new openVDBItem is about to create\n");
#endif
	}

	OpenVDBItem			*m_openVDBItem;
	unsigned			 m_memUsage;
	int				 m_refCount;
	bool				 m_cached;

    private:
	friend class			 VoxelItemCachePolicy;
};

class VoxelItemCachePolicy
{
    public:
	static VoxelItemCachePolicy* getInstance ();
	~VoxelItemCachePolicy ();

		void
	setCacheSize(
		const size_t	&sizeInByte)
	{
		g_CacheMax = (sizeInByte > g_CacheUsed)? sizeInByte : g_CacheUsed;
	}
	/*
	*	if hits, return true, "cache" would be pointed to the item 
	*	that has been cached, ref_count++, else, return false, "cache" 
	*	would be the address for the new item in the cache system.
	*/
		bool
	cacheHits (
		const VoxelItemKey	&key,
		VoxelItemCache		**cache);
	/*
	*	update the cache used, release the lowest priority items.
	*	This function must be called just after new item is assigned.
	*	It will return false, if cache is not big enough.
	*	cacheUpdates won't do anything if cache is not full.
	*/
		bool
	cacheUpdates();

	/*
	*	unreference the previous model
	*/
		void
	MarkCacheUnused (
		const VoxelItemKey	&status);
	/*
	*	Make used item from action context.
	*	Check if any items in the LazyFreeItems are used later by the action context.
	*/
		void
	MarkUsedByAction (
		const OpenVDBItem	*item);

	/*
	*	Make unused item from action context.
	*	Check if any items in the LazyFreeItems are used later by the action context.
	*/
		void
	MarkUnusedByAction (
		const OpenVDBItem	*item);

	/*
	*	The same as MarkUsedByAction, but it returns true 
	*	if the item is waiting to be deleted in the LazyFreeItems
	*/
		bool
	IsToDelete (
		const OpenVDBItem	*item);

	/*
	*	Clean the cache in this entity, if force is true, all stuff 
	*	will be cleaned evenif the reference count is not 0
	*/
		void
	CleanCache (
		bool			 force);

		void
	LazyCleanCache (
		bool			 force);

	/*
	*	Execute all lazy actions now
	*/
		void
	Flush ();

    protected:
	class VoxelItemInfoComparator
	{
	    public:
			bool
		operator() (
			const VoxelItemInfo	&key1,
			const VoxelItemInfo	&key2) const
		{
			if (key1.vdbDataname < key2.vdbDataname)
				return true;
			else if (key2.vdbDataname < key1.vdbDataname)
				return false;

			if (key1.voxelSize < key2.voxelSize)
				return true;
			else if (key2.voxelSize < key1.voxelSize)
				return false;

			if (key1.halfWidth < key2.halfWidth)
				return true;
			else if (key2.halfWidth < key1.halfWidth)
				return false;

			if (key1.radius < key2.radius)
				return true;
			else if (key2.radius < key1.radius)
				return false;

			if (key1.time < key2.time)
				return true;
			else if (key2.time < key1.time)
				return false;

			return false;
		}
	};

	class VoxelItemKeyComparator
	{
	public:
			bool
		operator() (
			const VoxelItemKey	&key1,
			const VoxelItemKey	&key2) const
		{
			if (key1.base < key2.base)
				return true;
			else if (key2.base < key1.base)
				return false;

			if (key1.mesh < key2.mesh)
				return true;
			else if (key2.mesh < key1.mesh)
				return false;

			return false;
		}
	};

	/*
	*	some utility functions
	*/
		void
	evalMemUsage ();

	/*
	*	singleton and the flag
	*/
	static bool			 instanceFlag;
	static VoxelItemCachePolicy	*cachePolicy;

	/*
	*	cache info
	*/
	map <VoxelItemKey,
	VoxelItemCache,
	VoxelItemKeyComparator>	 g_ItemCache;
	list <VoxelItemKey>		 g_priorityList;
	list <VoxelItemCache>		 g_LazyFreeItems;
	// A global map to record active OpenVDBItems created in evaluation but used in action, used for reference counting.
	BoostMap			 g_openVDBItems;
	size_t				 g_CacheUsed;
	size_t				 g_CacheMax;
	bool				 g_WaitForClean;
	bool				 g_LazyCleanForce;
	bool				 g_UsedByActionContext;

    private:
	VoxelItemCachePolicy ();
};

#endif