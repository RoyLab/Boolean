#ifndef _TOPOLOGY_H_
#define _TOPOLOGY_H_
#include <cmath>
#include "typedefs.h"
#include "arithmetic.h"
#include "transform.h"
#include "Intersect.inl"

namespace GS{



template< typename P>
static inline P TriangleArea(const vec2<P>& p0, const vec2<P>& p1, const vec2<P>& p2)
{
    return  ( p0.x* p1.y - p1.x * p0.y + p0.y* p2.x - p0.x * p2.y + p1.x * p2.y - p2.x * p1.y);
}

template <typename P> 
static inline vec2<P> NormalOf(const vec2<P>& p1, const vec2<P>& p2, const vec2<P>& p3 )
{
    return normalize(cross(p3-p1, p2-p1));

}

template <typename P> 
static inline vec3<P> NormalOf(const vec3<P>& p1, const vec3<P>& p2, const vec3<P>& p3 )
{
    //vec2<P>  v1 = p1.yz;
    //vec2<P>  v2 = p2.yz;
    //vec2<P>  v3 = p2.yz;
    //P Syz =  TriangleArea(v1, v2, v3); 
    //v1 = p1.xz;
    //v2 = p2.xz;
    //v3 = p3.xz;
    //P Sxz = TriangleArea(v1 , v2 , v3);

    //v1 = p1.xy;
    //v2 = p2.xy;
    //v3 = p3.xy;
    //P Sxy = TriangleArea(v1 , v2 , v3);

    //return normalize(vec3<P>(Syz, Sxz, Sxy));
    return normalize(cross(p3-p1, p2-p1));
}

template <typename P >
static inline vec2<P>  CenterOfGravity(const vec2<P>& p1, const vec2<P>& p2, const vec2<P>& p3)
{
	vec2<P> v1  =p2- p1;
	vec2<P> v2 = p3-p1;
	return p1+ (v1 +v2)*1/3;
}



template <typename P>
static inline vec3<P>  CenterOfGravity(const vec3<P>& p1, const vec3<P>& p2, const vec3<P>& p3)
{
	vec3<P> v1  =p2 - p1;
	vec3<P> v2 = p3 - p1;
	return p1+ (v1 +v2)*1/3;
}

//http://www.blackpawn.com/texts/pointinpoly/default.html
// Point in triangle test
template <typename P> 
static inline bool PointInTriangle(const vec3<P>& p, const vec3<P>& p1, const vec3<P>& p2, 
								   const vec3<P>& p3, const vec3<P>& normal)
{
	

	P DistToPlane = fabs(dot((p - p1), normal));
	if (DistToPlane >= EPSF)
		return false;
	int max_id = 0 ; 
	if ( fabs(normal[1]) > fabs(normal[0]))
        max_id = 1;
     if (fabs(normal[2]) > fabs(normal[max_id]))  
        max_id = 2;
      int x = (max_id + 1) % 3;
      int y = (max_id + 2) % 3;
      P cu = p[x] - p1[x];
      P cv = p[y] - p1[y];
      P bu = p2[x] - p1[x];
      P bv = p2[y] - p1[y];
      P au = p3[x] - p1[x];
      P av = p3[y] - p1[y];
      double  f = bu*av - bv*au;
      double h1 = (cu*av - cv*au) / f;
      double h2 = (bu*cv - bv*cu) / f;
     // double h1 = (cu*av - cv*au) / (bu*av - bv*au);
     // double h2 = (cu*bv - cv*bu) / (au*bv - av*bu);
      //h1 + h2 + h3 = 1 && h1 > 0 && h2 > 0 && h3 >0
      //以下三角形做了做了加宽处理
      return  ((h1 > -EPSF) && (h2 > -EPSF) && (h1 + h2 < 1 + EPSF*2));

}

template <typename P>
static inline bool IntersectSegmentWithPlane(const vec3<P>& a , const vec3<P>& b, const vec3<P>& n, const P d, vec3<P>& q)
{
    vec3<P> ab = b - a ; 
    P t = (-d -dot(n, a)) /dot(n, ab) ; 
    if ( t >= 0.0f && t<= 1.0f )
    {
        q = a + t * ab; 
        return true ; 
    }
    return false ; 

}



template <typename P> 
static inline bool PointInTriangle(const vec2<P>& p, const vec2<P>& p1, const vec2<P>& p2, const vec2<P>& p3)
								   
{
      P cu = p.x  - p1.x;
      P cv = p.y - p1.y;
      P bu = p2.x - p1.x;
      P bv = p2.y - p1.y;
      P au = p3.x - p1.x;
      P av = p3.y - p1.y;
      double h1 = (cu*av - cv*au) / (bu*av - bv*au);
      double h2 = (cu*bv - cv*bu) / (au*bv - av*bu);
      //h1 + h2 + h3 = 1 && h1 > 0 && h2 > 0 && h3 >0
      //以下三角形做了做了加宽处理
      return  ((h1 > -EPSF) && (h2 > -EPSF) && (h1 + h2 < 1 + EPSF*2));

}

template <typename P>
static inline bool TestTriangleAABBOverlap(const vec3<P>& v0, const vec3<P>& v1, const vec3<P>& v2, const Box3& bbox)
{
    return TriangleAABBIntersectTest(v0, v1, v2, bbox); 
}

template<typename P> 
static inline bool TriangleInterTriangle(const vec3<P>& v0, const vec3<P>& v1, const vec3<P>& v2, const vec3<P>& nv, 
										 const vec3<P>& u0, const vec3<P>& u1, const vec3<P>& u2, const vec3<P>& nu,
										 std::vector<Seg3D<double> >& intersects)
{
	
	return TriTriIntersectTest(v0, v1, v2, nv, u0, u1, u2, nu, intersects);
}

template<typename P> 
static inline bool SignDeterminant(const vec3<P>& v0, const vec3<P>& v1, const vec3<P>& v2, const vec3<P>& v3 )
{
    mat3<P> m;
    m[0] =v0 -v3;
    m[1] = v1 -v3;
    m[2] = v2- v3;
    P det = determinant(m);
    if (det > EPSF) return true;
    return false ; 
}


template <typename P>
static inline vec3<P> PosToLocal(const vec3<P>& U, const vec3<P>& V, const vec3<P>& N, const vec3<P>& origin, const vec3<P>& p)
{
	mat4<P> mat(id4x4());
	mat[0].x = U.x;
	mat[1].x = U.y;
	mat[2].x = U.z;
	mat[0].y = V.x;
	mat[1].y = V.y;
	mat[2].y = V.z;
	mat[0].z = N.x;
	mat[1].z = N.y;
	mat[2].z = N.z;
	mat[0].w = 0;
    mat[1].w = 0;
    mat[2].w = 0;
	mat[3].x = - dot( origin, U);
	mat[3].y = - dot( origin, V);
	mat[3].z = - dot( origin, N);
	return (mul(mat, point(p))).xyz;
	

}


template <typename P>
static inline vec3<P> PosToGlobal(const vec3<P>& U, const vec3<P>& V, const vec3<P>& N, const vec3<P> origin, const vec3<P>& p)
{
	mat4<P> mat(id4x4());
	mat[0].xyz = U;
	mat[1].xyz = V;
	mat[2].xyz = -N;
	mat[3].xyz = origin;
	return (mul(mat, point(p))).xyz;
	

	//mat[0].x = U.x;
	//mat[1].x = U.y;
	//mat[2].x = U.z;
	//mat[0].y = V.x;
	//mat[1].y = V.y;
	//mat[2].y = V.z;
	//mat[0].z = N.x;
	//mat[1].z = N.y;
	//mat[2].z = N.z;
	//mat[0].w = 0;
 //   mat[1].w = 0;
 //   mat[2].w = 0;
	//mat[3].x = - dot( origin, U);
	//mat[3].y = - dot( origin, V);
	//mat[3].z = - dot( origin, N);
	//
	//return (mul(inverse(mat), point(p))).xyz;
}



}
#endif 