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

#ifndef COMMON_H
#define COMMON_H

#define ITEM_SERVER_NAME	"VDBVoxel"
#define GRAPH_VOXELITEM		"VDBVoxel.Graph"
#define SPNNAME_SURFACE		"VDBVoxel.Surf"
#define SPNNAME_ELEMENT		"VDBVoxel.Element"
#define SPNNAME_ITEM		"VDBVoxel.ITEM"
/*
* Name of Channels
*/

#define CN_DATA_FILE_NAME	"dataFileName"
#define CN_FILE_NAME		"fileName"
#define CN_DRAW_GRIDS		"drawGrids"
#define CN_DRAW_SURFACES	"drawSurfaces"
#define CN_DRAW_SAMPLES		"drawSamples"
#define CN_DRAW_SURF_FRAMES	"drawFrames"
#define CN_SAMPLE_NUMBER	"samplePercent"
#define CN_REVERSE		"reverse"
#define CN_CACHE_BUDGET		"cacheSize"
#define CN_OFFSET_TIME		"offsetTime"
#define CN_CACHE_MODE		"cacheMode"
#define CN_FILT_ITER		"filtIter"
#define CN_FILT_RADIUS		"filtRadius"
#define CN_FILT_MINMASK		"filtMinMask"
#define CN_FILT_MAXMASK		"filtMaxMask"
#define CN_FILT_INVERTMASK	"filtInvMask"
#define CN_FILT_TYPE		"filtMode"

/*
* Particle to volume
*/

#define CN_VDBOBJ		"VDBVoxel.Obj"
#define CN_RADIUS		"radius"
#define CN_VELOCITY		"velocity"
#define CN_VELOCITYSPREAD	"velocitySpread"
#define CN_VOXELRESOLUTION	"voxelResolution"
#define CN_HALFWIDTH		"halfWidth"

/*
* Implicit ones that are not visible on the UI.
*/

#define FEATURE_NAME_LIST	"featureList"
#define CURRENT_FEATURE		"currentFeature"
#define CN_USE_HDDA		"useHDDA"

/*
* Filters. The order must match the one in the cfg file aka VoxelItem.cfg
*/

#define FILT_NONE		0
#define FILT_MEAN		1
#define FILT_GAUSSIAN		2
#define FILT_MEDIAN		3

/*
* The default feature for the voxel item created from input source item
*/

#define DEFAULT_FEATURE		"default"

/*
* Settings for preferences.
*/

#define PREF_CACHE_MODE		"FinalRendering.VDBVoxel.Cache.Mode"
#define PREF_CACHE_SIZE		"FinalRendering.VDBVoxel.Cache.Size"

/*
* Additional Utilities
*/

#include <lxvmath.h>

	void inline
LXx_Normalize (LXtVector	&v)
{
	double	il = 1.0 / LXx_VLEN(v);
	LXx_VSCL(v, il);
}
	void inline
LXx_Normalize (LXtFVector	&v)
{
	float	il = 1.0 / LXx_VLEN(v);
	LXx_VSCL(v, il);
}

#define LOG_ON	0

#endif