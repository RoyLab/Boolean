#pragma once
#include <OpenMesh\Core\Geometry\VectorT.hh>
#include "arithmetic.h"

namespace CSG
{
    struct AABBmp {

		enum BOX_TYPE
		{
			BOX_IDENTITY, 
			BOX_MIRROR,
			BOX_MIRROR_SCALE,
			BOX_LARGE
		};

        OpenMesh::Vec3d  mMin, mMax;

        AABBmp(BOX_TYPE flag = BOX_LARGE)
        {
            const double larger = 1.125;
            switch (flag)
            {
            case BOX_IDENTITY:
                new (this)AABBmp(0,0,0,1,1,1);
                break;
            case BOX_MIRROR:
                new (this)AABBmp(-1,-1,-1,1,1,1);
                break;
            case BOX_MIRROR_SCALE:
                new (this)AABBmp(-larger,-larger,-larger,
                    larger,larger,larger);
                break;
            case BOX_LARGE:
                Clear();
                break;
            default:
                break;
            }
        }

        AABBmp(OpenMesh::Vec3d min, OpenMesh::Vec3d max) : mMin(min), mMax(max)
        {
        }

        AABBmp(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax) 
            : mMin(xMin, yMin, zMin), mMax(xMax, yMax, zMax)
        {
        }

        AABBmp& operator=(const AABBmp& rhs)
        {
            // Avoids self-assignment.
            if(this == &rhs) return *this;

            mMin = rhs.mMin;
            mMax = rhs.mMax;
            return *this;
        }

        void Set(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax)
        {
            mMin[0] = xMin;
            mMin[1] = yMin;
            mMin[2] = zMin;
            mMax[0] = xMax;
            mMax[1] = yMax;
            mMax[2] = zMax;
        }


        void Set(OpenMesh::Vec3d min, OpenMesh::Vec3d max)
        {
            mMin = min;
            mMax = max;
        }


        void Set(const AABBmp& from)
        {
            mMin = from.mMin;
            mMax = from.mMax;
        }

        const OpenMesh::Vec3d& Min() const
        {
            return mMin;
        }

        void Min(OpenMesh::Vec3d value)
        {
            mMin = value;
        }

        const OpenMesh::Vec3d& Max() const
        {
            return mMax;
        }

        void Max(OpenMesh::Vec3d value)
        {
            mMax = value;
        }

        /// Gets and sets the box size.

        OpenMesh::Vec3d Size() const
        {
            return mMax - mMin;
        }

        /// <description>
        /// Gets and sets the box center.
        /// </description>
        OpenMesh::Vec3d Center() const
        {
            return (mMax + mMin) * 0.5f;
        }

        /// <description>
        /// Gets and sets the box radius.
        /// </description>
        double Radius() const
        {
            OpenMesh::Vec3d d = mMax - mMin;
            double len = 0.5f * d.length(); 
            return len;
            
        }

        bool IsEmpty() const
        {
            return mMin[0] > 1e30f && mMax[0] < -1e30f;
        }

        void Clear()
        {
            OpenMesh::Vec3d huge( 2e30f, 2e30f, 2e30f );
            mMin = huge;
            mMax = -huge;
        }

        /// Checks if the point is contained in the box.
        bool IsInBox(double x, double y, double z) const
        {
            if ((x >= mMin[0]) && (x <= mMax[0]) && (y >= mMin[1]) && (y <= mMax[1])
                &&(z >= mMin[2]) && (z <= mMax[2]))
                return true;
            else
                return false;
        }

        bool IsInBox(const OpenMesh::Vec3d& s, const OpenMesh::Vec3d& t) const 
        {
            
            if (s[0] < mMin[0]  && t[0] < mMin[0] )
                return false ; 
            if (s[1] < mMin[1] && t[1] < mMin[1])
                return false ;
            if (s[2] < mMin[2] && t[2] < mMin[2])
                return false ; 
            if (s[0] > mMax[0] && t[0] > mMax[0])
                return false ; 
            if (s[1] > mMax[1] && t[1] > mMax[1])
                return false ;
            if (s[2] > mMax[2] && t[2] > mMax[2])
                return false ;
            return true;
        }

        /// Checks if the point contained is in the box.
        bool IsInBox(OpenMesh::Vec3d p) const
        {
           return IsInBox(p[0], p[1], p[2]);
        }

        /// Checks if this box is within the containing box.
        bool IsWithinBox(const AABBmp& containingBox) const
        {
            if (containingBox.IsInBox(mMin) && containingBox.IsInBox(mMax))
                return true;
            else
                return false;
        }

        /// Checks if the 2 boxes touches or crosses each other.
        bool Intersects(const AABBmp& b) const
        {
            if ((mMax[0] <= b.mMin[0]) || (mMax[1] <= b.mMin[1]) || (mMax[2] <= b.mMin[2])
                || (mMin[0] >= b.mMax[0]) || (mMin[1] >= b.mMax[1]) || (mMin[2] >= b.mMax[2]))
                return false;
            else
                return true;
        }
            

        /// Clip the box to the box b.
        void Clip(AABBmp& b)
        {
            mMin[0] = GS::max(mMin[0], b.mMin[0]);
            mMin[1] = GS::max(mMin[1], b.mMin[1]);
            mMin[2] = GS::max(mMin[2], b.mMin[2]);
            mMax[0] = GS::min(mMax[0], b.mMax[0]);
            mMax[1] = GS::min(mMax[1], b.mMax[1]);
            mMax[2] = GS::min(mMax[2], b.mMax[2]);
        }

        /// Adds new box to this box.
        void IncludeBox(const AABBmp& b)
        {
            // enlarge box by merging with new box
            if (b.mMin[0] < mMin[0])
                mMin[0] = b.mMin[0];
            if (b.mMin[1] < mMin[1])
                mMin[1] = b.mMin[1];
            if (b.mMin[2] < mMin[2])
                mMin[2] = b.mMin[2];

            if (b.mMax[0] > mMax[0])
                mMax[0] = b.mMax[0];
            if (b.mMax[1] > mMax[1])
                mMax[1] = b.mMax[1];
            if (b.mMax[2] > mMax[2])
                mMax[2] = b.mMax[2];
        }


        /// Adds a point to this box.
        void IncludePoint(double x, double y, double z)
        {
            IncludePoint(OpenMesh::Vec3d(x, y, z));
        }

        /// Adds a point to this box.
        void IncludePoint(const OpenMesh::Vec3d& pt)
        {
            // enlarge box by merging point with new box
            if (pt[0] < mMin[0])
                mMin[0] = pt[0];
            if (pt[1] < mMin[1])
                mMin[1] = pt[1];
            if (pt[2] < mMin[2])
                mMin[2] = pt[2];

            if (pt[0] > mMax[0])
                mMax[0] = pt[0];
            if (pt[1] > mMax[1])
                mMax[1] = pt[1];
            if (pt[2] > mMax[2])
                mMax[2] = pt[2];
        }

        /// Enlarges by an x sized band on x min and max 
        /// and a y sized band on each of y min and max
        /// and a z sized band on each of z min and max
        void Enlarge(double x, double y, double z)
        {
            mMin[0] -= x;
            mMax[0] += x;

            mMin[1] -= y;
            mMax[1] += y;

            mMin[2] -= z;
            mMax[2] += z;
        }

        /// Enlarges the box in the direction and size of v = (x,y,z).
        void EnlargeByVector(double x, double y, double z)
        {
            Enlarge(OpenMesh::Vec3d(x, y, z));
        }


        /// Enlarges the box in the direction and size of v.
        void Enlarge(const OpenMesh::Vec3d& v)
        {
            if(v[0] < 0)
                mMin[0] += v[0];
            else
                mMax[0] += v[0];

            if (v[1] < 0)
                mMin[1] += v[1];
            else
                mMax[1] += v[1];

            if (v[2] < 0)
                mMin[2] += v[2];
            else
                mMax[2] += v[2];
        }

        /// Enlarges the box by epsilon all around.
        void Enlarge(double e)
        {
            Enlarge( e, e, e );
        }

        /// Find intersection of the two boxes, i.e. where both boxes exist.
        /// This method takes the original box and "applies" the input box to it,
        /// finding the overlap of the two. If the resulting box is "empty",
        /// no overlap, "false" is returned (the box is also given "empty"
        /// coordinates by using Clear()). True is returned if the resulting
        /// rectangle is not empty, i.e. there is overlap.
        bool IntersectBox(AABBmp& b)
        {
            if ( Intersects( b ) )
            {
                // minimize box by intersecting with input box
                if (b.mMin[0] > mMin[0])
                    mMin[0] = b.mMin[0];
                if (b.mMin[1] > mMin[1])
                    mMin[1] = b.mMin[1];
                if (b.mMin[2] > mMin[2])
                    mMin[2] = b.mMin[2];

                if (b.mMax[0] < mMax[0])
                    mMax[0] = b.mMax[0];
                if (b.mMax[1] < mMax[1])
                    mMax[1] = b.mMax[1];
                if (b.mMax[2] < mMax[2])
                    mMax[2] = b.mMax[2];
                return true;
            }
            else
            {
                // the boxes don't overlap, so the new box is empty.
                Clear();
                return false;
            }
        }

        /// Returns the diagonal vector of the box..
        OpenMesh::Vec3d Diagonal() const
        {
            return mMax - mMin;
        }

        /// The indexer gets the points of the 8 corners of the box.

        OpenMesh::Vec3d operator [] (int index ) const
        {
            switch(index)
            {
            case 0: return OpenMesh::Vec3d(mMin[0], mMin[1], mMin[2]);
            case 1: return OpenMesh::Vec3d(mMax[0], mMin[1], mMin[2]);
            case 2: return OpenMesh::Vec3d(mMin[0], mMax[1], mMin[2]);
            case 3: return OpenMesh::Vec3d(mMax[0], mMax[1], mMin[2]);
            case 4: return OpenMesh::Vec3d(mMin[0], mMin[1], mMax[2]);
            case 5: return OpenMesh::Vec3d(mMax[0], mMin[1], mMax[2]);
            case 6: return OpenMesh::Vec3d(mMin[0], mMax[1], mMax[2]);
            case 7: return OpenMesh::Vec3d(mMax[0], mMax[1], mMax[2]);
            default : return mMin;
            }
        }


        bool operator == ( const AABBmp& box )const
        {
            return (mMin == box.mMin) && (mMax == box.mMax);
        }


        /// Returns the axis index of the box size in length order: 
        /// index 0 is shortest, index 1 is middle, index 2 is longest.
		void AxisByLength( OpenMesh::Vec3i& outAxes ) const
        {
            if (IsEmpty())
            {
                // Arbitrary value.
                outAxes[0] = 0;
                outAxes[1] = 1;
                outAxes[2] = 2;
                return;
            }
            OpenMesh::Vec3d sz = (mMax - mMin);
            outAxes[0] = (sz[0] <= sz[1])?
                ((sz[0] <= sz[2]) ? 0 : 2) :
                ((sz[1] <= sz[2]) ? 1 : 2);
            outAxes[2] =  (sz[0] >= sz[1])?
                ((sz[0] >= sz[2]) ? 0 : 2) :
                ((sz[1] >= sz[2]) ? 1 : 2);

            bool used[3] = { false, false, false };
            used[ outAxes[0] ] = true;
            used[ outAxes[2] ] = true;
            outAxes[1] = used[0] ? (used[1]? 2 : 1) : 0;
        }

        /// Returns the given component of the size.
        double LengthByAxis(int axis) const
        {
            if (IsEmpty()) return 0.0f;
            switch (axis)
            {
            case 0: 
                return mMax[0] - mMin[0];
            case 1:
                return mMax[1] - mMin[1];
            case 2:
                return mMax[2] - mMin[2];
            }
            return 0.0f;
        }

        /// Compares the given box to us. Returns -1 if box all inside, 0 if all outside, 1 if intersents.
        /// The other return value means error.
        int CompareBox(AABBmp& b) const
        {
            if (IsEmpty() || b.IsEmpty()) return -2; // Error.
            OpenMesh::Vec3d mn = b.Min();
            OpenMesh::Vec3d mx = b.Max();

            // compare the incoming box to us
            // outside & touching is all outside, inside & touching is all inside
            if((mx[0] <= mMin[0]) || (mx[1] <= mMin[1]) || (mx[2] <= mMin[2]) 
                || (mn[0] >= mMax[0]) || (mn[1] >= mMax[1]) || (mn[2] >= mMax[2]))
                return 0; // all outside
            else if( (mn[0] >= mMin[0]) && (mn[1] >= mMin[1]) && (mn[2] >= mMin[2]) 
                && (mx[0] <= mMax[0]) && (mx[1] <= mMax[1]) && (mx[2] <= mMax[2]))
                return -1; // all inside
            else
                return 1; // intersects
        }

        /// Checks if the point is contained in the box.
        /// left open, right close (a, b].
        bool IsInBox_LORC(OpenMesh::Vec3d p) const
        {
            return IsInBox_LORC(p[0], p[1], p[2]);
        }

        /// Checks if the point is contained in the box.
        /// left open, right close (a, b].
        bool IsInBox_LORC(double x, double y, double z) const
        {
            if ((x >= mMin[0]) && (x < mMax[0]) && (y >= mMin[1]) && (y < mMax[1])
                &&(z >= mMin[2]) && (z < mMax[2]))
                return true;
            else return false;
        }

        
        /// left open, right close (a, b].
        bool Intersect_LORC(const AABBmp& b) const
        {
            if ((mMax[0] < b.mMin[0]) || (mMax[1] < b.mMin[1]) || 
                (mMax[2] < b.mMin[2]) || (mMin[0] >= b.mMax[0]) ||
                (mMin[1] >= b.mMax[1]) || (mMin[2] >= b.mMax[2]))
                return false;
            else return true;
        }

    };

} // namespace GS
