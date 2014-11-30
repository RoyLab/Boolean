#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include "typedefs.h"
#include "arithmetic.h"

namespace GS {

enum {
    KM_EULER_REPEAT_NO =    0,
    KM_EULER_REPEAT_YES =   1
};

enum {
    KM_EULER_PARITY_EVEN =  0,
    KM_EULER_PARITY_ODD =   2
};

enum {
    KM_EULER_AXIS_X =   0,
    KM_EULER_AXIS_Y =   1,
    KM_EULER_AXIS_Z =   2
};

#define KM_EULER_ORDER(Axis, Parity, Repeat)    (((Axis) << 2) | (Parity) | (Repeat))

enum {
    KM_EULER_XYZ = KM_EULER_ORDER(KM_EULER_AXIS_X, KM_EULER_PARITY_EVEN, KM_EULER_REPEAT_NO),
    KM_EULER_XYX = KM_EULER_ORDER(KM_EULER_AXIS_X, KM_EULER_PARITY_EVEN, KM_EULER_REPEAT_YES),
    KM_EULER_XZY = KM_EULER_ORDER(KM_EULER_AXIS_X, KM_EULER_PARITY_ODD,  KM_EULER_REPEAT_NO),
    KM_EULER_XZX = KM_EULER_ORDER(KM_EULER_AXIS_X, KM_EULER_PARITY_ODD,  KM_EULER_REPEAT_YES),
    KM_EULER_YZX = KM_EULER_ORDER(KM_EULER_AXIS_Y, KM_EULER_PARITY_EVEN, KM_EULER_REPEAT_NO),
    KM_EULER_YZY = KM_EULER_ORDER(KM_EULER_AXIS_Y, KM_EULER_PARITY_EVEN, KM_EULER_REPEAT_YES),
    KM_EULER_YXZ = KM_EULER_ORDER(KM_EULER_AXIS_Y, KM_EULER_PARITY_ODD,  KM_EULER_REPEAT_NO),
    KM_EULER_YXY = KM_EULER_ORDER(KM_EULER_AXIS_Y, KM_EULER_PARITY_ODD,  KM_EULER_REPEAT_YES),
    KM_EULER_ZXY = KM_EULER_ORDER(KM_EULER_AXIS_Z, KM_EULER_PARITY_EVEN, KM_EULER_REPEAT_NO),
    KM_EULER_ZXZ = KM_EULER_ORDER(KM_EULER_AXIS_Z, KM_EULER_PARITY_EVEN, KM_EULER_REPEAT_YES),
    KM_EULER_ZYX = KM_EULER_ORDER(KM_EULER_AXIS_Z, KM_EULER_PARITY_ODD,  KM_EULER_REPEAT_NO),
    KM_EULER_ZYZ = KM_EULER_ORDER(KM_EULER_AXIS_Z, KM_EULER_PARITY_ODD,  KM_EULER_REPEAT_YES)
};

const int kMBSphericXYZ = KM_EULER_ZYZ + 1;

const int KmEulerAxis[][3] = {
        { KM_EULER_AXIS_X, KM_EULER_AXIS_Y, KM_EULER_AXIS_Z },  // KM_EULER_XYZ
        { KM_EULER_AXIS_X, KM_EULER_AXIS_Y, KM_EULER_AXIS_Z },  // KM_EULER_XYX
        { KM_EULER_AXIS_X, KM_EULER_AXIS_Z, KM_EULER_AXIS_Y },  // KM_EULER_XZY
        { KM_EULER_AXIS_X, KM_EULER_AXIS_Z, KM_EULER_AXIS_Y },  // KM_EULER_XZX
        { KM_EULER_AXIS_Y, KM_EULER_AXIS_Z, KM_EULER_AXIS_X },  // KM_EULER_YZX
        { KM_EULER_AXIS_Y, KM_EULER_AXIS_Z, KM_EULER_AXIS_X },  // KM_EULER_YZY
        { KM_EULER_AXIS_Y, KM_EULER_AXIS_X, KM_EULER_AXIS_Z },  // KM_EULER_YXZ
        { KM_EULER_AXIS_Y, KM_EULER_AXIS_X, KM_EULER_AXIS_Z },  // KM_EULER_YXY
        { KM_EULER_AXIS_Z, KM_EULER_AXIS_X, KM_EULER_AXIS_Y },  // KM_EULER_ZXY
        { KM_EULER_AXIS_Z, KM_EULER_AXIS_X, KM_EULER_AXIS_Y },  // KM_EULER_ZXZ
        { KM_EULER_AXIS_Z, KM_EULER_AXIS_Y, KM_EULER_AXIS_X },  // KM_EULER_ZYX
        { KM_EULER_AXIS_Z, KM_EULER_AXIS_Y, KM_EULER_AXIS_X }   // KM_EULER_ZYZ
    };

static inline float4 quat_identity()
{
    return cfloat4(0, 0, 0, 1);
}


/*
 * quaternion conjugate
 */
template <typename P > 
static inline vec4<P> quat_conj(const vec4<P>& q)
{
    return vec4<P>(-q.xyz, q.w);
}

/*
 * Grassman quaternion product { q1*q2 == (v1^v2 + s2v1 + s1v2, s1s2 - v1.v2) }
 */
template <typename P > 
static inline vec4<P> quat_mul(const vec4<P>& q1, const vec4<P>& q2)
{
    return quat_conj(q1.zxyw*q2.yzxw - q1.ywzx*q2.zywx - q1.wyxz*q2.xwyz - q1.xzwy*q2.wxzy);
}

/*
 * { ~(q1*q2) == (v2^v1 - s2v1 - s1v2, s1s2 - v1.v2) }
 */
template <typename P> 
static inline vec4<P> conj_quat_mul(const vec4<P>& q1, const vec4<P>& q2)
{

    return q1.zxyw*q2.yzxw - q1.ywzx*q2.zywx - q1.wyxz*q2.xwyz - q1.xzwy*q2.wxzy;

}

/*
 * { ~q1*q2 == (v1^v2 + s2v1 - s1v2, s1s2 + v1.v2) }
 */
template <typename P > 
static inline vec4<P> quat_inv_mul(const vec4<P>& q1,  const vec4<P>& q2)
{
    const vec4<P> iq1 = quat_conj(q1);

    return quat_conj(iq1.zxyw*q2.yzxw - iq1.ywzx*q2.zywx - iq1.wyxz*q2.xwyz - iq1.xzwy*q2.wxzy);
}

template <typename P > 
static inline P quat_norm(const vec4<P>& q)
{
    return dot(q);
}

/*
 * quaternion inverse (to invert an already unit quaternion, use quat_conj)
 */
template <typename P > 
static inline vec4<P> quat_inverse(const vec4<P>& q)
{
    return normalize(quat_conj(q));
}

/*
 *  quaternion mirror about x axis
 */
template <typename P > 
static inline vec4<P> quat_mirror_z(const vec4<P>& q)
{

    return vec4<P>(-q.z, q.w, -q.x, q.y);

}

template <typename P > 
static inline vec4<P> quat_lerp(const vec4<P>& p, const vec4<P>& q,  P t)
{
   return normalize(p + t*(q*sgn(dot(p, q)) - p));
}

/*
 * quaternion spherical linear interpolation
 */
template <typename P > 
static inline vec4<P> quat_slerp(const vec4<P>& a, const vec4<P>& b,  P t)
{
    P aw, bw;

    const P c = dot(a, b);

    if(::abs(c) < cfloat1(1.f - EPSF)) {

        const P rs = rsqrt(cfloat1(1) - c*c);
        const P theta = ::acos(c);
        const P ttheta = t*theta;

        aw = sin(theta - ttheta) * rs;
        bw = sin(ttheta) * rs;

    } else {

        aw = cfloat1(1) - t;
        bw = t;
//      b *= sign(c); // ?
    }
    return a*aw + b*bw;
}

/*
 * spherical cubic interpolation (bilinear interpolation on a quadrilateral, spherical quadrangle)
 */
static inline float4 quat_squad(const float4& q1, const float4& q2, const float4& q3, const float4& q4, float1 t)
{
    return quat_slerp(quat_slerp(q1, q4, t), quat_slerp(q2, q3, t), (t + t)*(cfloat1(1) - t));
}

// quaternion from axis and angle (u must be unit)
static inline float4 quat_axis_rotate(float1 rad, const float4& u)
{
    return point(u)*sincos(cfloat1(.5)*rad);
}

// not-unit axis and angle from unit quaternion
static inline float1 quat_rotation_angle(const float4& q)
{
    const float1 rad = ::acos(float1(q.w));

    return rad + rad;
}

static inline float4 quat_rotation_axis(const float4& q)
{
    return normalize(vector(q));
}

/*
 *  returns a quaternion to rotate direction u to direction v, assuming u != -v
 */
static inline float4 quat_arc_rotate(const float4& u, const float4& v)
{
    return normalize(float4(cross(u, v).xyz, dot(u, v) + ::sqrt(dot(u)*dot(v))));
}

// euler are stored in xyz (radians) while the transformations are in zyx order
static inline float4 quat_euler_rotate(const float4& euler)
{
    float4 s, c; sincos(cfloat1(.5)*euler.xyz, s.xyz, c.xyz);

    const float4 t = float4(s.x*c.z, s.x*s.z, c.x*s.z, c.x*c.z);

    return c.y*t + s.y*cfloat4(-1, 1, -1, 1)*t.zwxy;
}

static inline float4 quat_rotation_euler(const float4& q)
{
    float4 euler;

    const float4 x = q.x*q;
    const float4 y = q.y*q;
    const float1 discr = x.z - y.w;

    if(discr >= cfloat1(.5 - EPSF)) {
        euler = vector(atan2(float1(x.w - y.z), float1(-x.z - y.w)), -cfloat1(PI_2), cfloat1(0));
    } else {
        const float4 w = q.wwwz*q.wwzz - cfloat4(.5, 0, 0, 0);
        if(discr <= cfloat1(EPSF - .5)) {
            euler = vector(atan2(float1(x.y - w.z), float1(y.y + w.x)), cfloat1(PI_2), cfloat1(0));
        } else {
            euler = vector(atan2(float1(x.w + y.z), float1(w.w + w.x)), -asin(discr + discr), atan2(float1(x.y + w.z), float1(x.x + w.x)));
        }
    }
    return euler;
}

// direction cosines from unit quaternion
static inline void quat_dcos(const float4& q, float4& u, float4& v, float4& w)
{
    const float4 qw = q.w*q - cfloat4(0, 0, 0, .5);

    u = q.x*q + cfloat4(1, 1, -1, -1)*qw.wzyx; u += u;
    v = q.y*q + cfloat4(-1, 1, 1, -1)*qw.zwxy; v += v;
    w = q.z*q + cfloat4(1, -1, 1, -1)*qw.yxwz; w += w;
}

static inline float4 quat_dcosx(const float4& q)
{
    const float4 qw = q.w*q - cfloat4(0, 0, 0, .5);
    const float4 u = q.x*q + cfloat4(1, 1, -1, -1)*qw.wzyx;
    
    return u + u;
}

static inline float4 quat_dcosy(const float4& q)
{
    const float4 qw = q.w*q - cfloat4(0, 0, 0, .5);
    const float4 v = q.y*q + cfloat4(-1, 1, 1, -1)*qw.zwxy;

    return v + v;
}

static inline float4 quat_dcosz(const float4& q)
{
    const float4 qw = q.w*q - cfloat4(0, 0, 0, .5);
    const float4 w = q.z*q + cfloat4(1, -1, 1, -1)*qw.yxwz;

    return w + w;
}

// bob's spherical parametrization
static inline float4 quat_spherical_rotate(const float4& coord)
{
    float4 s, c; sincos(cfloat1(.5)*coord.xyz, s.xyz, c.xyz);

    return normalize(float4(s.x*c.y*c.z, s.y*c.x + s.z*s.x, s.z*c.x - s.y*s.x, c.x*c.y*c.z));
}

static inline float4 quat_rotation_spherical(const float4& q)
{
    const float4 dcosx = quat_dcosx(normalize(q));

    const float4 qzy = normalize(float4( cfloat1(0.0), -dcosx.z, dcosx.y, cfloat1(1.0)+dcosx.x));

    const float1 a = qzy.y * qzy.y * qzy.z * qzy.z;
    
    float1 s = cfloat1(1.0);

    if(a > cfloat1(EPSF))
    {
        const float1 d = max(cfloat1(0.0),cfloat1(1.0)-cfloat1(4.0)*a);
        s = ::sqrt(cfloat1(0.5)/a*(cfloat1(1.0)- ::sqrt(d)));
    }

    
    const float1 x     = ::abs( float1(q.w) ) > cfloat1(EPSF)?  atan( float1(q.x/q.w) ): cfloat1(PI_2);
    const float4 coord = vector( x, asin(s*float1(qzy.y)),asin(s*float1(qzy.z)));
    return coord + coord;
}
// get unit quaternion from rotation matrix
static inline float4 quat_matrix_rotate(const float4& u, const float4& v, const float4& w)
{
    float4 q;

    if(u.x >= cfloat1(0)) {
        const float1 t = v.y + w.z;
        if(t >= cfloat1(0)) {
            q = float4(v.z - w.y, w.x - u.z, u.y - v.x, cfloat1(1) + u.x + t);
        } else {
            q = float4(cfloat1(1) + u.x - t, u.y + v.x, w.x + u.z, v.z - w.y);
        }
    } else {
        const float1 t = v.y - w.z;
        if(t >= cfloat1(0)) {
            q = float4(u.y + v.x, cfloat1(1) - u.x + t, v.z + w.y, w.x - u.z);
        } else {
            q = float4(w.x + u.z, v.z + w.y, cfloat1(1) - u.x - t, u.y - v.x);
        }
    }
    return normalize(q);
}

// rotation matrix from unit quaternion
static inline float4x4 quat_rotation_matrix(const float4& q)
{
    float4x4 m;

    const float4 qw = q.w*q - cfloat4(0, 0, 0, .5);

    m[0] = q.x*q + cfloat4(1, 1, -1, -1)*qw.wzyx; m[0] += m[0];
    m[1] = q.y*q + cfloat4(-1, 1, 1, -1)*qw.zwxy; m[1] += m[1];
    m[2] = q.z*q + cfloat4(1, -1, 1, -1)*qw.yxwz; m[2] += m[2];
    m[3] = cfloat4(0, 0, 0, 1);

    return m;
}

// rigid transform matrix from unit quaternion
static inline float4x4 quat_rigid_matrix(const float4& t, const float4& q)
{
    float4x4 m;

    const float4 qw = q.w*q - cfloat4(0, 0, 0, .5);



    m[0] = q.x*q + cfloat4(1, 1, -1, -1)*qw.wzyx; m[0] += m[0];
    m[1] = q.y*q + cfloat4(-1, 1, 1, -1)*qw.zwxy; m[1] += m[1];
    m[2] = q.z*q + cfloat4(1, -1, 1, -1)*qw.yxwz; m[2] += m[2];
    m[3] = point(t);
    return m;
}

// affine transform matrix from unit quaternion
static inline float4x4 quat_affine_matrix(const float4& t, const float4& q, const float4& s)
{
    float4x4 m;

    const float4 qw = q.w*q - cfloat4(0, 0, 0, .5);


    m[0] = q.x*q + cfloat4(1, 1, -1, -1)*qw.wzyx; m[0] = s.x*(m[0] + m[0]);
    m[1] = q.y*q + cfloat4(-1, 1, 1, -1)*qw.zwxy; m[1] = s.y*(m[1] + m[1]);
    m[2] = q.z*q + cfloat4(1, -1, 1, -1)*qw.yxwz; m[2] = s.z*(m[2] + m[2]);
    m[3] = point(t);


    return m;
}

// rotates a vector by a unit quaternion
template <typename P>
static inline vec4<P> quat_transform(const vec4<P>& q, const vec4<P>& p)
{

    const vec4<P> v = vector(p + p);
    const vec4<P> qv = q*v;
    return q.w*(cross(q, v) + q.w*v) + q*(qv.xyzw + qv.yzxw + qv.zxyw) + quat_conj(p);

}

// Euler to quaternion with given rotation order.
static inline float4 quat_euler_rotate_ordered( const float4& euler, int order)
{
    
    if(order == KM_EULER_XYZ)
        return quat_euler_rotate(euler);
    
    else if (order == kMBSphericXYZ)
        return quat_spherical_rotate(euler);

    else
    {       
        float4 s, c; sincos(cfloat1(.5)*euler.xyz, s.xyz, c.xyz);

        float4 lQ[3];
        lQ[0] = float4(s.x, cfloat1(0.0f), cfloat1(0.0f),  c.x);
        lQ[1] = float4(cfloat1(0.0f), s.y,  cfloat1(0.0f), c.y);
        lQ[2] = float4(cfloat1(0.0f), cfloat1(0.0f), s.z,  c.z);
    
        lQ[KmEulerAxis[order][2]] = quat_mul(lQ[KmEulerAxis[order][2]] , lQ[KmEulerAxis[order][1]]);
        lQ[KmEulerAxis[order][2]] = quat_mul(lQ[KmEulerAxis[order][2]] , lQ[KmEulerAxis[order][0]]);

        return lQ[KmEulerAxis[order][2]]; 
    }   
}

// Quaternion to Euler with given rotation order.
static inline float4 quat_rotation_euler_ordered(const float4& q, int order )
{           
    float4 euler;
    switch(order)
    {
    case KM_EULER_XYZ:          
            return quat_rotation_euler(q);
                
    case KM_EULER_ZYX:
        {               
            const float4 x = q.x*q;
            const float4 y = q.y*q;             
            const float1 discr = x.z + y.w;                                             
            
            if(discr >= cfloat1(.5 - EPSF)){                                                                              
                euler = vector( cfloat1(0), cfloat1(PI_2), atan2(float1(y.z + x.w), float1(y.w - x.z)));
            }else if(discr <= cfloat1(EPSF - .5)) {                                       
                    euler = vector( cfloat1(0), -cfloat1(PI_2), atan2(float1(-y.z - x.w), float1(-y.w + x.z)));
            }else {
                    const float4 w = q.wwwz*q.wwzz - cfloat4(.5, 0, 0, 0);
                    euler = vector(atan2(float1(x.w - y.z), float1(w.w + w.x)), asin(discr + discr), atan2(float1(-x.y + w.z), float1(x.x + w.x)));                 
            }
            return euler;
        }
    case KM_EULER_XZY:      
        {                               
            const float4 y = q.y*q;
            const float4 w = q.wwwz*q.wwzz - cfloat4(.5, 0, 0, 0);                                                                                                          
            const float1 discr = y.x + w.z;

            if( discr >= cfloat1(.5 - EPSF)){
                euler = vector(cfloat1(0), cfloat1(2.f) * atan2(float1(q.x), float1(q.w)), cfloat1(PI_2) );
            } else if (discr <= cfloat1(EPSF - .5))   {
                euler = vector(cfloat1(0), cfloat1(-2.f) * atan2(float1(q.x), float1(q.w)), -cfloat1(PI_2));
            }else {
                const float4 x = q.x*q; 
                euler = vector( atan2(float1(x.w - y.z) ,float1(y.y + w.x)), atan2(float1(y.w-x.z), float1(x.x + w.x)) , asin(discr + discr));                                      
            }
            return euler;
        }
    case KM_EULER_YZX:
        {               
            const float4 z = q.z*q;
            const float4 y = q.y*q;                                                             
            const float1 discr = z.w - y.x;
            
            if( discr >= cfloat1(.5 - EPSF)){
                euler = vector(atan2(float1(z.x - y.w), float1(y.x + z.w)), cfloat1(0), cfloat1(PI_2) );
            }else if (discr <= cfloat1(EPSF - .5)){                                               
                euler = vector(atan2(float1(y.w - z.x), float1(-y.x - z.w)), cfloat1(0), -cfloat1(PI_2) );
            }else{                      
                const float4 w = q.wxyz*q.wwwz - cfloat4(.5, 0, 0, 0);
                euler = vector( atan2(float1(w.y + y.z), float1(y.y + w.x)), atan2(float1(y.w + z.x), float1(cfloat1(.5f) - y.y - z.z)) , asin(discr + discr));                                     
            }
            return euler;
        }

    case KM_EULER_ZXY:
        {               
            const float4 x = q.x*q;
            const float4 y = q.y*q;             
            const float1 discr = x.w - y.z;
            
            if( discr >= cfloat1(.5 - EPSF)){
                euler = vector(cfloat1(PI_2), atan2(float1(y.w - x.z), float1(y.z + x.w)), cfloat1(0));
            }else if (discr <= cfloat1(EPSF - .5)){
                euler = vector(-cfloat1(PI_2), atan2(float1(y.w - x.z), float1(-y.z - x.w)), cfloat1(0));                 
            }else{
                const float4 w = q.wwwz*q.wwzz - cfloat4(.5, 0, 0, 0);
                euler = vector(asin(discr + discr) , atan2(float1(x.z + y.w) ,float1(w.w + w.x)), atan2(float1(x.y + w.z) ,float1(y.y + w.x)));             
            }
            return euler;
        }

    case KM_EULER_YXZ:
        {           
            const float4 x = q.x*q;
            const float4 y = q.y*q;     
            const float1 discr = x.w + y.z;
            
            if( discr >= cfloat1(.5 - EPSF))  {
                euler = vector(cfloat1(PI_2), cfloat1(0),  atan2(float1(x.z + y.w), float1(x.w - y.z)));
            } else if (discr <= cfloat1(EPSF - .5)) {                 
                    euler = vector(-cfloat1(PI_2), cfloat1(0),  atan2(float1(-x.z - y.w), float1(-x.w + y.z)));
            }else { 
                const float4 w = q.wwwz*q.wwzz - cfloat4(.5, 0, 0, 0);      
                euler = vector(asin(discr + discr) , atan2(float1(y.w - x.z) ,float1(w.w + w.x)), atan2(float1(w.z - x.y ) ,float1(y.y + w.x)));
            }
            return euler;       
        }

    case kMBSphericXYZ:
        {
                return quat_rotation_spherical(q);
        }   
    }   

    return euler ;
}



}


#endif 
