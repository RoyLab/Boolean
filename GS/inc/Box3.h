#ifndef _BOX3_H_
#define _BOX3_H_
#include <math.h>
#include "typedefs.h"
#include "arithmetic.h"

namespace GS{
class Box3 {
private:
	float3  mMin, mMax;
 public:
        enum BOX_TYPE{BOX_IDENTITY, BOX_MIRROR, BOX_MIRROR_SCALE, BOX_LARGE};

        inline Box3(BOX_TYPE flag = BOX_LARGE)
        {
            switch (flag)
            {
            case GS::Box3::BOX_IDENTITY:
                new (this)Box3(0,0,0,1,1,1);
                break;
            case GS::Box3::BOX_MIRROR:
                new (this)Box3(-1,-1,-1,1,1,1);
                break;
            case GS::Box3::BOX_MIRROR_SCALE:
                new (this)Box3(-1.125,-1.125,-1.125,1.125,1.125,1.125);
                break;
            case GS::Box3::BOX_LARGE:
                Clear();
                break;
            default:
                break;
            }
        }

        inline Box3(float3 min, float3 max) : mMin(min), mMax(max)
        {
        }

        inline Box3(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax) 
            : mMin(xMin, yMin, zMin), mMax(xMax, yMax, zMax)
        {
        }

        inline Box3& operator=(const Box3& rhs)
        {
            // Avoids self-assignment.
            if(this == &rhs) return *this;

            mMin = rhs.mMin;
            mMax = rhs.mMax;
            return *this;
        }

        inline void Set(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax)
        {
            mMin.x = xMin;
            mMin.y = yMin;
            mMin.z = zMin;
            mMax.x = xMax;
            mMax.y = yMax;
            mMax.z = zMax;
        }


        inline void Set(float3 min, float3 max)
        {
            mMin = min;
            mMax = max;
        }


        inline void Set(const Box3& from)
        {
            mMin = from.mMin;
            mMax = from.mMax;
        }

        inline const float3& Min() const
        {
            return mMin;
        }

        inline void Min(float3 value)
        {
            mMin = value;
        }

        inline const float3& Max() const
        {
            return mMax;
        }

        inline void Max(float3 value)
        {
            mMax = value;
        }

        /// Gets and sets the box size.

        inline float3 Size() const
        {
            return mMax - mMin;
        }

        /// <description>
        /// Gets and sets the box center.
        /// </description>
        inline float3 Center() const
        {
            return (mMax + mMin) * 0.5f;
        }

        /// <description>
        /// Gets and sets the box radius.
        /// </description>
        inline float Radius() const
        {
            float3 d = mMax - mMin;
            float len = 0.5f * length(d); 
            return len;
			
        }

        inline bool IsEmpty() const
        {
            return mMin.x > 1e30f && mMax.x < -1e30f;
        }

        inline void Clear()
        {
            float3 huge( 2e30f, 2e30f, 2e30f );
            mMin = huge;
            mMax = -huge;
        }

        /// Checks if the point is contained in the box.
        inline bool IsInBox(float x, float y, float z) const
        {
            if ((x >= mMin.x) && (x <= mMax.x) && (y >= mMin.y) && (y <= mMax.y)
                &&(z >= mMin.z) && (z <= mMax.z))
                return true;
            else
                return false;
        }

        inline bool IsInBox(const double3& s, const double3& t) const 
        {
            
            if (s.x < mMin.x  && t.x < mMin.x )
                return false ; 
            if (s.y < mMin.y && t.y < mMin.y)
                return false ;
            if (s.z < mMin.z && t.z < mMin.z)
                return false ; 
            if (s.x > mMax.x && t.x > mMax.x)
                return false ; 
            if (s.y > mMax.y && t.y > mMax.y)
                return false ;
            if (s.z > mMax.z && t.z > mMax.z)
                return false ;
            return true;
        }

        /// Checks if the point contained is in the box.
        inline bool IsInBox(float3 p) const
        {
           return IsInBox(p.x, p.y, p.z);
        }

        /// Checks if this box is within the containing box.
        inline bool IsWithinBox(const Box3& containingBox) const
        {
            if (containingBox.IsInBox(mMin) && containingBox.IsInBox(mMax))
                return true;
            else
                return false;
        }

        /// positive in, negative out
        inline bool IntersectBias(const Box3& b) const
        {
            if ((mMax.x < b.mMin.x) || (mMax.y < b.mMin.y) || 
                (mMax.z < b.mMin.z) || (mMin.x >= b.mMax.x) ||
                (mMin.y >= b.mMax.y) || (mMin.z >= b.mMax.z))
                return false;
            else return true;
        }

        /// Checks if the 2 boxes touches or crosses each other.
        inline bool Intersects(const Box3& b) const
        {
            if ((mMax.x <= b.mMin.x) || (mMax.y <= b.mMin.y) || (mMax.z <= b.mMin.z)
                || (mMin.x >= b.mMax.x) || (mMin.y >= b.mMax.y) || (mMin.z >= b.mMax.z))
                return false;
            else
                return true;
        }
            

        /// Clip the box to the box b.
        inline void Clip(Box3& b)
        {
            mMin.x = max(mMin.x, b.mMin.x);
            mMin.y = max(mMin.y, b.mMin.y);
            mMin.z = max(mMin.z, b.mMin.z);
            mMax.x = min(mMax.x, b.mMax.x);
            mMax.y = min(mMax.y, b.mMax.y);
            mMax.z = min(mMax.z, b.mMax.z);
        }

        /// Adds new box to this box.
        inline void IncludeBox(const Box3& b)
        {
            // enlarge box by merging with new box
            if (b.mMin.x < mMin.x)
                mMin.x = b.mMin.x;
            if (b.mMin.y < mMin.y)
                mMin.y = b.mMin.y;
            if (b.mMin.z < mMin.z)
                mMin.z = b.mMin.z;

            if (b.mMax.x > mMax.x)
                mMax.x = b.mMax.x;
            if (b.mMax.y > mMax.y)
                mMax.y = b.mMax.y;
            if (b.mMax.z > mMax.z)
                mMax.z = b.mMax.z;
        }


        /// Adds a point to this box.
        inline void IncludePoint(float x, float y, float z)
        {
            IncludePoint(float3(x, y, z));
        }

        /// Adds a point to this box.
        inline void IncludePoint(const float3& pt)
        {
            // enlarge box by merging point with new box
            if (pt.x < mMin.x)
                mMin.x = pt.x;
            if (pt.y < mMin.y)
                mMin.y = pt.y;
            if (pt.z < mMin.z)
                mMin.z = pt.z;

            if (pt.x > mMax.x)
                mMax.x = pt.x;
            if (pt.y > mMax.y)
                mMax.y = pt.y;
            if (pt.z > mMax.z)
                mMax.z = pt.z;
        }

        /// Enlarges by an x sized band on x min and max 
        /// and a y sized band on each of y min and max
        /// and a z sized band on each of z min and max
        inline void Enlarge(float x, float y, float z)
        {
            mMin.x -= x;
            mMax.x += x;

            mMin.y -= y;
            mMax.y += y;

            mMin.z -= z;
            mMax.z += z;
        }

        /// Enlarges the box in the direction and size of v = (x,y,z).
        inline void EnlargeByVector(float x, float y, float z)
        {
            Enlarge(float3(x, y, z));
        }


        /// Enlarges the box in the direction and size of v.
        inline void Enlarge(const float3& v)
        {
            if(v.x < 0)
                mMin.x += v.x;
            else
                mMax.x += v.x;

            if (v.y < 0)
                mMin.y += v.y;
            else
                mMax.y += v.y;

            if (v.z < 0)
                mMin.z += v.z;
            else
                mMax.z += v.z;
        }

        /// Enlarges the box by epsilon all around.
        inline void Enlarge(float e)
        {
            Enlarge( e, e, e );
        }

        /// Find intersection of the two boxes, i.e. where both boxes exist.
        /// This method takes the original box and "applies" the input box to it,
        /// finding the overlap of the two. If the resulting box is "empty",
        /// no overlap, "false" is returned (the box is also given "empty"
        /// coordinates by using Clear()). True is returned if the resulting
        /// rectangle is not empty, i.e. there is overlap.
        inline bool IntersectBox(Box3& b)
        {
            if ( Intersects( b ) )
            {
                // minimize box by intersecting with input box
                if (b.mMin.x > mMin.x)
                    mMin.x = b.mMin.x;
                if (b.mMin.y > mMin.y)
                    mMin.y = b.mMin.y;
                if (b.mMin.z > mMin.z)
                    mMin.z = b.mMin.z;

                if (b.mMax.x < mMax.x)
                    mMax.x = b.mMax.x;
                if (b.mMax.y < mMax.y)
                    mMax.y = b.mMax.y;
                if (b.mMax.z < mMax.z)
                    mMax.z = b.mMax.z;
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
        inline float3 Diagonal() const
        {
            return mMax - mMin;
        }

        /// The indexer gets the points of the 8 corners of the box.

        inline float3 operator [] (int index ) const
        {
            switch(index)
            {
            case 0: return float3(mMin.x, mMin.y, mMin.z);
            case 1: return float3(mMax.x, mMin.y, mMin.z);
            case 2: return float3(mMin.x, mMax.y, mMin.z);
            case 3: return float3(mMax.x, mMax.y, mMin.z);
            case 4: return float3(mMin.x, mMin.y, mMax.z);
            case 5: return float3(mMax.x, mMin.y, mMax.z);
            case 6: return float3(mMin.x, mMax.y, mMax.z);
            case 7: return float3(mMax.x, mMax.y, mMax.z);
            default : return mMin;
            }
        }


        inline bool operator == ( const Box3& box )const
        {
            return (mMin == box.mMin) && (mMax == box.mMax);
        }


        /// Returns the axis index of the box size in length order: 
        /// index 0 is shortest, index 1 is middle, index 2 is longest.
        inline void AxisByLength( int3& outAxes ) const
        {
            if (IsEmpty())
            {
                // Arbitrary value.
                outAxes.x = 0;
                outAxes.y = 1;
                outAxes.z = 2;
                return;
            }
            float3 sz = (mMax - mMin);
            outAxes.x = (sz.x <= sz.y)?
                ((sz.x <= sz.z) ? 0 : 2) :
                ((sz.y <= sz.z) ? 1 : 2);
            outAxes.z =  (sz.x >= sz.y)?
                ((sz.x >= sz.z) ? 0 : 2) :
                ((sz.y >= sz.z) ? 1 : 2);

            bool used[3] = { false, false, false };
            used[ outAxes.x ] = true;
            used[ outAxes.z ] = true;
            outAxes.y = used[0] ? (used[1]? 2 : 1) : 0;
        }

        /// Returns the given component of the size.
        inline float LengthByAxis(int axis) const
        {
            if (IsEmpty()) return 0.0f;
            switch (axis)
            {
            case 0: 
                return mMax.x - mMin.x;
            case 1:
                return mMax.y - mMin.y;
            case 2:
                return mMax.z - mMin.z;
            }
            return 0.0f;
        }

        /// Compares the given box to us. Returns -1 if box all inside, 0 if all outside, 1 if intersents.
        /// The other return value means error.
        inline int CompareBox(Box3& b) const
        {
            if (IsEmpty() || b.IsEmpty()) return -2; // Error.
            float3 mn = b.Min();
            float3 mx = b.Max();

            // compare the incoming box to us
            // outside & touching is all outside, inside & touching is all inside
            if((mx.x <= mMin.x) || (mx.y <= mMin.y) || (mx.z <= mMin.z) 
                || (mn.x >= mMax.x) || (mn.y >= mMax.y) || (mn.z >= mMax.z))
                return 0; // all outside
            else if( (mn.x >= mMin.x) && (mn.y >= mMin.y) && (mn.z >= mMin.z) 
                && (mx.x <= mMax.x) && (mx.y <= mMax.y) && (mx.z <= mMax.z))
                return -1; // all inside
            else
                return 1; // intersects
        }
		
		// ansume mat is a row-vector matrix 
		inline void TransformBy(const float4x4& mat)
		{
			float3 ntMin(mat[3].xyz);
			float3 ntMax(mat[3].xyz);

			for (int i = 0; i < 3; i++) 
			{
				for (int j = 0; j < 3; j++) 
				{
					const double matji = mat[j][i];
					const double a = matji * mMin[j];
					const double b = matji * mMax[j];
					if (a < b) 
					{
						ntMin[i] += a;
						ntMax[i] += b;
					}
					else 
					{
						ntMin[i] += b;
						ntMax[i] += a;
					}
				}
			}

			mMin = ntMin;
			mMax = ntMax;  
	}
};
}




#endif 