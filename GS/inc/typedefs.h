#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_
#pragma warning(disable:4005)
#include <functional>
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat2.h"
#include "mat3.h"
#include "mat4.h"
namespace GS{

#define EPSF                  1e-5f


#ifndef PI
#define PI            3.1415926535897932384626433832795028841971693993751
#endif

#ifndef PI_2
#define PI_2          1.5707963267948966192313216916397514420985846996875
#endif

#ifndef PI_4
#define PI_4          .78539816339744830961566084581987572104929234984377
#endif
//int vector definition
typedef vec2<int>		      int2;
#define cint2(x, y)			  vec2<int>(x, y)
typedef vec3<int>		      int3;
#define cint3(x, y, z)		  const vec3<int>(x, y, z)
typedef vec4<int>		      int4;
	
//float vector definition 

typedef float                  float1;
#define cfloat1(x)			   float(x)
typedef vec2<>		           float2;
#define cfloat2(x, y)		   const vec3<>(x, y)
typedef vec3<>		           float3;
#define cfloat3(x, y, z)       const vec3<>(x, y, z)
typedef vec4<>		           float4;
#define cfloat4(x, y, z, w)	   vec4<>(float(x), float(y), float(z), float(w))

//double vector definition 
typedef double                 double1;
typedef vec2<double>		   double2;
#define cdouble2(x, y)   	   vec2<double>(x, y)
typedef vec3<double>		   double3;
#define cdouble3(x, y, z)      vec2<double>(x, y, z)
typedef vec4<double>		   double4;
#define cdouble4(x, y, z, w)   vec2<double>(x, y, z, w)


	template<> inline vec2<int>::vec2()
{
    x = 0;
    y = 0;
}

template<> inline vec3<int>::vec3()
{
    x = 0;
    y = 0;
    z = 0;
}

template<> inline vec4<int>::vec4()
{
    x = 0;
    y = 0;
    z = 0;
    w = 0;
}



template <typename P> static inline vec4<P> point(P x, P y, P z)
{
    return vec4<P>(x, y, z, 1);
}

template <typename P> static inline vec4<P> point(P s)
{
    return vec4<P>(s, 0, 0, 1);
}
template<typename R> static inline vec4<typename R::scalar> point(const rhs2<R> &v)
{
    return vec4<R>(v.x, v.y, 0, 1);
}
template<typename R> static inline vec4<typename R::scalar> point(const rhs3<R> &v)
{
    return vec4<typename R::scalar>(v, 1);
}
template<typename R> static inline vec4<R> point(const vec2<R> &v)
{
    return vec4<R>(v.x, v.y, 0, 1);
}
template<typename R> static inline vec4<R> point(const vec3<R> &v)
{
    return vec4<R>(v, 1);
}

template <typename P> static inline vec4<P> point(const vec4<P> &v)
{
    // for consistency with SIMD
    return vec4<P>(v.xyz, v.w + 1.f);
}

template <typename P> static inline vec4<P> vector(const P &x)
{
    return vec4<P>(x, 0, 0, 0);
}

template <typename P> static inline vec4<P> vector(const P &x, const P &y)
{
    return vec4<P>(x, y, 0, 0);
}

template <typename P> static inline vec4<P> vector(const P &x, const P &y, const P &z)
{
    return vec4<P>(x, y, z, 0);
}

template<typename R> static inline vec4<typename R::scalar> vector(const rhs2<R> &v)
{
    return vec4<R>(v.x, v.y, 0, 0);
}

template<typename R> static inline vec4<R> vector(const vec2<R> &v)
{
    return vec4<R>(v.x, v.y, 0, 0);
}

template<typename R> static inline vec4<typename R::scalar> vector(const rhs3<R> &v)
{
    return vec4<R>(v, 0);
}

template<typename R> static inline vec4<R> vector(const vec3<R> &v)
{
    return vec4<R>(v, 0);
}

template<typename R> static inline vec4<typename R::scalar> vector(const rhs4<R> &v)
{
    return vec4<typename R::scalar>(v.x, v.y, v.z, 0);
}

template<typename R> static inline vec4<R> vector(const vec4<R> &v)
{
    return vec4<R>(v.x, v.y, v.z, 0);
}

// matrix definition
typedef mat2<>					float2x2;
typedef mat3<>					float3x3;
typedef mat4<>				    float4x4;
typedef mat2<double>			double2x2;
typedef mat3<double>			double3x3;
typedef mat4<double>			double4x4;


	
enum RelationToPlane{
    Front, 
    On,
    Behind, 
    Straddling
};

}


#endif 