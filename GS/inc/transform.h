#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_
#include "typedefs.h"
#include "arithmetic.h"
namespace GS {
// t.w == 0
static inline float4x4 translate(const float4& t)
{
    return float4x4(
        cfloat4(1, 0, 0, 0),
        cfloat4(0, 1, 0, 0),
        cfloat4(0, 0, 1, 0),
        t + cfloat4(0, 0, 0, 1)
    );
}

// t.w == 0
// This is the safe translation method, in which the incoming matrix is
// multiplied by the translation matrix. If the incoming matrix is affine,
// translate_affine is faster.
static inline float4x4 translate(float4& t, const float4x4& m)
{
    return mul(translate(t), m );
}

// u == unit
static inline float4x4 rotate(float1 rad, float4 u)
{
    float1 s, c; sincos(rad, s, c);
    const float4 uh = u*(cfloat1(1) - c);
    const float4 us = u*s;
    return float4x4(
        u.x*uh + float4(c, us.z, -us.y, cfloat1(0)),
        u.y*uh + float4(-us.z, c, us.x, cfloat1(0)),
        u.z*uh + float4(us.y, -us.x, c, cfloat1(0)),
        cfloat4(0, 0, 0, 1)
    );

}

// s.w == 1
static inline float4x4 scale(float4 s)
{
    return float4x4(
        float4(s.x, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), s.y, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), s.z, cfloat1(0)),
        cfloat4(0, 0, 0, 1)
    );
}

// s.w == 1
static inline float4x4 inv_scale(float4 s)
{
    return float4x4(
        float4(cfloat1(1)/s.x, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(1)/s.y, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), cfloat1(1)/s.z, cfloat1(0)),
        cfloat4(0, 0, 0, 1)
    );
}

// s.w == 1
// This is the safe scale method, in which the incoming matrix is
// multiplied by the scale matrix. If the incoming matrix is affine,
// scale_affine is faster.
static inline float4x4 scale( float4 t,  const float4x4& m)
{
    return mul( scale(t), m );
}

static inline float4x4 ortho( float1 l,  float1 r,  float1 b,  float1 t,  float1 n,  float1 f)
{
    const float1 rdx = cfloat1(1)/(r - l);
    const float1 rdy = cfloat1(1)/(t - b);
    const float1 rdz = cfloat1(1)/(n - f);

    return float4x4(

        float4(rdx + rdx, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), rdy + rdy, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), rdz + rdz, cfloat1(0)),
        float4(-(r + l)*rdx, -(t + b)*rdy, (n + f)*rdz, cfloat1(1))

        );
}


static inline float4x4 inv_ortho( float1 l, float1 r, float1 b, float1 t, float1 n, float1 f)
{
    const float1 dx = r - l;
    const float1 dy = t - b;
    const float1 dz = n - f;

    return float4x4(

        float4(cfloat1(.5)*dx, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(.5)*dy, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), cfloat1(.5)*dz, cfloat1(0)),
        float4(cfloat1(.5)*(r + l), cfloat1(.5)*(t + b), cfloat1(.5)*(n + f), cfloat1(1))

        );
}


/*****************************************************************************

 GLU-like transformations

 *****************************************************************************/

static inline float4x4 ortho2d(float1 l, float1 r, float1 b, float1 t)
{
    const float1 rdx = cfloat1(1)/(r - l);
    const float1 rdy = cfloat1(1)/(t - b);

    return float4x4(

        float4(rdx + rdx, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), rdy + rdy, cfloat1(0), cfloat1(0)),
        cfloat4(0, 0, -2, 0),
        float4(-(r + l)*rdx, -(t + b)*rdy, cfloat1(-1), cfloat1(1))

        );
}

// eye.w == 0, center.w == 0, up.w == 0
static inline float4x4 lookat (const float4& eye, const float4& center, const float4& up)
{

    const float4 f = normalize(center - eye);
    const float4 s = normalize(cross(f, up));
    const float4 u = cross(s, f);
    
    return mul(transpose(float4x4(s, u, -f, cfloat4(0, 0, 0, 1))), translate(-eye));

}

static inline float4x4 perspective(float1 fovy, float1 aspect, float1 znear, float1 zfar)
{
    const float1 f = tan( cfloat1(2)/fovy );

    return float4x4(
        float4(f/aspect, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), f, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), (zfar + znear)/(znear - zfar), cfloat1(-1)),
        float4(cfloat1(0), cfloat1(0), cfloat1(2)*zfar*znear/(znear - zfar), cfloat1(0))
    );

}

static inline float4x4 inv_perspective (float1 fovy, float1 aspect, float1 znear, float1 zfar)
{
    const float1 rf = tan(cfloat1(.5)*fovy);
    const float1 rfn = cfloat1(.5)/(zfar*znear);


    return float4x4(
        float4(aspect*rf, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), rf, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), cfloat1(0), (znear - zfar)*rfn),
        float4(cfloat1(0), cfloat1(0), cfloat1(-1), (zfar + znear)*rfn)
    );

}

static inline float4 project( float4 obj, float4x4 model, float4x4 proj, float4 view)
{
    const float4 v = mul(proj, mul(model, obj));
    float4 w = cfloat1(.5)*v + cfloat1(.5);

    w.xy *= view.zw;
    w.xy += view.xy;

    return w;
}


static inline float4x4 inv_ortho2d(float1 l, float1 r, float1 b, float1 t, float1 /*n*/, float1 /*f*/)
{
    const float1 dx = r - l;
    const float1 dy = t - b;

    return float4x4(

        float4(cfloat1(.5)*dx, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(.5)*dy, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), cfloat1(-.5), cfloat1(0)),
        float4(cfloat1(.5)*(r + l), cfloat1(.5)*(t + b), cfloat1(.5), cfloat1(1))

        );
}

// eye.w == 0, center.w == 0, up.w == 0
static inline float4x4 inv_lookat( float4 eye, float4 center, float4 up)
{

    const float4 l = normalize(center - eye);
    const float4 s = normalize(cross(l, up));
    const float4 u = cross(s, l);
    
    return float4x4(s, u, -l, eye);

}


static inline float4x4 rotate_x(float1 s, float1 c)
{

    return float4x4(
        cfloat4(1, 0, 0, 0),
        float4(cfloat1(0), c, s, cfloat1(0)),
        float4(cfloat1(0), -s, c, cfloat1(0)),
        cfloat4(0, 0, 0, 1)
    );

}
static inline float4x4 rotate_x(float1 rad)
{
    float1 s, c; sincos(rad, s, c);

    return rotate_x(s, c);
}

static inline float4x4 rotate_y(float1 s, float1 c)
{

    return float4x4(
        float4(c, cfloat1(0), -s, cfloat1(0)),
        cfloat4(0, 1, 0, 0),
        float4(s, cfloat1(0), c, cfloat1(0)),
        cfloat4(0, 0, 0, 1)
    );

}
static inline float4x4 rotate_y(float1 rad)
{
    float1 s, c; sincos(rad, s, c);

    return rotate_y(s, c);
}

static inline float4x4 rotate_z( float1 s,  float1 c)
{

    return float4x4(
        float4(c, s, cfloat1(0), cfloat1(0)),
        float4(-s, c, cfloat1(0), cfloat1(0)),
        cfloat4(0, 0, 1, 0),
        cfloat4(0, 0, 0, 1)
    );
}

static inline float4x4 rotate_z( float1 rad)
{
    float1 s, c; sincos(rad, s, c);

    return rotate_z(s, c);
}



// rotates around z, then y, then x
static inline float4x4 rotate_euler(float4 euler)
{
    float4 s, c; sincos(euler.xyz, s.xyz, c.xyz);

    const float1 sysx = s.y*s.x;
    const float1 sycx = s.y*c.x;


    return float4x4(
        float4(c.z*c.y, s.z*c.y, -s.y, cfloat1(0)),
        float4(c.z*sysx - s.z*c.x, s.z*sysx + c.z*c.x, c.y*s.x, cfloat1(0)),
        float4(c.z*sycx + s.z*s.x, s.z*sycx - c.z*s.x, c.y*c.x, cfloat1(0)),
        cfloat4(0, 0, 0, 1)
    );

}

static inline float4x4 scale(float1 s)
{
    return float4x4(
        float4(s, cfloat1(0), cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), s, cfloat1(0), cfloat1(0)),
        float4(cfloat1(0), cfloat1(0), s, cfloat1(0)),
        cfloat4(0, 0, 0, 1)
    );
}

static inline float4x4 shear_z(float1 x,  float1 y)
{
    return float4x4(
        float4(cfloat1(1), cfloat1(0), x, cfloat1(0)), 
        float4(cfloat1(0), cfloat1(1), y, cfloat1(0)), 
        cfloat4(0, 0, 1, 0), 
        cfloat4(0, 0, 0, 1));
}


// t.w == 0
static inline float4 translate( float4 t, float4 p)
{
    return p + t;
}

// s.w == 1
static inline float4 scale(float4 s,  float4 p)
{
    return s*p;
}

static inline float4 scale(float1 s, float4 p)
{
    return float4(s, s, s, 1)*p;
}



static inline float1 rotation(float4x4 r,  float4 u)
{
    const float1 c = cfloat1(.5)*(r[0].x + r[1].y + r[2].z - cfloat1(1));
    float1 ang;
    if(c < cfloat1(1 - EPSF)) {

        u = cfloat1(.5)*float4(r[1].z - r[2].y, r[2].x - r[0].z, r[0].y - r[1].x, cfloat1(0)) * rsqrt(cfloat1(1) - c*c);

        ang = ::acos(c);
    } else {
        u = r[0];
        ang = cfloat1(0);
    }
    return ang;
}

}

#endif 
