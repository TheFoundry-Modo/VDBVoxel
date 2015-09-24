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

#ifndef VOXELITEMCHANNEL_H
#define VOXELITEMCHANNEL_H

#include "common.h"
#include "VoxelItemPref.h"
#include <lxu_attrdesc.hpp>

/*
*	Channels of VDBVoxel Item
*/

class VoxelItemChannel
{
public:
	double		 cv_radius, cv_speed, cv_speedSpread, cv_halfWidth, cv_sampleNum, cv_offsetTime, cv_filtMinMask, cv_filtMaxMask;
	int		 cv_resolution, cv_drawGrid, cv_drawSurf, cv_drawSample, cv_drawFrame, cv_useHDDA, cv_filtIter, cv_filtRadius, cv_filtInvertMask, cv_filtType;
	std::string	 cv_fileName, cv_dataName, cv_featureName, cv_featureNameList;

	static CLxAttributeDesc		 desc;

	static void
	initialize ()
	{
		VoxelItemChannel		*chan = 0;

		static LXtTextValueHint pathHint[] = {
			0,	 "@vdb",
			-1,	 NULL
		};
		static LXtTextValueHint ragHint[] = {
			1,	 "&min",
			256,	 "&max",
			-1,	 NULL
		};

		static LXtTextValueHint radHint[] = {
			1,	 "&min",
			16,	 "&max",
			-1,	 NULL
		};

		static LXtTextValueHint iterHint[] = {
			0,	 "&min",
			32,	 "&max",
			-1,	 NULL
		};

		static LXtTextValueHint maskHint[] = {
			0,	 "&min",
			1,	 "&max",
			-1,	 NULL
		};

		static LXtTextValueHint filtHint[] = {
			FILT_NONE,	 "null",
			FILT_MEAN,	 "mean",
			FILT_GAUSSIAN,	 "gaussian",
			FILT_MEDIAN,	 "median",
			-1,	 "=VDBVoxel-FiltMode",
			0,	 NULL

		};

		desc.add_channel (CN_RADIUS,		LXsTYPE_FLOAT,   0.5, &chan->cv_radius, LXfECHAN_READ);
		desc.add_channel (CN_VELOCITY,		LXsTYPE_SPEED,   0.5, &chan->cv_speed, LXfECHAN_READ);
		desc.add_channel (CN_VELOCITYSPREAD,	LXsTYPE_SPEED,   0.5, &chan->cv_speedSpread, LXfECHAN_READ);
		desc.add_channel (CN_VOXELRESOLUTION,	LXsTYPE_INTEGER, 4,  &chan->cv_resolution, LXfECHAN_READ);
		desc.hint (ragHint);

		desc.add_channel (CN_HALFWIDTH,		LXsTYPE_FLOAT,   1.0, &chan->cv_halfWidth, LXfECHAN_READ);

		std::string tmp ("");
		desc.add_channel (CN_FILE_NAME,		LXsTYPE_FILEPATH,tmp, &chan->cv_fileName, LXfECHAN_READ);
		desc.hint (pathHint);

		desc.add_channel (CN_DATA_FILE_NAME,	LXsTYPE_STRING,  tmp, &chan->cv_dataName, LXfECHAN_READ);
		desc.add_channel (FEATURE_NAME_LIST,	LXsTYPE_STRING,  tmp, &chan->cv_featureNameList, LXfECHAN_READ);
		desc.add_channel (CURRENT_FEATURE,	LXsTYPE_STRING,  tmp, &chan->cv_featureName, LXfECHAN_READ);

		desc.add_channel (CN_DRAW_GRIDS,	LXsTYPE_BOOLEAN,   1, &chan->cv_drawGrid, LXfECHAN_READ);
		desc.add_channel (CN_DRAW_SURFACES,	LXsTYPE_BOOLEAN,   0, &chan->cv_drawSurf, LXfECHAN_READ);
		desc.add_channel (CN_DRAW_SAMPLES,	LXsTYPE_BOOLEAN,   0, &chan->cv_drawSample, LXfECHAN_READ);
		desc.add_channel (CN_DRAW_SURF_FRAMES,	LXsTYPE_BOOLEAN,   0, &chan->cv_drawFrame, LXfECHAN_READ);

		desc.add_channel (CN_SAMPLE_NUMBER,	LXsTYPE_FLOAT,   1.0, &chan->cv_sampleNum, LXfECHAN_READ);
		desc.add_channel (CN_USE_HDDA,		LXsTYPE_BOOLEAN,   0, &chan->cv_useHDDA, LXfECHAN_READ);

		desc.add_channel (CN_OFFSET_TIME,	LXsTYPE_FLOAT,   0.0, &chan->cv_offsetTime, LXfECHAN_READ);

		desc.add_channel (CN_FILT_INVERTMASK,	LXsTYPE_BOOLEAN,   0, &chan->cv_filtInvertMask, LXfECHAN_READ);

		desc.add_channel (CN_FILT_ITER,		LXsTYPE_INTEGER,   1, &chan->cv_filtIter, LXfECHAN_READ);
		desc.hint (iterHint);

		desc.add_channel (CN_FILT_RADIUS,	LXsTYPE_INTEGER,   1, &chan->cv_filtRadius, LXfECHAN_READ);
		desc.hint (radHint);

		desc.add_channel (CN_FILT_TYPE,		LXsTYPE_INTEGER,   0, &chan->cv_filtType, LXfECHAN_READ);
		desc.hint (filtHint);

		desc.add_channel (CN_FILT_MINMASK,	LXsTYPE_FLOAT,   0.0, &chan->cv_filtMinMask, LXfECHAN_READ);
		desc.hint (maskHint);
		desc.add_channel (CN_FILT_MAXMASK,	LXsTYPE_FLOAT,   1.0, &chan->cv_filtMaxMask, LXfECHAN_READ);
		desc.hint (maskHint);

	}

		void
	copy_channels (
		VoxelItemChannel		*other)
	{
		*this = *other;
	}
};

#endif
