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

#ifndef SourceItem_H
#define SourceItem_H

#include <lxvmath.h>
#include <lxparticle.h>
#include <lx_particle.hpp>
#include <lxu_math.hpp>
#include <lx_tableau.hpp>
#include <lx_vertex.hpp>
#include <lx_tableau.hpp>
#include <lx_particle.hpp>
#include <lx_trisurf.hpp>
#include <lx_package.hpp>
#include <lx_item.hpp>
#include <openvdb/openvdb.h>
#include <vector>
#include "common.h"

using namespace openvdb;
using namespace std;

	static bool
FindSource (
	ILxUnknownID		 item,
	CLxUser_Item		&source)
{
	CLxUser_Scene		 scene;
	CLxUser_ItemGraph	 graph;

	return (source.set(item)
		&& scene.from(source)
		&& graph.from(scene, GRAPH_VOXELITEM)
		&& graph.RevByIndex(source, 0, source));
}

class CSourceFinder
{
public:
	CLxUser_Item			source_item;

		bool
	Find(
		ILxUnknownID		 item)
	{
		return FindSource (item, source_item);
	}
		bool
	Test(
		ILxUnknownID		 item)
	{
		CLxUser_Item		 current;

		if (!FindSource (item, current))
			return !source_item.test ();

		return (source_item.test () && current == source_item);
	}
};

/*
*	these public functions are template-based interfaces that used by the OpenVDB and needed to be implemented.
*	the code is from testcode of OpenVDB but has been modified
*/

class CSample
{
    public:
	CSample ()
	{
		LXx_VSET(pos, 0);
		LXx_VSET(vel, 0);
		size = 0;
	}
	LXtFVector		 pos;
	LXtFVector		 vel;
	float			 size;
};

class CTriangleList
{
    public:
	vector<Vec3s>	 points;
	vector<Vec3I>	 triangles;
};

class CParticleList
{
    protected:
	struct Particle {
		Vec3R	 p, v;
		Real	 r;
	};
	float			 m_RadiusScale;
	float			 m_VelocityScale;
	vector<Particle>	 m_ParticleList;

    public:
	// required typedef for bucketing
	typedef openvdb::Vec3R	 value_type;

	CParticleList(
		float rScale = 1,
		float vScale = 1)
		: m_RadiusScale(rScale), m_VelocityScale(vScale) {}

		inline void
	add(
		const CSample& sam)
	{
		Particle pa;
		LXx_VCPY(pa.p, sam.pos);
		LXx_VCPY(pa.v, sam.vel);
		pa.r = sam.size;
		m_ParticleList.push_back (pa);
	}

		inline void
	add(
		float px, float py, float pz,
		float r,
		float vx, float vy, float vz)
	{
		Particle pa;
		LXx_VSET3(pa.p, px, py, pz);
		LXx_VSET3(pa.v, vx, vy, vz);
		pa.r = r;
		m_ParticleList.push_back (pa);
	}

		CoordBBox
	getBBox(
		const GridBase	&grid)
	{
		CoordBBox	 bbox;
		Coord		&min = bbox.min(), &max = bbox.max();
		Vec3R pos;
		Real rad, invDx = 1 / grid.voxelSize()[0];

		for (size_t n = 0, e = this->size(); n<e; ++n) {
			this->getPosRad (n, pos, rad);
			const Vec3d	xyz = grid.worldToIndex (pos);
			const Real	r = rad * invDx;

			for (unsigned i = 0; i<3; ++i) {
				min[i] = math::Min (min[i], math::Floor (xyz[i] - r));
				max[i] = math::Max (max[i], math::Ceil (xyz[i] + r));
			}
		}
		return bbox;
	}
		inline size_t
	size() const { return m_ParticleList.size(); }

		inline void
	getPos(size_t n, Vec3R&pos) const { pos = m_ParticleList[n].p; }


		inline void
	getPosRad(size_t n, Vec3R& pos, Real& rad) const {
		pos = m_ParticleList[n].p;
		rad = m_RadiusScale * m_ParticleList[n].r;
	}
		inline void
	getPosRadVel(size_t n, Vec3R& pos, Real& rad, Vec3R& vel) const {
		pos = m_ParticleList[n].p;
		rad = m_RadiusScale * m_ParticleList[n].r;
		vel = m_VelocityScale * m_ParticleList[n].v;
	}

		inline void
	getAtt(size_t n, Index32& att) const { att = n; }
};

/*
*	TriangleSoup is used for processing particle source
*	A particle list is generated with the soup, then used for 
*	creating VDB voxel from particles.
*/

class CTriangleSoup :
	public CLxImpl_TriangleSoup,
	public CLxSingletonPolymorph
{
    public:
	LXxSINGLETON_METHOD;

	CParticleList	*particleList;
	LXtMatrix4	 xfrm;
	unsigned	 off_vel, off_size;
	unsigned	 basic_number;
	float		 radius;

	CTriangleSoup (
		CParticleList	*plist,
		LXtMatrix4	 mat,
		float		 r)
	{
		particleList = plist;
		lx::Matrix4Copy (xfrm, mat);
		off_vel = 0;
		off_size = 0;
		radius = r;
		basic_number = plist->size();
		AddInterface (new CLxIfc_TriangleSoup<CTriangleSoup>);
	}

		LxResult
	soup_Segment (
		unsigned	 segID,
		unsigned	 type)			LXx_OVERRIDE
	{
		return (type == LXiTBLX_SEG_POINT) ? LXe_TRUE : LXe_FALSE;
	}

		LxResult
	soup_Vertex (
		const float	*vertex,
		unsigned	*index)			LXx_OVERRIDE
	{
		unsigned	 n = particleList->size ();
		CSample		sam;

		lx::Matrix4Multiply (sam.pos, xfrm, vertex + 0);

		if (off_vel)
			LXx_VCPY(sam.vel, vertex + off_vel);
		else
			LXx_VCLR(sam.vel);

		if (off_size)
			sam.size = vertex[off_size];
		else
			sam.size = 1.0;

		sam.size *= radius;

		particleList->add (sam);

		index[0] = n - basic_number;
		return LXe_OK;
	}

		LxResult
	soup_Polygon (
		unsigned	 v0,
		unsigned	 v1,
		unsigned	 v2)			LXx_OVERRIDE
	{
		return LXe_OK;
	}
};
#endif