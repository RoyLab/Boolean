#ifndef _MAT3_H_
#define _MAT3_H_
#include "vec3.h"
namespace GS {

template <typename P = float>
struct mat3 {

    typedef typename vec3<P>::scalar    scalar;

    vec3<P>    v[3];

    // default
    inline mat3()
        {}

    inline mat3(const mat3<P> &rhs)
        { v[0] = rhs.v[0]; v[1] = rhs.v[1]; v[2] = rhs.v[2]; }
    inline mat3(const vec3<P> &v0, const vec3<P> &v1, const vec3<P> &v2)
        { v[0] = v0; v[1] = v1; v[2] = v2; }
    inline mat3(scalar rhs)
        { v[0] = rhs; v[1] = rhs; v[2] = rhs; }

    inline const vec3<P> &operator[](int i) const
        { return v[i];  }
    inline vec3<P> &operator[](int i)
        { return v[i];  }

    inline operator const scalar *() const
        { return (const scalar *) &v[0]; }
    inline operator scalar *()
        { return (scalar *) &v[0]; }

    inline mat3 &operator=(const mat3<P> &m)
        { v[0] = m.v[0]; v[1] = m.v[1]; v[2] = m.v[2]; return *this; }

    inline mat3 &operator+=(const mat3<P> &m)
        { v[0] += m.v[0]; v[1] += m.v[1]; v[2] += m.v[2]; return *this; }
    inline mat3 &operator-=(const mat3<P> &m)
        { v[0] -= m.v[0]; v[1] -= m.v[1]; v[2] -= m.v[2]; return *this; }
    inline mat3 &operator*=(const mat3<P> &m)
        { v[0] *= m.v[0]; v[1] *= m.v[1]; v[2] *= m.v[2]; return *this; }
    inline mat3 &operator/=(const mat3<P> &m)
        { v[0] /= m.v[0]; v[1] /= m.v[1]; v[2] /= m.v[2]; return *this; }
};

template <typename P> static inline mat3<P> operator+(const mat3<P> &lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1], lhs.v[2] + rhs.v[2]); }
template <typename P> static inline mat3<P> operator+(const mat3<P> &lhs, typename mat3<P>::scalar rhs)
    { return mat3<P>(lhs.v[0] + rhs, lhs.v[1] + rhs, lhs.v[2] + rhs); }
template <typename P> static inline mat3<P> operator+(typename mat3<P>::scalar lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs + rhs.v[0], lhs + rhs.v[1], lhs + rhs.v[2]); }

template <typename P> static inline mat3<P> operator-(const mat3<P> &lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1], lhs.v[2] - rhs.v[2]); }
template <typename P> static inline mat3<P> operator-(const mat3<P> &lhs, typename mat3<P>::scalar rhs)
    { return mat3<P>(lhs.v[0] - rhs, lhs.v[1] - rhs, lhs.v[2] - rhs); }
template <typename P> static inline mat3<P> operator-(typename mat3<P>::scalar lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs - rhs.v[0], lhs - rhs.v[1], lhs - rhs.v[2]); }

template <typename P> static inline mat3<P> operator*(const mat3<P> &lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs.v[0] * rhs.v[0], lhs.v[1] * rhs.v[1], lhs.v[2] * rhs.v[2]); }
template <typename P> static inline mat3<P> operator*(const mat3<P> &lhs, typename mat3<P>::scalar rhs)
    { return mat3<P>(lhs.v[0] * rhs, lhs.v[1] * rhs, lhs.v[2] * rhs); }
template <typename P> static inline mat3<P> operator*(typename mat3<P>::scalar lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs * rhs.v[0], lhs * rhs.v[1], lhs * rhs.v[2]); }

template <typename P> static inline mat3<P> operator/(const mat3<P> &lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs.v[0] / rhs.v[0], lhs.v[1] / rhs.v[1], lhs.v[2] / rhs.v[2]); }
template <typename P> static inline mat3<P> operator/(const mat3<P> &lhs, typename mat3<P>::scalar rhs)
    { return mat3<P>(lhs.v[0] / rhs, lhs.v[1] / rhs, lhs.v[2] / rhs); }
template <typename P> static inline mat3<P> operator/(typename mat3<P>::scalar lhs, const mat3<P> &rhs)
    { return mat3<P>(lhs / rhs.v[0], lhs / rhs.v[1], lhs / rhs.v[2]); }

static inline const mat3<> &id3x3()
{
    static const mat3<>::scalar id[3][3] = {
        { 1.f, 0.f, 0.f },
        { 0.f, 1.f, 0.f },
        { 0.f, 0.f, 1.f }
    };
    return *(const mat3<> *) &id;
}


}

#endif 
