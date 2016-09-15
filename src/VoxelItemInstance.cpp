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

#include <lx_action.hpp>
#include <lxw_draw.hpp>
#include <lxw_locator.hpp>
#include "common.h"
#include "VoxelItemInstance.h"
#include "VoxelItemSurface.h"
#include "VoxelItem.h"
#include "ParticleAttach.h"
#include "VoxelItemChannel.h"
#include "Verification.h"

/*
*	Impl Voxel Source Interface
*
*	Voxel source interface is not needed for VoxelItem Instance now.
*	It is implemented in a lighter object called VDBVoxel, which stores
*	in item channel, as changing from "IS A" to "HAS A".
*/

	LxResult
VoxelItemInstance::voxel_FeatureCount (
	unsigned	*num)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_FeatureCount (num);
	else {
		*num = 0;
		return LXe_OK;
	}
}
	LxResult
VoxelItemInstance::voxel_FeatureByIndex (
	unsigned	 index,
	const char	**name)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_FeatureByIndex (index, name);
	else {
		*name = NULL;
		return LXe_OK;
	}
}
	LxResult
VoxelItemInstance::voxel_BBox (
	LXtTableauBox bbox)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_BBox (bbox);
	else
		return LXe_OK;
}
	LxResult
VoxelItemInstance::voxel_NextPos (
	float		 currentPos,
	unsigned	 currentSegment,
	float		 stride,
	float		*segmentList,
	unsigned	*nextSegment,
	float		*nextPos)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_NextPos (currentPos, currentSegment, stride, segmentList, nextSegment, nextPos);
	else
		return LXe_OK;
}
	LxResult
VoxelItemInstance::voxel_Sample (
	const LXtFVector pos,
	unsigned	 index,
	float		*val)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_Sample (pos, index, val);
	else {
		*val = 0.0;
		return LXe_OK;
	}
}
	LxResult
VoxelItemInstance::voxel_VDBData (
	void		**ppvObj)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_VDBData (ppvObj);
	else {
		*ppvObj = NULL;
		return LXe_OK;
	}
}

	LxResult
VoxelItemInstance::voxel_RayIntersect (
	const LXtVector		 origin,
	const LXtFVector	 direction,
	unsigned		*numberOfSegments,
	float			**Segmentlist)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_RayIntersect (origin, direction, numberOfSegments, Segmentlist);
	else {
		*numberOfSegments = 0;
		return LXe_OK;
	}
}

	LxResult
VoxelItemInstance::voxel_RayRelease (
	unsigned	 numberOfSegments,
	float		**Segmentlist)
{
	if (VoxelItem::IsValid(m_openVDBItem))
		return m_openVDBItem->voxel_RayRelease (numberOfSegments, Segmentlist);
	else {
		return LXe_OK;
	}
}

	LxResult
VoxelItemInstance::pins_Initialize (
	ILxUnknownID	 item,
	ILxUnknownID	 super)
{
	if (m_item.set(item)) {
		std::string x = "-- got item ";
	}

	OpenVDBInit ();
	m_openVDBItem = NULL;
	this->listener_service.AddListener (this->inst_ifc);
	return LXe_OK;
}
	void
VoxelItemInstance::pins_Doomed (void)
{
	m_openVDBItem = NULL;
	this->listener_service.RemoveListener (this->inst_ifc);
}

	void
VoxelItemInstance::pins_Cleanup (void)
{
	VoxelItemCachePolicy	*cachePolicy = VoxelItemCachePolicy::getInstance ();
	cachePolicy->CleanCache (true);
	m_item.clear ();
}

	LxResult
VoxelItemInstance::pins_SynthName (
	char		*buf,
	unsigned	 len)
{
	std::string name (ITEM_SERVER_NAME);
	size_t count = name.size () + 1;
	if (count > len) {
		count = len;
	}
	memcpy (buf, &name[0], count);

	return LXe_OK;
}

	LxResult
VoxelItemInstance::pins_Newborn (
	ILxUnknownID	 original,
	unsigned	 flags)
{
	m_openVDBItem = NULL;
	return LXe_NOTIMPL;
}
	LxResult
VoxelItemInstance::pins_AfterLoad ()
{
	return LXe_OK;
}
	LxResult
VoxelItemInstance::tsrc_PreviewUpdate (
	int		 chanIndex,
	int		*update)
{
	*update = LXfTBLX_PREVIEW_UPDATE_SHADING;

	return LXe_OK;
}

/*
*	Based on the channel values, draw the abstract item representation
*	using the stroke drawing interface.
*/

	LxResult
VoxelItemInstance::vitm_Draw (
	ILxUnknownID		 itemChanRead,
	ILxUnknownID		 viewStrokeDraw,
	int			 selectionFlags,
	const LXtVector		 itemColor)
{
	CLxUser_ValueReference	 ref;
	CLxUser_ChannelRead	 chanRead;
	CLxLoc_StrokeDraw	 strokeDraw;
	CLxUser_Scene		 scene;
	LXtObjectID		 obj;
	LXtVector		 vertex1, vertex2, vertex_color = { 1., 1., 1. };
	unsigned		 gridNo;
	float			 lineWidth, dotWidth, sampleRate, fDensity;
	int			 strokeFlags = LXiSTROKE_ABSOLUTE;
	bool			nextExists;


	chanRead.set (itemChanRead);
	chanRead.Object (m_item, CN_VDBOBJ, ref);

	if (ref.GetObject (&obj) != LXe_OK)
		return LXe_OK;

	CLxSpawner<VDBVoxelItem>	 spawner(SPNNAME_ITEM);
	VDBVoxelItem * vdbVoxel = spawner.Cast ((ILxUnknownID)obj);
	m_openVDBItem = vdbVoxel->GetOpenVDBItem();
	lx::ObjRelease (obj);
	
	if (m_openVDBItem == NULL)
		return LXe_OK;

	VoxelItemChannel	chan;
	VoxelItemChannel::desc.chan_read (itemChanRead, m_item, &chan);

	m_openVDBItem->getFeatureID (chan.cv_featureName, gridNo);

	if (gridNo == (unsigned)-1)
		return LXe_OK;

	strokeDraw.set (viewStrokeDraw);

	lineWidth = 0.1;
	dotWidth = 2.0;

	// make sure loadOpenVDBModel == Lxe_OK
	if (chan.cv_drawFrame) {

		m_openVDBItem->genMesh (gridNo);

		strokeDraw.BeginW (LXiSTROKE_LINES, itemColor, 1.0, lineWidth);

		unsigned numQuads = m_openVDBItem->getMeshQuadNum (gridNo) / 4;
		for (unsigned i = 0; i < numQuads; i++) {
			unsigned index[4];
			m_openVDBItem->readMeshQuaIndices (gridNo, i, index);
			m_openVDBItem->readMeshPoints (gridNo, index[0], vertex1);
			strokeDraw.Vertex (vertex1, strokeFlags);

			m_openVDBItem->readMeshPoints (gridNo, index[1], vertex1);
			strokeDraw.Vertex (vertex1, strokeFlags);

			strokeDraw.Vertex (vertex1, strokeFlags);

			m_openVDBItem->readMeshPoints (gridNo, index[2], vertex1);
			strokeDraw.Vertex (vertex1, strokeFlags);

			strokeDraw.Vertex (vertex1, strokeFlags);

			m_openVDBItem->readMeshPoints (gridNo, index[3], vertex1);
			strokeDraw.Vertex (vertex1, strokeFlags);

			strokeDraw.Vertex (vertex1, strokeFlags);

			m_openVDBItem->readMeshPoints (gridNo, index[0], vertex1);
			strokeDraw.Vertex (vertex1, strokeFlags);
		}
	}

	// makesure GenVoxelSamples() is called before
	if (chan.cv_drawSample) {

		m_openVDBItem->genVoxelSamples(gridNo);

		// set the percentage of samples for display
		sampleRate = chan.cv_sampleNum;
		sampleRate = LXxCLAMP(sampleRate, 0.0f, 1.0f);

		unsigned threshold = sampleRate * 1000;
		srand (1234);

		unsigned numSample = m_openVDBItem->getSampleNum (gridNo);
		for (unsigned i = 0; i < numSample; i++) {

			// not very efficient
			unsigned random = rand () % 1000 + 1;
			if (random > threshold)
				continue;

			m_openVDBItem->readSamplePoints (gridNo, i, vertex1);
			m_openVDBItem->readSampleColors (gridNo, i, vertex_color);

			strokeDraw.BeginW (LXiSTROKE_POINTS, vertex_color, .125, dotWidth);

			strokeDraw.Vertex (vertex1, strokeFlags);
		}
	}
	if (chan.cv_drawGrid) {
		m_openVDBItem->beginTraversal(gridNo);
		do {
			nextExists = m_openVDBItem->iterNextNode (gridNo, vertex1, vertex2, fDensity);

			vertex_color[0] = fDensity; vertex_color[1] = 0; vertex_color[2] = 0;
			strokeDraw.BeginW (LXiSTROKE_FRONT_BOXES, vertex_color, .125, lineWidth);

			strokeDraw.Vertex (vertex1, strokeFlags);
			strokeDraw.Vertex (vertex2, strokeFlags);
		} while (nextExists);
	}

	return LXe_OK;
}

	LxResult
VoxelItemInstance::isurf_GetSurface (
	ILxUnknownID	 itemChanRead,
	unsigned	 morph,
	void		**ppvObj)
{
	VoxelItemSurface	*surface = src_pkg->surf_factory.Alloc(ppvObj);
	if (surface) {

		VoxelItemChannel::desc.chan_read (itemChanRead, m_item, (VoxelItemChannel *) surface);
		unsigned		 gridNo = 0;

		if (m_openVDBItem != NULL && VoxelItem::IsValid (m_openVDBItem)) {

			m_openVDBItem->getFeatureID (surface->cv_featureName, gridNo);
		}
		else
			m_openVDBItem = NULL;
#if LOG_ON
		printf ("isurf_GetSurface %p %d\n", m_openVDBItem, gridNo);
#endif

		// VDB voxel can not return any surface outside of evaluation context since too costly, however this function is needed for BBox testing, hit testing and type conversions under an action context. So the solution here is to check if the VDB voxel is waiting to be deleted. If yes, we do not create any surfaces instead, just a empty dummy.
		if (gridNo == unsigned(-1)) {
			m_openVDBItem = NULL;
			gridNo = 0;
#if LOG_ON
			printf ("isurf_GetSurface: current feature is not found in %p\n", m_openVDBItem);
#endif
		}

		// never submit wrong parameters
		surface->Initialize (src_pkg, gridNo, m_openVDBItem, 1);

	}
	else {
		return LXe_FAILED;
	}

	return LXe_OK;
}

	LxResult
VoxelItemInstance::isurf_Evaluate (
	ILxUnknownID	 attr,
	unsigned	 index,
	void		**ppvObj)
{
	unsigned		gridNo;
	VoxelItemSurface	*surface = src_pkg->surf_factory.Alloc (ppvObj);

	if (surface) {
		/* the order is defined in isurf_Prepare */
		CLxUser_Attributes	 attributes(attr);
		CLxUser_ValueReference	 ref;
		LXtObjectID		 obj;

		attributes.ObjectRO ( index, ref );
		if (ref.GetObject (&obj) != LXe_OK)
			return LXe_OK;

		VoxelItemChannel::desc.eval_read (attr, index + 1, (VoxelItemChannel *) surface);

		CLxSpawner<VDBVoxelItem>	 spawner(SPNNAME_ITEM);
		VDBVoxelItem * voxelItem = spawner.Cast ((ILxUnknownID)obj);
		m_openVDBItem = voxelItem->GetOpenVDBItem();
		lx::ObjRelease (obj);

		if (m_openVDBItem == NULL)
			gridNo = 0;
		else
			m_openVDBItem->getFeatureID (surface->cv_featureName, gridNo);
		
		surface->Initialize (src_pkg, gridNo, m_openVDBItem, 0);
	}
	else {
		return LXe_FAILED;
	}
	return LXe_OK;
}

	LxResult
VoxelItemInstance::isurf_Prepare (
	ILxUnknownID	 eval,
	unsigned	*index)
{
	//surface updates according to the change of the channels
	CLxUser_Evaluation evaluation (eval);
	index[0] = evaluation.AddChan (m_item, CN_VDBOBJ);
	VoxelItemChannel::desc.eval_attach (eval, m_item);

	return LXe_OK;
}

	LxResult
VoxelItemInstance::stag_Get (
	LXtID4		 type,
	const char	**tag)
{
	tag[0] = "Default";
	return LXe_OK;
}

/*
*	The instance's TableauSource interface allows it to place elements into the
*	tableau, in this case our voxel item element
*/
	LxResult
VoxelItemInstance::tsrc_Elements (
	ILxUnknownID		 tblx)
{
	CLxUser_Tableau		 tbx(tblx);
	CLxUser_ChannelRead	 chan;
	CLxUser_TableauShader	 shader;
	ILxUnknownID		 element;
	LxResult		 rc;
	CLxLoc_Locator		 locator;
	
	/*
	*	This is our opportunity to fetch our custom channel values.
	*/
	if (!tbx.GetChannels (chan, 0))
		return LXe_NOINTERFACE;

	if (!tbx.GetShader (shader, m_item, inst_ifc))
		return LXe_NOTFOUND;

	CLxUser_ValueReference	 ref;
	LXtObjectID		 obj;
	chan.Object (m_item, CN_VDBOBJ, ref);

	if (ref.GetObject (&obj) != LXe_OK)
		return LXe_OK;

	CLxSpawner<VDBVoxelItem>	 spawner(SPNNAME_ITEM);
	VDBVoxelItem * voxelItem = spawner.Cast ((ILxUnknownID)obj);
	m_openVDBItem = voxelItem->GetOpenVDBItem();

	lx::ObjRelease (obj);
	if (m_openVDBItem == NULL)
		return LXe_OK;

	VoxelItemChannel	 chanData;
	unsigned		 gridNo;

	VoxelItemChannel::desc.chan_read (chan, m_item, &chanData);

	m_openVDBItem->getFeatureID (chanData.cv_featureName, gridNo);

	element = src_pkg->elt_factory.Spawn ();
	if (!element)
		return LXe_FAILED;

	LXCWxOBJ(element, VoxelItemElement)->copy_channels(&chanData);
	LXCWxOBJ(element, VoxelItemElement)->init (gridNo, m_openVDBItem);

	/*
	* We also need to store the locator transform, so it can be looked
	* up later on when TableauInstance::GetTransform is called.
	*/
	if (locator.set (m_item)) {
		LXtMatrix	 xfrm;
		LXtVector	 offset;

		locator.WorldTransform (chan, xfrm, offset);

		for (unsigned i = 0; i < 3; ++i)
			LXx_VCPY(LXCWxOBJ(element, VoxelItemElement)->m_xfrm[i], xfrm[i]);

		LXx_VCPY(LXCWxOBJ(element, VoxelItemElement)->m_offset, offset);
	}

	rc = tbx.AddElement (element, shader);
	lx::UnkRelease (element);

	return rc;
}
	void 
VoxelItemInstance::sil_ChannelValue (
		const char	*action,
		ILxUnknownID	 itemObj,
		unsigned	 channel)
{
	CLxUser_Item		item (itemObj);
	/*
	* listener is global, so we have to check if the item is the one that we need.
	*/
	if (item.m_loc != m_item.m_loc)
		return;

	CLxUser_ChannelRead	 readChan;
	CSourceFinder		 src;

	if (channel == item.ChannelIndex (CN_USE_HDDA)) {

		readChan.from (item);
		int res = readChan.IValue(m_item, channel);
		if (VoxelItem::IsValid(m_openVDBItem))
			this->m_openVDBItem->setUseHDDA((res !=0));
	}
	else if (channel == item.ChannelIndex (CURRENT_FEATURE)) {

		const char*		 name;

		readChan.from (item);
		readChan.String (item, channel, &name);

		if (VoxelItem::IsValid(m_openVDBItem)) {
			unsigned gridNo;
			this->m_openVDBItem->getFeatureID (name, gridNo);
			this->m_openVDBItem->setCurrentGrid (gridNo);
		}
	}
}

	void
VoxelItemInstance::GraphChange (
	const char	*graph,
	ILxUnknownID	 item1,
	ILxUnknownID	 item2)
{
	CLxUser_Item		 source (item1);
	CLxUser_Item		 item (item2);
	CSourceFinder		 src;

	// GraphChange can be triggered by anything, such as translation, so
	if (strcmp (graph, GRAPH_VOXELITEM))
		return;

	if (m_item.m_loc != item.m_loc)
		return;

	/*
	* Update UI
	*/
	if (!src.Find (item))
		return;

	CLxUser_ChannelWrite	 chanWrite, chanWriteEdit;

	chanWrite.setupFrom (item);
	chanWrite.Set (item, CN_DATA_FILE_NAME, source.GetIdentity ().c_str ());

	const char* feature = DEFAULT_FEATURE;
	chanWrite.Set (item, FEATURE_NAME_LIST, feature);

	chanWriteEdit.from (item);
	chanWriteEdit.Set (item, CURRENT_FEATURE, feature);

	sil_ChannelValue (0, item2, 0);

}
