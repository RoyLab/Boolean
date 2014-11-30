#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat2.h"
#include "mat3.h"
#include "mat4.h"
namespace GS{
	//int vector definition
    typedef vec2<int>		int2;
    typedef vec3<int>		int3;
    typedef vec4<int>		int4;
	
	//float vector definition 
	typedef vec2<>		float2;
	typedef vec3<>		float3;
    typedef vec4<>		float4;

	//double vector definition 
	typedef vec2<double>		double2;
	typedef vec3<double>		double3;
	typedef vec4<double>		double4;

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

	// matrix definition
    typedef mat2<>					float2x2;
	typedef mat3<>					float3x3;
	typedef mat4<>					float4x4;
    typedef mat2<double>					double2x2;
	typedef mat3<double>					double3x3;
	typedef mat4<double>					double4x4;

}


#endif 