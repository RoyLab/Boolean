#ifndef _BOX2_H_
#define _BOX2_H_
#include "typedefs.h"
namespace GS{

class Box2{

public:
        inline Box2()
		{
            Clear();
        }

        inline Box2(double2 min, double2 max) : mMin(min), mMax(max)
        {
        }

        inline Box2(double xMin, double yMin, double xMax, double yMax) : mMin(xMin,yMin), mMax(xMax,yMax)
        {
        }

        inline Box2& operator=(const Box2& rhs)
        {
            // Avoids self-assignment.
            if(this == &rhs) return *this;

            mMin = rhs.mMin;
            mMax = rhs.mMax;
            return *this;
        }


        inline void Set(double xMin, double yMin, double xMax, double yMax)
        {
            mMin.x = xMin;
            mMin.y = yMin;
            mMax.x = xMax;
            mMax.y = yMax;
        }

        inline double2 Min() const
        {
            return mMin;
        }

        /// <description>
        /// Sets the box minimum.
        /// </description>
        inline void Min(double2 value)
        {
            mMin = value;
        }

        /// <description>
        /// Gets the box maximum.
        /// </description>
        inline double2 Max() const
        {
            return mMax;
        }

        /// <description>
        /// Sets the box maximum.
        /// </description>
        inline void Max(double2 value)
        {
            mMax = value;
        }


        inline double2 Size() const
        {
            return mMax - mMin;
        }

        inline double2 Center() const
        {
            return (mMax + mMin) * 0.5f;
        }

        inline double Radius() const
        {
            double2 d = mMax - mMin;
            double len = 0.5f * length(d); 
            return len;
        }

        inline double Width() const
        {
            return mMax.x - mMin.x;
        }

        inline double Height() const
        {
            return mMax.y - mMin.y;
        }

        inline bool IsEmpty() const
        {
            return mMin.x > 1e30f && mMax.x < -1e30f;
        }

        inline void Clear()
        {
            double2 huge( 2e30f, 2e30f );
            mMin = huge;
            mMax = -huge;
        }

        inline bool IsInBox(double _x, double _y) const
        {
            if ((_x >= mMin.x) && (_x <= mMax.x) && (_y >= mMin.y) && (_y <= mMax.y))
                return true;
            else
                return false;
        }

        inline bool IsInBox(double2 p) const
        {
            if ((p.x >= mMin.x) && (p.x <= mMax.x) && (p.y >= mMin.y) && (p.y <= mMax.y))
                return true;
            else
                return false;
        }

		inline bool IsInBox(double2 p, double EPS) const
        {
            if ((p.x >= mMin.x- EPS) && (p.x <= mMax.x+EPS) && (p.y >= mMin.y -EPS) && (p.y <= mMax.y+EPS))
                return true;
            else
                return false;
        }

        inline bool IsWithinBox(Box2& containingBox) const
        {
            if (containingBox.IsInBox(mMin) && containingBox.IsInBox(mMax))
                return true;
            else
                return false;
        }

        inline bool Intersects(const Box2& b) const
        {
            if ((mMax.x < b.mMin.x) || (mMax.y < b.mMin.y) || (mMin.x > b.mMax.x) || (mMin.y > b.mMax.y))
                return false;
            else
                return true;
        }
            
		inline bool Intersects(const Box2& b , double EPS) const 
		{
			if ((mMax.x < b.mMin.x -EPS) || (mMax.y < b.mMin.y - EPS )
				||(mMin.x > b.mMax.x +EPS) || (mMin.y > b.mMax.y +EPS))
				return false ;
			return true;
		}

        inline void Clip(Box2& b)
        {
            mMin.x = max(mMin.x, b.mMin.x);
            mMin.y = max(mMin.y, b.mMin.y);
            mMax.x = min(mMax.x, b.mMax.x);
            mMax.y = min(mMax.y, b.mMax.y);
        }


        inline void AddBox(Box2& b)
        {
            // enlarge box by merging with new box
            if (b.mMin.x < mMin.x)
                mMin.x = b.mMin.x;
            if (b.mMin.y < mMin.y)
                mMin.y = b.mMin.y;

            if (b.mMax.x > mMax.x)
                mMax.x = b.mMax.x;
            if (b.mMax.y > mMax.y)
                mMax.y = b.mMax.y;
        }

        inline void AddPoint(double x, double y)
        {
            AddPoint(double2(x, y));
        }

        inline void AddPoint(const double2& pt)
        {
            // enlarge box by merging point with new box
            if (pt.x < mMin.x)
                mMin.x = pt.x;
            if (pt.y < mMin.y)
                mMin.y = pt.y;

            if (pt.x > mMax.x)
                mMax.x = pt.x;
            if (pt.y > mMax.y)
                mMax.y = pt.y;
        }

        inline void Enlarge(double x, double y)
        {
            mMin.x -= x;
            mMax.x += x;

            mMin.y -= y;
            mMax.y += y;
        }

        inline void EnlargeByVector(double x, double y)
        {
            Enlarge(double2(x, y));
        }

        inline void Enlarge(const double2& v)
        {
            if(v.x < 0)
                mMin.x += v.x;
            else
                mMax.x += v.x;

            if (v.y < 0)
                mMin.y += v.y;
            else
                mMax.y += v.y;
        }

        inline void Enlarge(double e)
        {
            Enlarge( e, e );
        }

        /// Find intersection of the two boxes, i.e. where both boxes exist.
        /// This method akes the original box and "applies" the input box to it,
        /// finding the overlap of the two. If the resulting box is "empty",
        /// no overlap, "false" is returned (the box is also given "empty"
        /// coordinates by using Clear()). True is returned if the resulting
        /// rectangle is not empty, i.e. there is overlap.
        inline bool IntersectBox(Box2& b)
        {
            if ( Intersects( b ) )
            {
                // minimize box by intersecting with input box
                if (b.mMin.x > mMin.x)
                    mMin.x = b.mMin.x;
                if (b.mMin.y > mMin.y)
                    mMin.y = b.mMin.y;

                if (b.mMax.x < mMax.x)
                    mMax.x = b.mMax.x;
                if (b.mMax.y < mMax.y)
                    mMax.y = b.mMax.y;
                return true;
            }
            else
            {
                Clear();
                return false;
            }
        }

        inline double2 Diagonal() const
        {
            return mMax - mMin;
        }

        inline double2 operator [] (int index ) const
        {
            switch(index)
            {
            case 0:    return double2(mMin.x, mMin.y);
            case 1:    return double2(mMax.x, mMin.y);
            case 2:    return double2(mMin.x, mMax.y);
            case 3:    return double2(mMax.x, mMax.y);
            default :  return mMin;
            }
        }


        static inline int FarthestBoxCornerIndex(const double2& v)
        {
            // Calculates the fastest corner index. The idea is that any given vector points towards one
            // of the 4 quadrants of 2D space. For each quadrant, there is one box corner that
            // is the farthest along that vector, i.e. that gives the largest value when
            // a dot-product with the vector is done. This method identifies that corner index.
            // The box itself is not necessary; all box corners at the index will be the farthest
            // along the vector.
            //
            // If plane points to -X, -Y, value is 0.
            // +X, -Y == 1
            // -X, +Y == 2
            // on up to +X, +Y == 3.
            // This matches the Box2's double2 this[ int index ] ordering.
            return (((v.x < 0) ? 0 : 1) + ((v.y < 0) ? 0 : 2));
        }

        inline bool operator == ( const Box2& box )const
        {
            return  (mMin == box.mMin) && (mMax == box.mMax);
        }
private:
	double2 mMin, mMax; 

};
}

#endif 