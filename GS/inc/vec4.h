
#ifndef _VEC4_H_
#define _VEC4_H_
#include "vec.h"
namespace GS{

template<typename L> struct rhs4 : public L {
    // unary operators
    inline const rhs4 &operator+() const
        { return *this; }
    inline rhs4 operator-() const
        { rhs4 r; r.x = -this->x; r.y = -this->y; r.z = -this->z; r.w = -this->w; return r; }
};

template<typename L> struct lhs4 : rhs4<L> {

    template<typename R> inline const lhs4 &operator=(const rhs4<R> &r) const
        { const typename L::scalar x = r.x, y = r.y, z = r.z, w = r.w; this->x = x; this->y = y; this->z = z; this->w = w; return *this; }
    inline const lhs4 &operator=(typename L::scalar r) const
        { this->x = r; this->y = r; this->z = r; this->w = r; return *this; }

    // prefix increment
    inline const lhs4 &operator++() const
        { ++this->x; ++this->y; ++this->z; ++this->w; return *this; }
    // postfix increment
    inline lhs4 operator++(int) const
        { lhs4 r; r.x = this->x++; r.y = this->y++; r.z = this->z++; r.w = this->w++; return r; }

    // prefix decrement
    inline const lhs4 &operator--() const
        { --this->x; --this->y; --this->z; --this->w; return *this; }
    // postfix decrement
    inline lhs4 operator--(int) const
        { lhs4 r; r.x = this->x--; r.y = this->y--; r.z = this->z--; r.w = this->w--; return r; }

    template<typename R> inline const lhs4 &operator+=(const rhs4<R> &r) const
        { const typename L::scalar x = r.x, y = r.y, z = r.z, w = r.w; this->x += x; this->y += y; this->z += z; this->w += w; return *this; }
    inline const lhs4 &operator+=(typename L::scalar r) const
        { this->x += r; this->y += r; this->z += r; this->w += r; return *this; }

    template<typename R> inline const lhs4 &operator-=(const rhs4<R> &r) const
        { const typename L::scalar x = r.x, y = r.y, z = r.z, w = r.w; this->x -= x; this->y -= y; this->z -= z; this->w -= w; return *this; }
    inline const lhs4 &operator-=(typename L::scalar r) const
        { this->x -= r; this->y -= r; this->z -= r; this->w -= r; return *this; }

    template<typename R> inline const lhs4 &operator*=(const rhs4<R> &r) const
        { const typename L::scalar x = r.x, y = r.y, z = r.z, w = r.w; this->x *= x; this->y *= y; this->z *= z; this->w *= w; return *this; }
    inline const lhs4 &operator*=(typename L::scalar r) const
        { this->x *= r; this->y *= r; this->z *= r; this->w *= r; return *this; }

    template<typename R> inline const lhs4 &operator/=(const rhs4<R> &r) const
        { const typename L::scalar x = r.x, y = r.y, z = r.z, w = r.w; this->x /= x; this->y /= y; this->z /= z; this->w /= w; return *this; }
    inline const lhs4 &operator/=(typename L::scalar r) const
        { this->x /= r; this->y /= r; this->z /= r; this->w /= r; return *this; }
};

template<typename T, typename P> struct XYZW4S {
    typedef T   scalar;
#if !defined(__INTEL_COMPILER) && !defined(__ICL) && defined(_MSC_VER)
    inline XYZW4S() {}
#endif
    union {
        mutable P                       p;
        struct {
            mutable T                   x;
            mutable T                   y;
            mutable T                   z;
            mutable T                   w;
        };
		CG_SWIZZLE4(hs, T, P);
    };
    inline operator const scalar *() const
        { return (const scalar *) &p; }
    inline scalar &operator[](int i) 
        { return ((scalar *) &p)[i]; }
};

template <typename P = float>
struct vec4 : public lhs4< XYZW4S<P, P[4]> > {
public:
    typedef P    scalar;
    typedef P    packed[4];

    inline vec4()
        {}

    inline vec4(const vec4<P> &r)
        { this->x = r.x; this->y = r.y; this->z = r.z; this->w = r.w; }

    template<typename R> inline vec4(const rhs4<R> &r)
        { this->x = (P)r.x; this->y = (P)r.y; this->z = (P)r.z; this->w = (P)r.w; }

    template<typename R> inline vec4(const rhs3<R> &r, const P& w)
        { this->xyz = r; this->w = w; }
    template<typename R1, typename R2> inline vec4(const rhs2<R1> &vxy, const rhs2<R2> &vzw)
        { this->xy = vxy; this->zw = vzw; }

    template <typename R1, typename R2, typename R3, typename R4>
    inline vec4(const R1 &x, const R2 &y, const R3 &z, const R4 &w)
        { this->x = (P) x; this->y = (P) y; this->z = (P) z; this->w = (P) w; }

    inline vec4(const P &r)
        { this->x = r; this->y = r; this->z = r; this->w = r; }

    inline  vec4 &operator=(const vec4<P> &r) 
        { this->x = r.x; this->y = r.y; this->z = r.z; this->w = r.w; return *this; }

    // be carefull to lookup members first because the source could be a self swizzle
    template<typename R> inline  vec4<P> &operator=(const vec4<R> &r) 
        { this->x = (P)r.x; this->y = (P)r.y; this->z = (P)r.z; this->w = (P)r.w; return *this; }

    // be carefull to lookup members first because the source could be a self swizzle
    template<typename R> inline  vec4<P> &operator=(const rhs4<R> &r) 
        { this->x = (P)r.x; this->y = (P)r.y; this->z = (P)r.z; this->w = (P)r.w; return *this; }

    inline  vec4 &operator=(const P &r) 
        { this->x = r; this->y = r; this->z = r; this->w = r; return *this; }

};

template<typename L, typename R> static inline vec4<typename L::scalar> operator+(const rhs4<L> &l, const rhs4<R> &r)
    { return vec4<typename L::scalar>(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w); }

template<typename L> static inline vec4<typename L::scalar> operator+(const rhs4<L> &l, typename L::scalar r)
    { return vec4<typename L::scalar>(l.x + r, l.y + r, l.z + r, l.w + r); }

template<typename R> static inline vec4<typename R::scalar> operator+(typename R::scalar l, const rhs4<R> &r)
    { return vec4<typename R::scalar>(l + r.x, l + r.y, l + r.z, l + r.w); }

template<typename L, typename R> static inline vec4<typename L::scalar> operator-(const rhs4<L> &l, const rhs4<R> &r)
    { return vec4<typename L::scalar>(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w); }

template<typename L> static inline vec4<typename L::scalar> operator-(const rhs4<L> &l, typename L::scalar r)
    { return vec4<typename L::scalar>(l.x - r, l.y - r, l.z - r, l.w - r); }

template<typename R> static inline vec4<typename R::scalar> operator-(typename R::scalar l, const rhs4<R> &r)
    { return vec4<typename R::scalar>(l - r.x, l - r.y, l - r.z, l - r.w); }

template<typename L, typename R> static inline vec4<typename L::scalar> operator*(const rhs4<L> &l, const rhs4<R> &r)
    { return vec4<typename L::scalar>(l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w); }
template<typename L> static inline vec4<typename L::scalar> operator*(const rhs4<L> &l, const typename L::scalar r)
    { return vec4<typename L::scalar>(l.x * r, l.y * r, l.z * r, l.w * r); }
template<typename R> static inline vec4<typename R::scalar> operator*(typename R::scalar l, const rhs4<R> &r)
    { return vec4<typename R::scalar>(l * r.x, l * r.y, l * r.z, l * r.w); }

template<typename L, typename R> static inline vec4<typename L::scalar> operator/(const rhs4<L> &l, const rhs4<R> &r)
    { return vec4<typename L::scalar>(l.x / r.x, l.y / r.y, l.z / r.z, l.w / r.w); }
template<typename L> static inline vec4<typename L::scalar> operator/(const rhs4<L> &l, typename L::scalar r)
    { return vec4<typename L::scalar>(l.x / r, l.y / r, l.z / r, l.w / r); }
template<typename R> static inline vec4<typename R::scalar> operator/(typename R::scalar l, const rhs4<R> &r)
    { return vec4<typename R::scalar>(l / r.x, l / r.y, l / r.z, l / r.w); }

template <typename Precision>
static inline vec4<Precision> load4(const Precision u[])
{
    return vec4<Precision> (u[0], u[1], u[2], u[3]);
}

template <typename Precision>
static inline void store4(Precision u[], const vec4<Precision> &v)
{
    u[0] = v.x; u[1] = v.y; u[2] = v.z; u[3] = v.w;
}
}

#endif 