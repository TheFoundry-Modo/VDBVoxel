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

#ifndef VDBVOXELMODIFIER_H
#define VDBVOXELMODIFIER_H

#include <lxu_modifier.hpp>
#include "common.h"
#include "VDBVoxelItem.h"
#include "OpenVDBCreation.h"
#include "VoxelItemChannel.h"

static CLxItemType		 cit_locator(LXsITYPE_LOCATOR);

/*
*	Modifier is used for defining how to return a VDBVoxelItem object.
*	A VDBVoxelItem is created only when it is used by reading from an
*	object channel of VoxelItem (aka VDBVoxel Item)
*/

class VDBVoxelModifierElement :
	public CLxItemModifierElement
{
    public:
	CLxSpawner<VDBVoxelItem>	 pp_spawn;

	VDBVoxelModifierElement() : pp_spawn(SPNNAME_ITEM) {}

	CSourceFinder		 e_src;
	CLxUser_ParticleItem	 pi_ifc;
	unsigned		 pi_index, xfrm_idx, mesh_idx, time_index;
	OpenVDBCreator		*vdb_Creator;

		void
	Attach(
		CLxUser_Evaluation	&eval,
		ILxUnknownID		 item)
	{
		eval.AddChan (item, CN_VDBOBJ, LXfECHAN_WRITE);
		VoxelItemChannel::desc.eval_attach(eval, item);
		time_index = eval.AddTime ();

		xfrm_idx = 0;
		mesh_idx = 0;

		if (e_src.Find (item) && pi_ifc.set (e_src.source_item)) {
			pi_ifc.Prepare (eval, &pi_index);
			CLxUser_Item piLoc (e_src.source_item);

			unsigned		 chanIndex;
			// Check if the input item is a mesh
			if (e_src.source_item.ChannelLookup(LXsICHAN_MESH_MESH, &chanIndex) == LXe_OK) {
				vdb_Creator = new OpenVDBCreateFromMesh (e_src.source_item);
				mesh_idx = eval.AddChan (piLoc, LXsICHAN_MESH_MESH);
			}
			else
				vdb_Creator = new OpenVDBCreateFromParticle ();

			if (piLoc.IsA (cit_locator))
				xfrm_idx = eval.AddChan (piLoc, LXsICHAN_XFRMCORE_WORLDMATRIX);
		}
		else {
			vdb_Creator = new OpenVDBCreateFromFile (item);
		}
	}

		bool
	Test(
		ILxUnknownID		 item)		LXx_OVERRIDE
	{
		return e_src.Test (item);
	}

		void
	Eval(
		CLxUser_Evaluation	&eval,
		CLxUser_Attributes	&attr)		LXx_OVERRIDE
	{
		VDBVoxelItem		*filt;
		CLxUser_ValueReference	 ref;
		ILxUnknownID		 obj;
		unsigned		 index = 1;
		CLxUser_Mesh		 mesh = NULL;

		filt = pp_spawn.Alloc (obj);
		CLxArmUnknownRef scoped_ref (obj);
		filt->c_eval.alloc ();

		VoxelItemChannel::desc.eval_read (attr, index, (VoxelItemChannel *) filt->c_eval);
		// For bug fix 45340 : VDBVoxel's Resolution Even-Odd Alternation.
		// No idea, why Even-Odd causes different result, so use only Odd.
		filt->c_eval->voxelSize = 1.0 / ( 2 * filt->c_eval->cv_resolution);
		filt->c_eval->time = attr.Float(time_index);

		if (pi_ifc.test ())
			pi_ifc.Evaluate (attr, pi_index, filt->c_eval->ptSrc);

		if (xfrm_idx) {
			CLxUser_Matrix	 mat;
			attr.ObjectRO (xfrm_idx, mat);
			mat.Get4 (filt->c_eval->xfrm);
		}
		else
			lx::Matrix4Ident (filt->c_eval->xfrm);

		if (mesh_idx && VoxelItemPref::getInstance()->CacheMode() == 2) {
			CLxUser_MeshFilter	 mfilt;
			attr.ObjectRO (mesh_idx, mfilt);
			if (!mfilt.GetMesh (mesh)) {
				mesh = NULL;
			}
		}

		filt->c_eval->mesh = mesh;
		filt->CreateVDBVoxel (vdb_Creator);

		attr.ObjectRW (0, ref);
		ref.SetObject (obj);
	}
};


class VDBVoxelModifier :
	public CLxItemModifierServer
{
    public:
		static void
	initialize()
	{
		CLxExport_ItemModifierServer <VDBVoxelModifier> (ITEM_SERVER_NAME);
	}

		const char *
	ItemType()
	{
		return ITEM_SERVER_NAME;
	}

		const char *
	GraphNames()
	{
		return GRAPH_VOXELITEM;
	}

		CLxItemModifierElement *
	Alloc(
		CLxUser_Evaluation	&eval,
		ILxUnknownID		 item)
	{
		VDBVoxelModifierElement	*elt;

		elt = new VDBVoxelModifierElement;
		elt->Attach (eval, item);
		return elt;
	}
};


#endif
