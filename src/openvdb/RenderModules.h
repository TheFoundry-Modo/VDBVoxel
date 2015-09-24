///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012-2014 DreamWorks Animation LLC
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// *     Neither the name of DreamWorks Animation nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////

#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/PointScatter.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/math/Operators.h>
#include <math.h>

namespace openvdb_viewer {

namespace util {

    template<typename TreeType>
    class PointGenerator
    {
    public:
        typedef openvdb::tree::LeafManager<TreeType> LeafManagerType;

        PointGenerator(
            std::vector<float>& points,
            std::vector<unsigned>& indices,
            LeafManagerType& leafs,
            std::vector<size_t>& indexMap,
            const openvdb::math::Transform& transform,
            openvdb::Index64 voxelsPerLeaf = TreeType::LeafNodeType::NUM_VOXELS)
            : mPoints(points)
            , mIndices(indices)
            , mLeafs(leafs)
            , mIndexMap(indexMap)
            , mTransform(transform)
            , mVoxelsPerLeaf(voxelsPerLeaf)
        {
        }

        void runParallel()
        {
            tbb::parallel_for(mLeafs.getRange(), *this);
        }


        inline void operator()(const typename LeafManagerType::RangeType& range) const
        {
            using openvdb::Index64;

            typedef typename TreeType::LeafNodeType::ValueOnCIter ValueOnCIter;

            openvdb::Vec3d pos;
            size_t index = 0;
            Index64 activeVoxels = 0;

            for (size_t n = range.begin(); n < range.end(); ++n) {

                index = mIndexMap[n];
                ValueOnCIter it = mLeafs.leaf(n).cbeginValueOn();

                activeVoxels = mLeafs.leaf(n).onVoxelCount();

                if (activeVoxels <= mVoxelsPerLeaf) {

                    for ( ; it; ++it) {
                        pos = mTransform.indexToWorld(it.getCoord());
                        insertPoint(pos, index);
                        ++index;
                    }

                } else if (1 == mVoxelsPerLeaf) {

                     pos = mTransform.indexToWorld(it.getCoord());
                     insertPoint(pos, index);

                } else {

                    std::vector<openvdb::Coord> coords;
                    coords.reserve(static_cast<size_t>(activeVoxels));
                    for ( ; it; ++it) { coords.push_back(it.getCoord()); }

                    pos = mTransform.indexToWorld(coords[0]);
                    insertPoint(pos, index);
                    ++index;

                    pos = mTransform.indexToWorld(coords[static_cast<size_t>(activeVoxels-1)]);
                    insertPoint(pos, index);
                    ++index;

                    Index64 r = Index64(std::floor(double(mVoxelsPerLeaf) / activeVoxels));
                    for (Index64 i = 1, I = mVoxelsPerLeaf - 2; i < I; ++i) {
                        pos = mTransform.indexToWorld(coords[static_cast<size_t>(i * r)]);
                        insertPoint(pos, index);
                        ++index;
                    }
                }
            }
        }

    private:
        void insertPoint(const openvdb::Vec3d& pos, size_t index) const
        {
            mIndices[index] = unsigned(index);
            const size_t element = index * 3;
            mPoints[element    ] = static_cast<float>(pos[0]);
            mPoints[element + 1] = static_cast<float>(pos[1]);
            mPoints[element + 2] = static_cast<float>(pos[2]);
        }

        std::vector<float>& mPoints;
        std::vector<unsigned>& mIndices;
        LeafManagerType& mLeafs;
        std::vector<size_t>& mIndexMap;
        const openvdb::math::Transform& mTransform;
        const openvdb::Index64 mVoxelsPerLeaf;
    }; // PointGenerator


    template<typename GridType>
    class PointAttributeGenerator
    {
    public:
        typedef typename GridType::ValueType ValueType;

        PointAttributeGenerator(
            std::vector<float>& points,
            std::vector<float>& colors,
            const GridType& grid,
            ValueType minValue,
            ValueType maxValue,
            openvdb::Vec3s (&colorMap)[4],
            bool isLevelSet = false)
            : mPoints(points)
            , mColors(colors)
            , mNormals(NULL)
            , mGrid(grid)
            , mAccessor(grid.tree())
            , mMinValue(minValue)
            , mMaxValue(maxValue)
            , mColorMap(colorMap)
            , mIsLevelSet(isLevelSet)
            , mZeroValue(openvdb::zeroVal<ValueType>())
        {
            init();
        }

        PointAttributeGenerator(
            std::vector<float>& points,
            std::vector<float>& colors,
            std::vector<float>& normals,
            const GridType& grid,
            ValueType minValue,
            ValueType maxValue,
            openvdb::Vec3s (&colorMap)[4],
            bool isLevelSet = false)
            : mPoints(points)
            , mColors(colors)
            , mNormals(&normals)
            , mGrid(grid)
            , mAccessor(grid.tree())
            , mMinValue(minValue)
            , mMaxValue(maxValue)
            , mColorMap(colorMap)
            , mIsLevelSet(isLevelSet)
            , mZeroValue(openvdb::zeroVal<ValueType>())
        {
            init();
        }

        void runParallel()
        {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, (mPoints.size() / 3)), *this);
        }

        inline void operator()(const tbb::blocked_range<size_t>& range) const
        {
            openvdb::Coord ijk;
            openvdb::Vec3d pos, tmpNormal, normal(0.0, -1.0, 0.0);
            openvdb::Vec3s color(0.9f, 0.3f, 0.3f);
            float w = 0.0;

            size_t e1, e2, e3, voxelNum = 0;
            for (size_t n = range.begin(); n < range.end(); ++n) {
                e1 = 3 * n;
                e2 = e1 + 1;
                e3 = e2 + 1;

                pos[0] = mPoints[e1];
                pos[1] = mPoints[e2];
                pos[2] = mPoints[e3];

                pos = mGrid.worldToIndex(pos);
                ijk[0] = int(pos[0]);
                ijk[1] = int(pos[1]);
                ijk[2] = int(pos[2]);

                const ValueType& value = mAccessor.getValue(ijk);

                if (value < mZeroValue) { // is negative
                    if (mIsLevelSet) {
                        color = mColorMap[1];
                    } else {
                        w = (float(value) - mOffset[1]) * mScale[1];
                        color = openvdb::Vec3s(w * mColorMap[0] + (1.0 - w) * mColorMap[1]);
                    }
                } else {
                    if (mIsLevelSet) {
                        color = mColorMap[2];
                    } else {
                        w = (float(value) - mOffset[0]) * mScale[0];
                        color = openvdb::Vec3s(w * mColorMap[2] + (1.0 - w) * mColorMap[3]);
                    }
                }

                mColors[e1] = color[0];
                mColors[e2] = color[1];
                mColors[e3] = color[2];

                if (mNormals) {

                    if ((voxelNum % 2) == 0) {
                        tmpNormal = openvdb::Vec3d(openvdb::math::ISGradient<
                            openvdb::math::CD_2ND>::result(mAccessor, ijk));

                        double length = tmpNormal.length();
                        if (length > 1.0e-7) {
                            tmpNormal *= 1.0 / length;
                            normal = tmpNormal;
                        }
                    }
                    ++voxelNum;

                    (*mNormals)[e1] = static_cast<float>(normal[0]);
                    (*mNormals)[e2] = static_cast<float>(normal[1]);
                    (*mNormals)[e3] = static_cast<float>(normal[2]);
                }
            }
        }

    private:

        void init()
        {
            mOffset[0] = static_cast<float>(std::min(mZeroValue, mMinValue));
            mScale[0] = static_cast<float>(
                1.0 / (std::abs(std::max(mZeroValue, mMaxValue) - mOffset[0])));
            mOffset[1] = static_cast<float>(std::min(mZeroValue, mMinValue));
            mScale[1] = static_cast<float>(
                1.0 / (std::abs(std::max(mZeroValue, mMaxValue) - mOffset[1])));
        }

        std::vector<float>& mPoints;
        std::vector<float>& mColors;
        std::vector<float>* mNormals;

        const GridType& mGrid;
        openvdb::tree::ValueAccessor<const typename GridType::TreeType> mAccessor;

        ValueType mMinValue, mMaxValue;
        openvdb::Vec3s (&mColorMap)[4];
        const bool mIsLevelSet;

        ValueType mZeroValue;
        float mOffset[2], mScale[2];
    }; // PointAttributeGenerator
}
    }