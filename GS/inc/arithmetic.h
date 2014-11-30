#ifndef _ARITHMETIC_H_
#define _ARITHMETIC_H_
#include <math.h>
#include "typedefs.h"

namespace GS{

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif 

static inline float rsqrt(float x)
{
    return 1.f / (float) ::sqrt(x);
}

static inline double rsqrt(double x)
{
    return 1.0 / ::sqrt(x);
}

static inline float sgn(float x)
{
    return (x >= 0) ? 1.f : -1.f;
}

static inline double sgn(double x)
{
    return (x >= 0) ? 1.f : -1.f;
}

static inline double min(double a, double b)
{
    return (a < b) ? a : b;
}


static inline double max(double a, double b)
{
    return (a > b) ? a : b;
}

static inline double clamp(double x, double a, double b)
{
    return min(max(x, a), b);
}

static inline double saturate(double x)
{
    return clamp(x, 0.f, 1.f);
}

static inline double trunc(double x)
{
    return (double) (long) x;
}

static inline void sincos(float x, float &s, float &c)
{
    s = sin(x);
    c = cos(x);
}

static inline double round(double x)
{
    return (x > 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
}

template<typename R>
static inline typename R::scalar sum(const rhs2<R> &r)
{
    return r.x + r.y;
}

template<typename R>
static inline typename R::scalar sum(const rhs3<R> &r)
{
    return r.x + r.y + r.z;
}

template<typename R>
static inline typename R::scalar sum(const rhs4<R> &r)
{
    return r.x + r.y + r.z + r.w;
}


template<typename R>
inline typename R::scalar min(const rhs2<R> &r)
{
    return min(r.x, r.y);
}

template<typename R>
static inline typename R::scalar min(const rhs3<R> &r)
{
    return min(r.x, min(r.y, r.z));
}

template<typename R>
static inline typename R::scalar min(const rhs4<R> &r)
{
    return min(r.x, min(r.y, min(r.z, r.w)));
}

template<typename R>
static inline typename R::scalar max(const rhs2<R> &r)
{
    return max(r.x, r.y);
}
template<typename R> 
static inline typename R::scalar max(const rhs3<R> &r)
{
    return max(r.x, max(r.y, r.z));
}
template<typename R> 
static inline typename R::scalar max(const rhs4<R> &r)
{
    return max(r.x, max(r.y, max(r.z, r.w)));
}

template<typename L, typename R>
static inline typename L::scalar dot(const rhs2<L> &l, const rhs2<R> &r)
{
    return sum(l*r);
}
template<typename L, typename R>
static inline typename L::scalar dot(const rhs3<L> &l, const rhs3<R> &r)
{
    return sum(l*r);
}
template<typename L, typename R>
static inline typename R::scalar dot(const rhs4<L> &l, const rhs4<R> &r)
{
    return sum(l*r);
}

template<typename R> 
static inline typename R::scalar dot(const rhs2<R> &r)
{
    return sum(r*r);
}
template<typename R>
static inline typename R::scalar dot(const rhs3<R> &r)
{
    return sum(r*r);
}
template<typename R>
static inline typename R::scalar dot(const rhs4<R> &r)
{
    return sum(r*r);
}

template<typename L, typename R>
static inline typename rhs2<L>::scalar cross(const rhs2<L> &a, const rhs2<R> &b)
{
    return (typename rhs2<L>::scalar)(a.x*b.y - a.y*b.x);
}
template <typename R>
static inline vec3<typename R::scalar> cross(const rhs3<R>& a, const rhs3<R>& b)
{
    return vec3<typename R::scalar>((a.y*b.z - a.z*b.y), (a.z * b.x - a.x * b.z), (a.x * b.y - a.y * b.x));
}

template <typename R>
static inline vec4<typename R::scalar> cross(const rhs4<R>& a, const rhs4<R>& b)
{
    return vec4<typename R::scalar>((a.y*b.z - a.z*b.y), (a.z * b.x - a.x * b.z), (a.x * b.y - a.y * b.x), 0);
}

template<typename P>
static inline vec2<P> normalize(const vec2<P> &r)
{
    return r*rsqrt(dot(r));
}

template<typename P>
static inline vec3<P> normalize(const vec3<P> &r)
{
    return r*rsqrt(dot(r));
}

template<typename P>
static inline vec4<P> normalize(const vec4<P> &r)
{
    return r*rsqrt(dot(r));
}
template<typename R> 
static inline typename R::scalar length(const rhs2<R> &r)
{
    return ::sqrt(dot(r));
}
template<typename R> 
static inline typename R::scalar length(const rhs3<R> &r)
{
    return ::sqrt(dot(r));
}


template<typename R> static 
inline typename R::scalar length(const rhs4<R> &r)
{
    return ::sqrt(dot(r));
}


template<typename L, typename R> 
static inline typename R::scalar distance(const rhs2<L> &l, const rhs2<R> &r)
{
    return length(l - r);
}
template<typename L, typename R> 
static inline typename R::scalar distance(const rhs3<L> &l, const rhs3<R> &r)
{
    return length(l - r);
}


template<typename L, typename R> 
static inline typename R::scalar distance(const rhs4<L> &l, const rhs4<R> &r)
{
    return length(l - r);
}

template<typename P>
static inline vec2<P> sgn(const vec2<P> &r)
{
    return vec2<P>(sgn(r.x), sgn(r.y));
}

template<typename P>
static inline vec3<P> sgn(const vec3<P> &r)
{
    return vec3<P>(sgn(r.x), sgn(r.y), sgn(r.z));
}


template<typename P>
static inline vec4<P> sgn(const vec4<P> &r)
{
   return vec4<P>(sgn(r.x), sgn(r.y), sgn(r.z), sgn(r.w));
}


template<typename P>
static inline vec2<P> sign(const vec2<P> &r)
{
    return vec2<P>(sign(r.x), sign(r.y));
}

template<typename P>
static inline vec3<P> sign(const vec3<P> &r)
{
    return vec3<P>(sign(r.x), sign(r.y), sign(r.z));
}


template<typename P>
static inline vec4<P> sign(const vec4<P> &r)
{
    return vec4<P>(sign(r.x), sign(r.y), sign(r.z), sign(r.w));
}


template<typename P>
static inline vec2<P> abs(const vec2<P> &r)
{
    return vec2<P>(abs(r.x), abs(r.y));
}

template<typename P>
static inline vec3<P> abs(const vec3<P> &r)
{
    return vec3<P>(abs(r.x), abs(r.y), abs(r.z));
}

template<typename P>
static inline vec4<P> abs(const vec4<P> &r)
{
	return vec4<P>(abs(r.x), abs(r.y), abs(r.z), abs(r.w));
}


template<typename P>
static inline vec2<P> saturate(const vec2<P> &r)
{
    return vec2<P>(saturate(r.x), saturate(r.y));
}

template<typename P>
static inline vec3<P> saturate(const vec3<P> &r)
{
    return vec3<P>(saturate(r.x), saturate(r.y), saturate(r.z));
}


template<typename P>
static inline vec4<P> saturate(const vec4<P> &r)
{
    return vec4<P>(saturate(r.x), saturate(r.y), saturate(r.z), saturate(r.w));
}


template<typename P>
static inline vec2<P> min(const vec2<P> &l, const vec2<P> &r)
{
    return vec2<P>(min(l.x, r.x), min(l.y, r.y));
}

template<typename P>
static inline vec3<P> min(const vec3<P> &l, const vec3<P> &r)
{
    return vec3<P>(min(l.x, r.x), min(l.y, r.y), min(l.z, r.z));
}

template<typename P>
static inline vec4<P> min(const vec4<P> &l, const vec4<P> &r)
{
    return vec4<P>(min(l.x, r.x), min(l.y, r.y), min(l.z, r.z), min(l.w, r.w));
}


template<typename P>
static inline vec2<P> max(const vec2<P> &l, const vec2<P> &r)
{
    return vec2<P>(max(l.x, r.x), max(l.y, r.y));
}

template<typename P>
static inline vec3<P> max(const vec3<P> &l, const vec3<P> &r)
{
    return vec3<P>(max(l.x, r.x), max(l.y, r.y), max(l.z, r.z));
}


template<typename P>
static inline vec4<P> max(const vec4<P> &l, const vec4<P> &r)
{
	return vec4<P>(max(l.x, r.x), max(l.y, r.y), max(l.z, r.z), max(l.w, r.w));
}


template<typename P>
static inline vec2<P> clamp(const vec2<P> &x, const vec2<P> &a, const vec2<P> &b)
{
    return vec2<P>(clamp(x.x, a.x, b.x), clamp(x.y, a.y, b.y));
}

template<typename P>
static inline vec2<P> clamp(const vec2<P> &x, P a, P b)
{
    return vec2<P>(clamp(x.x, a, b), clamp(x.y, a, b));
}

template<typename P>
static inline vec3<P> clamp(const vec3<P> &x, const vec3<P> &a, const vec3<P> &b)
{
    return vec3<P>(clamp(x.x, a.x, b.x), clamp(x.y, a.y, b.y), clamp(x.z, a.z, b.z));
}

template<typename P>
static inline vec3<P> clamp(const vec3<P> &x, P a, P b)
{
    return vec3<P>(clamp(x.x, a, b), clamp(x.y, a, b), clamp(x.z, a, b));
}


template<typename P>
static inline vec4<P> clamp(const vec4<P> &x, const vec4<P> &a, const vec4<P> &b)
{
	return vec4<P>(clamp(x.x, a.x, b.x), clamp(x.y, a.y, b.y), clamp(x.z, a.z, b.z), clamp(x.w, a.w, b.w));
}

template<typename P>
static inline vec4<P> clamp(const vec4<P> &x, P a, P b)
{
	return vec4<P>(clamp(x.x, a, b), clamp(x.y, a, b), clamp(x.z, a, b), clamp(x.w, a, b));
}

template <typename P> static inline vec2<P> rsqrt(const vec2<P> &r)
{
    return vec2<P>(rsqrt(r.x), rsqrt(r.y));
}
template <typename P> static inline vec3<P> rsqrt(const vec3<P> &r)
{
    return vec3<P>(rsqrt(r.x), rsqrt(r.y), rsqrt(r.z));
}

template <typename P>
static inline vec4<P> rsqrt(const vec4<P> &r)
{
    return vec4<P>(rsqrt(r.x), rsqrt(r.y), rsqrt(r.z), rsqrt(r.w));
}

template <typename P> 
static inline vec2<P> sqrt(const vec2<P> &r)
{
    return vec2<P>((P) sqrt(r.x), (P) sqrt(r.y));
}

template <typename P> 
static inline vec3<P> sqrt(const vec3<P> &r)
{
    return vec3<P>((P) sqrt(r.x), (P) sqrt(r.y), (P) sqrt(r.z));
}

template <typename P>
static inline vec4<P> sqrt(const vec4<P> &r)
{
    return vec4<P>((typename P::scalar) sqrt(r.x), (typename P::scalar) sqrt(r.y), (typename P::scalar) sqrt(r.z), (typename P::scalar) sqrt(r.w));
}


template <typename P>
static inline vec4<P> sincos(P x)
{
    P s, c; sincos(x, s, c);

    return vec4<P>(s, s, s, c);
}

template<typename X, typename S, typename C>
static inline void sincos(const rhs2<X> &x, const lhs2<S> &s, const lhs2<C> &c)
{
    sincos(x.x, s.x, c.x);
    sincos(x.y, s.y, c.y);
}
template<typename X, typename S, typename C>
static inline void sincos(const rhs3<X> &x, const lhs3<S> &s, const lhs3<C> &c)
{
    sincos(x.x, s.x, c.x);
    sincos(x.y, s.y, c.y);
    sincos(x.z, s.z, c.z);
}

template<typename X, typename S, typename C>
static inline void sincos(const rhs4<X> &x, const lhs4<S> &s, const lhs4<C> &c)
{
	sincos(x.x, s.x, c.x);
	sincos(x.y, s.y, c.y);
	sincos(x.z, s.z, c.z);
	sincos(x.w, s.w, c.w);
}


template <typename P>
static inline vec2<P> acos(const vec2<P> &r)
{
    return vec2<P>(acos(r.x), acos(r.y));
}

template <typename P>
static inline vec3<P> acos(const vec3<P> &r)
{
    return vec3<P>(acos(r.x), acos(r.y), acos(r.z));
}

template <typename P>
static inline vec4<P> acos(const vec4<P> &r)
{
   return vec4<P>(acos((typename P::scalar) r.x), acos((typename P::scalar) r.y), acos((typename P::scalar) r.z), acos((typename P::scalar) r.w));
}

//mat2 
template <typename P>
static inline float determinant(const mat2<P>& m)
{
    return m[0].x*m[1].y - m[0].y*m[1].x;
}

template <typename P>
static inline mat2<P> transpose(const mat2<P>& m)
{
    return mat2<P>(vec2<P>(m[0].x, m[1].x), vec2<P>(m[0].y, m[1].y));
}

template <typename P>
static inline mat2<P> inverse(const mat2<P>& m)
{
    return mat2<P>(vec2<P>(m[1].y, -m[0].y), vec2<P>(-m[1].x, m[0].x)) / determinant(m);
}

template <typename P>
static inline vec2<P> mul(const mat2<P>& m, const vec2<P>& v)
{

	return m[0]*v.x + m[1]*v.y;
}

template <typename P>
static inline vec2<P> mul(const vec2<P>& v, const mat2<P>& m)
{
    return vec2<P>(dot(v, m[0]), dot(v, m[1]));
}

template <typename P>
static inline mat2<P> mul(const mat2<P>& m, const mat2<P>& n)
{
    return mat2<P>(m[0]*n[0].x + m[1]*n[0].y, m[0]*n[1].x + m[1]*n[1].y);
}

template <typename P>
static inline mat2<P> abs(const mat2<P>& m)
{
    return mat2<P>(abs(m[0]), abs(m[1]));
}

// mat3
template <typename P>
static inline P determinant(const mat3<P>& m)
{
    return 
        m[0].x*(m[1].y*m[2].z - m[1].z*m[2].y) +
        m[1].x*(m[0].z*m[2].y - m[0].y*m[2].z) +
        m[2].x*(m[0].y*m[1].z - m[0].z*m[1].y);
}

template <typename P>
static inline mat3<P> inverse(const mat3<P>& m)
{
    const vec3<P> v = vec3<P>(m[1].y*m[2].z - m[1].z*m[2].y, m[0].z*m[2].y - m[0].y*m[2].z, m[0].y*m[1].z - m[0].z*m[1].y);
    const P rdet = 1.f / (m[0].x*v.x + m[1].x*v.y + m[2].x*v.z);

    return mat3<P>(v,
        vec3<P>(m[1].z*m[2].x - m[1].x*m[2].z, m[0].x*m[2].z - m[0].z*m[2].x, m[0].z*m[1].x - m[0].x*m[1].z),
        vec3<P>(m[1].x*m[2].y - m[1].y*m[2].x, m[0].y*m[2].x - m[0].x*m[2].y, m[0].x*m[1].y - m[0].y*m[1].x))*rdet;
}


template <typename P>
static inline mat3<P> transpose(const mat3<P>& m)
{
    return mat3<P>(
        vec3<P>(m[0].x, m[1].x, m[2].x), 
        vec3<P>(m[0].y, m[1].y, m[2].y), 
        vec3<P>(m[0].z, m[1].z, m[2].z));
}

template <typename P>
static inline vec3<P> mul(const mat3<P>& m, const vec3<P>& v)
{
    return vec3<P>(m[0]*v.x + m[1]*v.y + m[2]*v.z);
}

template <typename P>
static inline vec3<P> mul(const vec3<P>& v, const mat3<P>& m)
{
    return vec3<P>(dot(v, m[0]), dot(v, m[1]), dot(v, m[2]));
}

template <typename P>
static inline mat3<P> mul(const mat3<P>& m, const mat3<P>& n)
{

    return mat3<P>(
        m[0]*n[0].x + m[1]*n[0].y + m[2]*n[0].z,
        m[0]*n[1].x + m[1]*n[1].y + m[2]*n[1].z,
        m[0]*n[2].x + m[1]*n[2].y + m[2]*n[2].z
    );
}

template <typename P>
static inline mat3<P> abs(const mat3<P>& m)
{
    return mat3<P>(abs(m[0]), abs(m[1]), abs(m[2]));
}

// mat4
template <typename P>
static inline P determinant(const mat4<P>& m)
{
    const vec4<P> la = m[2].zxxz*m[3].wwyy - m[2].wwyy*m[3].zxxz;
    const vec4<P> lb = m[2].wzyx*m[3].yxwz - m[2].yxwz*m[3].wzyx;

    return dot(m[0], m[1].yzwx*la + m[1].zwxy*lb - m[1].wxyz*la.wxyz);
}

// inverse transpose
template <typename P>
static inline mat4<P> inverse_t(const mat4<P>& m)
{
    mat4<P> madj;
    {
        const vec4<P> a = m[3].zxxz*m[2].wwyy - m[3].wwyy*m[2].zxxz;
        const vec4<P> c = a.wxyz;
        const vec4<P> b = (m[3]*m[2].zwxy - m[3].zwxy*m[2]).yxwz;

        madj[0] = (m[1]*a).wxyz - (m[1]*b).zwxy - (m[1]*c).yzwx;
        madj[1] = (m[0]*c).yzwx + (m[0]*b).zwxy - (m[0]*a).wxyz;
    }
    {
        const vec4<P> a = m[1].zxxz*m[0].wwyy - m[1].wwyy*m[0].zxxz;
        const vec4<P> c = a.wxyz;
        const vec4<P> b = (m[1]*m[0].zwxy - m[1].zwxy*m[0]).yxwz;

        madj[2] = (m[3]*a).wxyz - (m[3]*b).zwxy - (m[3]*c).yzwx;
        madj[3] = (m[2]*c).yzwx + (m[2]*b).zwxy - (m[2]*a).wxyz;
    }
    return madj / dot(m[3], madj[3]);
}

template <typename P>
static inline mat4<P> inverse(const mat4<P>& m)
{
    return inverse_t(transpose(m));
}

template <typename P>
static inline mat4<P> transpose(const mat4<P>& m)
{
	return mat4<P>(
        vec4<P>(m[0].x, m[1].x, m[2].x, m[3].x),
        vec4<P>(m[0].y, m[1].y, m[2].y, m[3].y),
        vec4<P>(m[0].z, m[1].z, m[2].z, m[3].z),
        vec4<P>(m[0].w, m[1].w, m[2].w, m[3].w)
    );

}

template <typename P>
static inline void transpose(vec4<P>& d0, vec4<P>& d1, vec4<P>& d2, vec4<P>& d3, const vec4<P>& s0, const vec4<P>& s1, const vec4<P>& s2, const vec4<P>& s3)
{

    d0 = vec4<P>(s0.x, s1.x, s2.x, s3.x);
    d1 = vec4<P>(s0.y, s1.y, s2.y, s3.y);
    d2 = vec4<P>(s0.z, s1.z, s2.z, s3.z);
    d3 = vec4<P>(s0.w, s1.w, s2.w, s3.w);

}

template <typename P>
static inline vec4<P> mul(const mat4<P>& m, const vec4<P>& v)
{
    return vec4<P>(m[0]*v.x + m[1]*v.y + m[2]*v.z + m[3]*v.w);
}

template <typename P>
static inline vec4<P> mul(const vec4<P>& v, const mat4<P>& m)
{
    return vec4<P>(dot(v, m[0]), dot(v, m[1]), dot(v, m[2]), dot(v, m[3]));
}



template <typename P>
static inline mat4<P> mul(const mat4<P>& m, const mat4<P>& n)
{
	return mat4<P>(
        m[0]*n[0].x + m[1]*n[0].y + m[2]*n[0].z + m[3]*n[0].w,
        m[0]*n[1].x + m[1]*n[1].y + m[2]*n[1].z + m[3]*n[1].w,
        m[0]*n[2].x + m[1]*n[2].y + m[2]*n[2].z + m[3]*n[2].w,
        m[0]*n[3].x + m[1]*n[3].y + m[2]*n[3].z + m[3]*n[3].w
    );
}

template <typename P>
static inline mat4<P> abs(const mat4<P>& m)
{
    return mat4<P>(abs(m[0]), abs(m[1]), abs(m[2]), abs(m[3]));
}




} //namespace GS

#endif 