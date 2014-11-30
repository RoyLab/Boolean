#ifndef _MAT4_H_
#define _MAT4_H_
#include "vec4.h"
namespace GS{

template <typename P = float>
struct mat4 {
    typedef typename vec4<P>::scalar    scalar;

    vec4<P> v[4];

    // default
    inline mat4()
        {}


    template <typename R>
    inline mat4(const mat4<R> &rhs)
        { v[0] = rhs.v[0]; v[1] = rhs.v[1]; v[2] = rhs.v[2]; v[3] = rhs.v[3]; }

    template <typename R>
    inline mat4(const rhs4<R> &v0, const rhs4<R> &v1, const rhs4<R> &v2, const rhs4<R> &v3)
        { v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3; }

    inline mat4(const P& rhs)
        { v[0] = rhs; v[1] = rhs; v[2] = rhs; v[3] = rhs; }


    /*
     * conversion operators
     */

    /*
     * subscripting operator
     */
    inline const vec4<P> &operator[](int i) const
        { return *(const vec4<P>*) &v[i];}
    inline vec4<P> &operator[](int i)
        { return *(vec4<P>*) &v[i];  }

    inline operator const scalar *() const
        { return (const scalar *) &v[0]; }
    inline operator scalar *()
        { return (scalar *) &v[0]; }


    template <typename R>
    inline mat4<P> &operator=(const mat4<R> &m)
        { v[0] = m.v[0]; v[1] = m.v[1]; v[2] = m.v[2]; v[3] = m.v[3]; return *this; }

    template <typename R>
    inline mat4<P> &operator+=(const mat4<R> &m)
        { v[0] += m.v[0]; v[1] += m.v[1]; v[2] += m.v[2]; v[3] += m.v[3]; return *this; }

    inline mat4<P> &operator+=(const P &s)
        { v[0] += s; v[1] += s; v[2] += s; v[3] += s; return *this; }

    template <typename R>
    inline mat4<P> &operator-=(const mat4<R> &m)
        { v[0] -= m.v[0]; v[1] -= m.v[1]; v[2] -= m.v[2]; v[3] -= m.v[3]; return *this; }

    inline mat4<P> &operator-=(const P &s)
        { v[0] -= s; v[1] -= s; v[2] -= s; v[3] -= s; return *this; }

    template <typename R>
    inline mat4<P> &operator*=(const mat4<R> &m)
        { v[0] *= m.v[0]; v[1] *= m.v[1]; v[2] *= m.v[2]; v[3] *= m.v[3]; return *this; }

    inline mat4<P> &operator*=(const P &s)
        { v[0] *= s; v[1] *= s; v[2] *= s; v[3] *= s; return *this; }

    template <typename R>
    inline mat4<P> &operator/=(const mat4<R> &m)
        { v[0] /= m.v[0]; v[1] /= m.v[1]; v[2] /= m.v[2]; v[3] /= m.v[3]; return *this; }

    inline mat4<P> &operator/=(const P &s)
        { const P r = 1/s; v[0] *= r; v[1] *= r; v[2] *= r; v[3] *= r; return *this; }
};

template <typename P >
static inline mat4<P> operator+(const mat4<P> &lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1], lhs.v[2] + rhs.v[2], lhs.v[3] + rhs.v[3]); }

template <typename P >
static inline mat4<P> operator+(const mat4<P> &lhs, typename mat4<P>::scalar rhs)
    { return mat4<P>(lhs.v[0] + rhs, lhs.v[1] + rhs, lhs.v[2] + rhs, lhs.v[3] + rhs); }

template <typename P >
static inline mat4<P> operator+(typename mat4<P>::scalar lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs + rhs.v[0], lhs + rhs.v[1], lhs + rhs.v[2], lhs + rhs.v[3]); }

template <typename P >
static inline mat4<P> operator-(const mat4<P> &lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1], lhs.v[2] - rhs.v[2], lhs.v[3] - rhs.v[3]); }
template <typename P >
static inline mat4<P> operator-(const mat4<P> &lhs, typename mat4<P>::scalar rhs)
    { return mat4<P>(lhs.v[0] - rhs, lhs.v[1] - rhs, lhs.v[2] - rhs, lhs.v[3] - rhs); }
template <typename P >
static inline mat4<P> operator-(typename mat4<P>::scalar lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs - rhs.v[0], lhs - rhs.v[1], lhs - rhs.v[2], lhs - rhs.v[3]); }

template <typename P >
static inline mat4<P> operator*(const mat4<P> &lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs.v[0] * rhs.v[0], lhs.v[1] * rhs.v[1], lhs.v[2] * rhs.v[2], lhs.v[3] * rhs.v[3]); }

template <typename P >
static inline mat4<P> operator*(const mat4<P> &lhs, const typename mat4<P>::scalar rhs)
    { return mat4<P>(lhs.v[0] * rhs, lhs.v[1] * rhs, lhs.v[2] * rhs, lhs.v[3] * rhs); }

template <typename P >
static inline mat4<P> operator*(typename mat4<P>::scalar lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs * rhs.v[0], lhs * rhs.v[1], lhs * rhs.v[2], lhs * rhs.v[3]); }

template <typename P >
static inline mat4<P> operator/(const mat4<P> &lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs.v[0] / rhs.v[0], lhs.v[1] / rhs.v[1], lhs.v[2] / rhs.v[2], lhs.v[3] / rhs.v[3]); }

template <typename P >
static inline mat4<P> operator/(const mat4<P> &lhs, typename mat4<P>::scalar rhs)
    { 
        const P rcp = float1(1)/rhs; return mat4<P>(lhs.v[0]*rcp, lhs.v[1]*rcp, lhs.v[2]*rcp, lhs.v[3]*rcp); 
    }

template <typename P >
static inline mat4<P> operator/(typename mat4<P>::scalar lhs, const mat4<P> &rhs)
    { return mat4<P>(lhs / rhs.v[0], lhs / rhs.v[1], lhs / rhs.v[2], lhs / rhs.v[3]); }

#define cmat4(a, b, c, d)   mat4(a, b, c, d)



template <typename Precision>
static inline mat4<Precision> load4x4(const Precision u[])
{
    return mat4<Precision>(load4(&u[0]), load4(&u[4]), load4(&u[8]), load4(&u[12]));
}

template <typename Precision>
static inline void store4x4(Precision u[], const mat4<Precision> &m)
{
    store4(&u[0], m[0]);
    store4(&u[4], m[1]);
    store4(&u[8], m[2]);
    store4(&u[12], m[3]);
}


static inline const mat4<> &id4x4()
{
    static const mat4<>::scalar id[4][4] =
    {
        { 1.f, 0.f, 0.f, 0.f },
        { 0.f, 1.f, 0.f, 0.f },
        { 0.f, 0.f, 1.f, 0.f },
        { 0.f, 0.f, 0.f, 1.f }
    };
    return *(const mat4<> *) &id;
}
}
#endif 