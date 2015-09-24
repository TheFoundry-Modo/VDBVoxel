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

#include "VoxelItemCache.h"

#include <stdio.h>

bool VoxelItemCachePolicy::instanceFlag = false;
VoxelItemCachePolicy* VoxelItemCachePolicy::cachePolicy = NULL;

VoxelItemCachePolicy::VoxelItemCachePolicy ()
{
	g_CacheMax = 0;	// by default, disable cache.
	g_CacheUsed = 0;
	g_ItemCache.clear ();
	g_WaitForClean = false;
	g_LazyCleanForce = false;
	g_UsedByActionContext = false;
}

VoxelItemCachePolicy::~VoxelItemCachePolicy ()
{
	if (instanceFlag) {
		delete cachePolicy;
		instanceFlag = false;
	}
}
	VoxelItemCachePolicy* 
VoxelItemCachePolicy::getInstance ()
{
	if (!instanceFlag) {
		cachePolicy = new VoxelItemCachePolicy ();
		instanceFlag = true;
		return cachePolicy;
	}
	else {
		return cachePolicy;
	}
}

	void
VoxelItemCachePolicy::MarkCacheUnused (
	const VoxelItemKey	&status)
{
	map<VoxelItemKey, VoxelItemCache, VoxelItemKeyComparator>::iterator l_it;
	l_it = g_ItemCache.find (status);

	if (l_it != g_ItemCache.end ()) {
		l_it->second.Unused ();
		/*
		* time to remove the cache deficit
		*/
		if (l_it->second.m_refCount <= 0
			&& g_CacheUsed > g_CacheMax) {
			g_CacheUsed -= l_it->second.m_memUsage;
			g_LazyFreeItems.push_back (l_it->second);
			g_ItemCache.erase (status);
			std::list <VoxelItemKey>::iterator findIter = std::find (g_priorityList.begin (), g_priorityList.end (), status);
			g_priorityList.erase (findIter);
		}
	}
}
	bool
VoxelItemCachePolicy::cacheUpdates()
{
	map <VoxelItemKey, VoxelItemCache, VoxelItemKeyComparator>::iterator l_it;

	/*
	* update the memory in use
	*/
	evalMemUsage();

	/*
	* the latest cache is pushed front, so the first item can not be released
	*/
	while (g_CacheUsed >= g_CacheMax) {

		if (g_priorityList.size() < 2)
			return false;

		VoxelItemKey key = *(--g_priorityList.end ());
		l_it = g_ItemCache.find (key);
		/*
		* safe code: if a item is still in use, we can not delete
		*/
		if (l_it->second.m_refCount > 0) {
			return false;
		}
		else {
			g_CacheUsed -= l_it->second.m_memUsage;
			g_LazyFreeItems.push_back (l_it->second);
			g_ItemCache.erase (key);
			g_priorityList.pop_back ();
		}
		
	}

	return true;
}
	bool
VoxelItemCachePolicy::cacheHits (
	const VoxelItemKey	&key,
	VoxelItemCache		**cache)
{
	/*
	* search in the cache [cache policy]
	*/
	map <VoxelItemKey, VoxelItemCache, VoxelItemKeyComparator>::iterator l_it;
	l_it = g_ItemCache.find (key);

	/*
	* set Cached flag
	*/
	LxResult isValide= LXe_FALSE;

	/*
	* reference count for the record updated
	*/
	int	oldRefCount = 0;

	if (l_it != g_ItemCache.end ()) {

		/*
		 * prepare for updating the priority
		 */
		std::list <VoxelItemKey>::iterator findIter = std::find (g_priorityList.begin (),g_priorityList.end (), l_it->first);
		g_priorityList.erase (findIter);

		/*
		* load from the cache
		*/
		cache[0] = &(*l_it).second;
		cache[0]->m_cached = true;

		isValide = LXe_TRUE;
#if 0
		printf ("hit %u %u\n", key.base, key.mesh);
#endif
	}
#if 0
	else
		printf ("Miss %u %u\n", key.base, key.mesh);
#endif

	if (isValide == LXe_TRUE) {

		l_it->second.Used ();
		g_priorityList.push_front (key);

		return true;
	} else {

		/*
		*	Cache miss, we have to make a new item to cache
		*	note that, the item is empty at the moment.
		*/

		g_ItemCache.insert (std::pair <VoxelItemKey, VoxelItemCache> (key, VoxelItemCache ()));
		cache[0] = &g_ItemCache[key];
		l_it = g_ItemCache.find (key);
		l_it->second.Used (oldRefCount + 1); // take into account both new and update
		g_priorityList.push_front (key);
		return false;
	}
}
	void
VoxelItemCachePolicy::evalMemUsage ()
{
	g_CacheUsed = 0;

	map<VoxelItemKey, VoxelItemCache, VoxelItemKeyComparator>::iterator l_it;

	for(l_it = g_ItemCache.begin(); l_it != g_ItemCache.end(); l_it++) {
		g_CacheUsed += l_it->second.m_memUsage;
	}
}

	void
VoxelItemCachePolicy::CleanCache (
	bool	force)
{

	// Update the memory in use

	evalMemUsage();
	list <VoxelItemKey>::iterator p_it;

	for (p_it = g_priorityList.begin(); p_it != g_priorityList.end();) {

		VoxelItemKey key = *p_it;
		map <VoxelItemKey, VoxelItemCache, VoxelItemKeyComparator>::iterator l_it = g_ItemCache.find (key);

		// Check if it needs to be freed.
		if (l_it->second.m_refCount <= 0 || force) {

			// We can only free the object when it is not held by the action context. Otherwise it will crash.
			if (g_openVDBItems.find (l_it->second.m_openVDBItem) == g_openVDBItems.end ()) {
				g_CacheUsed -= l_it->second.m_memUsage;
				l_it->second.ForceRelease();
				g_ItemCache.erase (key);
				p_it = g_priorityList.erase(p_it);
			}
			else {
#if LOG_ON
				printf ("Can not delete in CleanCache: %p is currently used by action context\n",l_it->second.m_openVDBItem);
#endif
				p_it++;
			}
		}
		else
			p_it++;
	}
}

	void
VoxelItemCachePolicy::LazyCleanCache (
	bool	force)
{
	g_WaitForClean = true;
	g_LazyCleanForce = force;
}

	void
VoxelItemCachePolicy::Flush()
{
	if (g_WaitForClean) {

		CleanCache (g_LazyCleanForce);
		g_WaitForClean = false;
		// cleanCache encloses g_LazyFreeItems,so we have to clear it afterwards.
		g_LazyFreeItems.clear();
	} else {

		std::list <VoxelItemCache>::iterator findIter = g_LazyFreeItems.begin();

		// You have to increment the iterator first and then remove the previous element, since erase invalidates the iterator.
		while (findIter != g_LazyFreeItems.end()) {

			// look into the action openvdbitem list, if they are not brought by action, we can safely delete them.
			if (g_openVDBItems.find (findIter->m_openVDBItem) == g_openVDBItems.end ()) {

				// try to release the memory
				if (findIter->Release()) {
					// if it is freed, we remove it from the lazy delete list.
					findIter = g_LazyFreeItems.erase(findIter);
				}
				else {
					findIter++;
				}
			} else {
#if LOG_ON
				printf ("Can not delete in Flush: %p is currently used by action context\n",findIter->m_openVDBItem);
#endif
				findIter++;
			}
		}
	}
}

	void
VoxelItemCachePolicy::MarkUsedByAction (
	const OpenVDBItem	*item)
{
	if (item == NULL)
		return;

	BoostMap::iterator it = g_openVDBItems.find (item);
	if (it != g_openVDBItems.end ()) {
		it->second = it->second + 1;
#if LOG_ON
		printf ("[%d]MarkUsedByAction %p %d\n",g_openVDBItems.size(), item, it->second);
#endif
	}
	else {
		g_openVDBItems.insert (BoostMap::value_type (item, 1));
#if LOG_ON
		printf ("[%d]MarkUsedByAction %p %d\n",g_openVDBItems.size(), item, 1);
#endif
	}

	return;
}

	void
VoxelItemCachePolicy::MarkUnusedByAction (
	const OpenVDBItem	*item)
{
	if (item == NULL)
		return;
	
	BoostMap::iterator it = g_openVDBItems.find (item);
	if (it != g_openVDBItems.end ()) {
		it->second = it->second - 1;

#if LOG_ON
		printf ("[%d]MarkUnusedByAction %p %d\n",g_openVDBItems.size(), item, it->second);
#endif

		if (it->second == 0)
			g_openVDBItems.erase (it);
	}
	else {

#ifdef LOG_ON
		printf ("OpenVDBItem %p is unused but never used before!\n", item);
#endif
	}

	return;
}
	void
VoxelItemCache::ApplyXFRM (
	const LXtMatrix4	&xfm)
{
	if (m_openVDBItem != NULL)
		m_openVDBItem->applyTransfrom (xfm);

}
