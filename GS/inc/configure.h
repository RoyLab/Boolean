#ifndef _CONFIGURE_H_
#define _CONFIGURE_H_
#include <assert.h>
#include <cmath>
#include <hash_map>
#include <vector>
#include "typedefs.h"
#include "hashlookup.h"


namespace GS{


typedef double2                         Point2D;
typedef double3                         Point3D;
typedef double2                         Vec2D;
typedef double3                         Vec3D;
typedef std::vector<Point3D>            ListOf3DPoints;
typedef std::vector<Point2D>            ListOf2DPoints;

template<typename P= float> struct TVertexInfo{
	vec3<P>  pos;
	vec3<P>  normal; 
	float4   color;
	TVertexInfo()
		:pos(0.0, 0.0, 0.0)
		,normal(0.0, 0.0, 0.0)
		,color (0.67, 0.67, 0.67, 1.0)
	{
	}
	TVertexInfo(const vec3<P>& p, const vec3<P>& n, const float4& c)
		:pos(p)
		,normal(n)
		,color(c)
	{
	}
};

typedef TVertexInfo<double>             VertexInfo;
typedef std::vector<VertexInfo>         ListOfvertices;



const int  INIT_VERTEX_CAPACITY = 8;
const int  INIT_SEG_CAPACITY = 8;
const int  INIT_PRIMITIVE_CAPACITY = 16;
const int  INIT_VERTEX_HASH_CAPACITY = 16;

const double PIXEL_ANGLE_SCALE = (PI / 180.0);
const double PITCH_MAX = (89.0 * PI / 180.0);
const double PITCH_MIN = (-PITCH_MAX);
const double TIME_ANGLE_SCALE = (PI / 4.0);
const double DEGREES_PER_PIXEL = (45.0 / 600.0);
const double DISTANCE_PER_PIXEL = 0.1;

//  //NOTE: 这个参数不能调太大了
//const float DEF_GEO3D_EPS = 4*1e-1;
//
//  //重心坐标（是相对坐标）eps
//const float   BARYCENTRIC_EPS = 1e-5;
//  //2维点共点的EPS
//const float   POINT2D_EQUAL_EPS = DEF_GEO3D_EPS;
//  //3维点共点的EPS
//const float   POINT3D_EQUAL_EPS = DEF_GEO3D_EPS;
//  //最短线段长度。注意保证最短线段长度小于2维点共点长度，便于线段重合的打断算法的处理
//const float   SEG_MIN_DIST_EPS = POINT2D_EQUAL_EPS/2;
//  //平面加厚EPS
//const float   PLANE_THICK_EPS = DEF_GEO3D_EPS;
//  //线段加厚EPS
//const float   LINE_THICK_EPS = DEF_GEO3D_EPS;
//  //3维AABB
//const float   AABB3D_EPS = DEF_GEO3D_EPS*4;
//  //2维AABB
//const float   SEG_AABB_EPS = DEF_GEO3D_EPS*4;
//  //线段平行判定
//const float   SEG_PARALLEL_EPS = 1e-2;
//  //平面平行判定
//const float   PLANE_PARALLEL_EPS = 1e-2;

template <typename P> 
static inline bool IsEqual(P a, P b,  P EPS)
{
	return fabs(a-b) <= EPS ;
}


/// Please refer to the article below for more detail but note that the article does not
/// say anything about the special case where intDiff is INT_MIN which we have added 
/// a special check for here.
///     http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
/// </remarks>
static inline bool float_compare(float a, float b, int max_ulps = 8)
{
    // Make sure max_ulps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    assert(max_ulps > 0 && max_ulps < (1 << 22));

    int aInt = *(int*)&a; // load the float bits into integers
    int bInt = *(int*)&b;

    // Comment out the NAN comparison check, and we did not also add the Infinite check.
    // Because these optional checks(Infinite, NAN, etc.) are not necessary in many cases, 
    // or even not desirable. And these checks will probably affact the running efficiency.
    //if ((aInt & 0x7fffffff) > 0x7f800000 || (bInt & 0x7fffffff) > 0x7f800000)
    //    return false; // Remember: NaN != NaN

    // Reorder negative floats to match integer sign handling. It can handle correctly 0 vs -0.
    if (aInt < 0) aInt = 0x80000000 - aInt;
    if (bInt < 0) bInt = 0x80000000 - bInt;

    int intDiff = aInt - bInt;

    // For the case where a is 2.0 and b is -2.0 or vice versa intDiff will be INT_MIN,
    // the most negative number representable by a signed integer.
    // In this case there is no corresponding positive number that can be represented
    // by a signed integer so abs will in fact return INT_MIN which breaks the last comparison
    // intDiff <= max_ulps.
    if (intDiff == INT_MIN)
    {
        return false;
    }
    else
    {
        intDiff = ::abs(aInt - bInt);
    }
    
    return (intDiff <= max_ulps) ? true : false;
    
}


static inline bool double_compare(double a, double b, int max_ulps = 8)
{
    // Make sure max_ulps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    assert(max_ulps > 0 && max_ulps < (1 << 22));

    long long  aLong = *(long long*)&a; // load the float bits into integers
    long long bLong = *(long long*)&b;

    // Comment out the NAN comparison check, and we did not also add the Infinite check.
    // Because these optional checks(Infinite, NAN, etc.) are not necessary in many cases, 
    // or even not desirable. And these checks will probably affact the running efficiency.
    //if ((aInt & 0x7fffffff) > 0x7f800000 || (bInt & 0x7fffffff) > 0x7f800000)
    //    return false; // Remember: NaN != NaN

    // Reorder negative floats to match integer sign handling. It can handle correctly 0 vs -0.
    if (aLong < 0) aLong = 0x8000000000000000L - aLong;
    if (bLong < 0) bLong = 0x8000000000000000L - bLong;

    long long longDiff = (aLong - bLong);

    // For the case where a is 2.0 and b is -2.0 or vice versa intDiff will be INT_MIN,
    // the most negative number representable by a signed integer.
    // In this case there is no corresponding positive number that can be represented
    // by a signed integer so abs will in fact return INT_MIN which breaks the last comparison
    // intDiff <= max_ulps.
    if (longDiff == LLONG_MIN)
    {
        return false;
    }
    else
    {
        longDiff = ::abs(longDiff);
    }
    
    return (longDiff <= max_ulps) ? true : false;
    
}

template <typename P> 
static inline bool vec3_equal(const vec3<P>& v1, const vec3<P>& v2, P EPS)
{
	return IsEqual(v1.x, v2.x, EPS)&&
		   IsEqual(v1.y, v2.y, EPS)&&
		   IsEqual(v1.z, v2.z, EPS);
}

template <typename P> 
static inline bool vec4_equal(const vec4<P>& v1, const vec4<P>& v2, P EPS)
{
	return IsEqual(v1.x, v2.x, EPS)&&
		   IsEqual(v1.y, v2.y, EPS)&&
		   IsEqual(v1.z, v2.z, EPS)&&
           IsEqual(v1.w, v2.w, EPS);
}

template <typename P> 
static inline bool vec2_equal(const vec2<P>& v1, const vec2<P>& v2, P EPS)
{
	return IsEqual(v1.x, v2.x, EPS)&&
		   IsEqual(v1.y, v2.y, EPS);
}

template <typename P> 
static inline bool vec3_lessthan(const vec3<P>& v1, const vec3<P>& v2, P EPS)
{
	 
    if (v1.x < v2.x -EPS )
		return true;
    if (IsEqual(v1.x, v2.x, EPS ))
	{
		if (v1.y < v2.y -EPS)
			return true;
		if (IsEqual(v1.y, v2.y, EPS))
		{
			if (v1.z < v2.z -EPS)
				return true;
		}
	}
	return false ;
}


template <typename P> 
static inline bool vec4_lessthan(const vec4<P>& v1, const vec4<P>& v2, P EPS)
{
    
    if (dot(v1,v1) < dot(v2,v2)- EPS)
        return true;
    return false ;
}

template <typename P> 
static inline bool vec2_lessthan(const vec2<P>& v1, const vec2<P>& v2, P EPS)
{
	if (v1.x < v2.x -EPS )
		return true;
    if (IsEqual(v1.x, v2.x, EPS ))
	{
		if (v1.y < v2.y -EPS)
			return true;
	}
	return false ;
}

static inline bool vec3_lessthan(const vec3<float>& v1, const vec3<float>& v2)
{
	return vec3_lessthan(v1,v2, EPSF);
}

static inline bool vec3_lessthan(const vec3<double>& v1, const vec3<double>& v2)
{
	return vec3_lessthan(v1,v2, (double)EPSF);
}


static inline bool vec3_equal(const vec3<float>& v1, const vec3<float>& v2)
{
	return vec3_equal(v1,v2, EPSF);
}

static inline bool vec3_equal(const vec3<double>& v1, const vec3<double>& v2)
{
	return vec3_equal(v1,v2, (double)EPSF);
}

static inline bool vec4_equal(const vec4<float>& v1, const vec4<float>& v2)
{
	return vec4_equal(v1,v2, EPSF);
}


static inline bool vec2_lessthan(const vec2<float>& v1, const vec2<float>& v2)
{
	return vec2_lessthan(v1,v2, EPSF);
}

static inline bool vec2_lessthan(const vec2<double>& v1, const vec2<double>& v2)
{
	return vec2_lessthan(v1,v2, (double)EPSF);
}

static inline bool vec2_equal(const vec2<float>& v1, const vec2<float>& v2)
{
	return vec2_equal(v1,v2, EPSF);
}

static inline bool vec2_equal(const vec2<double>& v1, const vec2<double>& v2)
{
	return vec2_equal(v1,v2, (double)EPSF);
}



template <typename P = float > 
class Vec2LessThan : public std::binary_function<vec2<P>, vec2<P>, bool> {
public: 
	bool operator() (const vec2<P>& p1, const vec2<P>& p2)
	{
		
		return vec2_lessthan(p1, p2);
	}
};

template <typename P = float > 
class Vec3LessThan : public std::binary_function<vec3<P>, vec3<P>, bool> {
public: 
	bool operator() (const vec3<P>& p1, const vec3<P>& p2)
	{
		return vec3_lessthan(p1, p2);
	}
};

template <typename P>
class Vec3HashCompare :public std::hash_compare<vec3<P> > 
{
public:
  size_t operator()(const vec3<P>& _Key) const 
  { 
	  int value[3];
	  value[0] = round(_Key.x*1000);
	  value[1] = round(_Key.y*1000);
	  value[2] = round(_Key.z*1000);
	  return hashword((const uint32_t*)value, 3, 0);
  }
            
  bool operator()(const vec3<P>& _Keyval1, const vec3<P>& _Keyval2) const 
  { 
	   return vec3_lessthan(_Keyval1, _Keyval2);
  } 
};


template <typename P = float > 
class Vec4LessThan : public std::binary_function<vec4<P>, vec4<P>, bool> {
public: 
	bool operator() (const vec4<P>& p1, const vec4<P>& p2)
	{
		return vec4_lessthan(p1, p2, EPSF);
	}
};


typedef long long IndexPair;

static inline void MakeIndex( const int id[], IndexPair& indexPair )
{
     indexPair = id[1]; 
     indexPair = indexPair << 32; 
     indexPair |= id[0];
}

static inline void GetIDFromIndex(int ID[], const IndexPair& indexPair)
{
        ID[0] = indexPair & 0xffffffff;
        ID[1] = indexPair>>32;
}

class IndexPairCompare : public std::hash_compare<IndexPair> 
{
public:
  size_t operator()(const IndexPair& _Key) const 
  { 
      int ids[2];
      GetIDFromIndex(ids, _Key);
	  if (ids[0] > ids[1])
          std::swap(ids[0], ids[1]);
		  
	  return hashword((const uint32_t*)ids, 2, 0);
  }
            
  bool operator()(const IndexPair& _Keyval1, const IndexPair& _Keyval2) const 
  { 
	   return ( _Keyval1 < _Keyval2);
  } 
};

}

#endif 
