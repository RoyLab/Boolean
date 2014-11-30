#ifndef _MAT2_H_
#define _MAT2_H_
#include "vec2.h"
namespace GS{
template <typename P = float>
struct mat2 {

    typedef typename vec2<P>::scalar    scalar;

    vec2<P>    v[2];

    // default
    inline mat2() {}

    // copy from other instances
    inline mat2(const mat2<P> &rhs)
        { v[0] = rhs.v[0]; v[1] = rhs.v[1]; }
    inline mat2(const vec2<P> &v0, const vec2<P> &v1)
        { v[0] = v0; v[1] = v1; }
    inline mat2(scalar rhs)
        { v[0] = rhs; v[1] = rhs; }

    /*
     * conversion operators
     */

    /*
     * subscripting operator
     */
    inline vec2<P> operator[](int i) const
        { return v[i];  }
    inline vec2<P>  &operator[](int i)
        { return v[i];  }

    inline operator const scalar *() const
        { return (const scalar *) &v[0]; }
    inline operator scalar *()
        { return (scalar *) &v[0]; }

    inline mat2<P> &operator=(const mat2<P> &m)
        { v[0] = m.v[0]; v[1] = m.v[1]; return *this; }

    inline mat2<P> &operator+=(const mat2<P> &m)
        { v[0] += m.v[0]; v[1] += m.v[1]; return *this; }
    inline mat2<P> &operator-=(const mat2<P> &m)
        { v[0] -= m.v[0]; v[1] -= m.v[1]; return *this; }
    inline mat2<P> &operator*=(const mat2<P> &m)
        { v[0] *= m.v[0]; v[1] *= m.v[1]; return *this; }
    inline mat2<P> &operator/=(const mat2<P> &m)
        { v[0] /= m.v[0]; v[1] /= m.v[1]; return *this; }
};

template <typename P> static inline mat2<P> operator+(const mat2<P> &lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1]); }
template <typename P> static inline mat2<P> operator+(const mat2<P> &lhs, typename mat2<P>::scalar rhs)
    { return mat2<P>(lhs.v[0] + rhs, lhs.v[1] + rhs); }
template <typename P> static inline mat2<P> operator+(typename mat2<P>::scalar lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs + rhs.v[0], lhs + rhs.v[1]); }

template <typename P> static inline mat2<P> operator-(const mat2<P> &lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1]); }
template <typename P> static inline mat2<P> operator-(const mat2<P> &lhs, typename mat2<P>::scalar rhs)
    { return mat2<P>(lhs.v[0] - rhs, lhs.v[1] - rhs); }
template <typename P> static inline mat2<P> operator-(typename mat2<P>::scalar lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs - rhs.v[0], lhs - rhs.v[1]); }

template <typename P> static inline mat2<P> operator*(const mat2<P> &lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs.v[0] * rhs.v[0], lhs.v[1] * rhs.v[1]); }
template <typename P> static inline mat2<P> operator*(const mat2<P> &lhs, typename mat2<P>::scalar rhs)
    { return mat2<P>(lhs.v[0] * rhs, lhs.v[1] * rhs); }
template <typename P> static inline mat2<P> operator*(typename mat2<P>::scalar lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs * rhs.v[0], lhs * rhs.v[1]); }

template <typename P> static inline mat2<P> operator/(const mat2<P> &lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs.v[0] / rhs.v[0], lhs.v[1] / rhs.v[1]); }
template <typename P> static inline mat2<P> operator/(const mat2<P> &lhs, typename mat2<P>::scalar rhs)
    { return mat2<P>(lhs.v[0] / rhs, lhs.v[1] / rhs); }
template <typename P> static inline mat2<P> operator/(typename mat2<P>::scalar lhs, const mat2<P> &rhs)
    { return mat2<P>(lhs / rhs.v[0], lhs / rhs.v[1]); }

template <typename P>
static inline const mat2<P> &id2x2()
{
    static const mat2<P>::scalar id[2][2] = {
        { (P)1.f, (P)0.f },
        { (P)0.f, (P)1.f }
    };
    return *(const mat2<P> *) &id;
}

}

#endif 
