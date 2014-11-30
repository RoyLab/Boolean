#ifndef _VEC3_H_
#define _VEC3_H_
#include "vec.h"
namespace GS{


template<typename L> struct rhs3 : public L {
    inline const rhs3 &operator+() const
        { return *this; }
    inline rhs3 operator-() const
        { rhs3 r; r.x = -this->x; r.y = -this->y; r.z = -this->z; return r; }
};

template<typename L> struct lhs3 : rhs3<L> {

    template<typename R> inline const lhs3 &operator=(const rhs3<R> &r) const
        { this->x = (typename L::scalar) r.x; this->y = (typename L::scalar) r.y; this->z = (typename L::scalar) r.z; return *this; }
    inline const lhs3 &operator=(typename L::scalar r) const
        { this->x = r; this->y = r; this->z = r; return *this; }

    // prefix increment
    inline const lhs3 &operator++() const
        { ++this->x; ++this->y; ++this->z; return *this; }
    // postfix increment
    inline lhs3 operator++(int) const
        { lhs3 r; r.x = this->x++; r.y = this->y++; r.z = this->z++; return r; }

    // prefix decrement
    inline const lhs3 &operator--() const
        { --this->x; --this->y; --this->z; return *this; }
    // postfix decrement
    inline lhs3 operator--(int) const
        { lhs3 r; r.x = this->x--; r.y = this->y--; r.z = this->z++; return r; }

    template<typename R> inline const lhs3 &operator+=(const rhs3<R> &r) const
        { const typename L::scalar x = (typename L::scalar) r.x, y = (typename L::scalar) r.y, z = (typename L::scalar)r.z; this->x += x; this->y += y; this->z += z; return *this; }
    inline const lhs3 &operator+=(typename L::scalar r) const
        { this->x += r; this->y += r; this->z += r; return *this; }

    template<typename R> inline const lhs3 &operator-=(const rhs3<R> &r) const
        { const typename L::scalar x = (typename L::scalar)r.x, y = (typename L::scalar)r.y, z = (typename L::scalar)r.z; this->x -= x; this->y -= y; this->z -= z; return *this; }
    inline const lhs3 &operator-=(typename L::scalar r) const
        { this->x -= r; this->y -= r; this->z -= r; return *this; }

    template<typename R> inline const lhs3 &operator*=(const rhs3<R> &r) const
        { const typename L::scalar x =(typename L::scalar) r.x, y = (typename L::scalar) r.y, z = (typename L::scalar) r.z; this->x *= x; this->y *= y; this->z *= z; return *this; }
    inline const lhs3 &operator*=(typename L::scalar r) const
        { this->x *= r; this->y *= r; this->z *= r; return *this; }

    template<typename R> inline const lhs3 &operator/=(const rhs3<R> &r) const
        { const typename L::scalar x =(typename L::scalar) r.x, y = (typename L::scalar)r.y, z = (typename L::scalar)r.z; this->x /= x; this->y /= y; this->z /= z; return *this; }
    inline const lhs3 &operator/=(typename L::scalar r) const
        { this->x /= r; this->y /= r; this->z /= r; return *this; }
};

template<typename T, typename P> struct XYZ3S {
    typedef T   scalar;
#if !defined(__INTEL_COMPILER) && !defined(__ICL) && defined(_MSC_VER)
    inline XYZ3S() {}
#endif
    union {
        mutable P                   p;
        struct {
            mutable T               x;
            mutable T               y;
            mutable T               z;
        };
	    CG_SWIZZLE3(hs, T, P);
    };
    inline operator const scalar *() const
        { return (const scalar *) &p; }
    inline scalar &operator[](int i) 
        { return ((scalar *) &p)[i]; }
};


template <typename P = float>
struct vec3 : lhs3< XYZ3S<P, P[3]> > {
public:
    inline vec3()
        {}

    inline vec3(const vec3<P> &r)
    {this->x = (P) r.x; this->y = (P) r.y; this->z = (P) r.z; }

    template<typename R> inline vec3(const vec3<R> &r)
        {this->x = (P) r.x; this->y = (P) r.y; this->z = (P) r.z; }

    template<typename R> inline vec3(const rhs3<R> &r)
        { this->x = (P) r.x; this->y = (P) r.y; this->z = (P) r.z; }

    template<typename R> inline vec3(const rhs2<R> &r, P z)
        { this->xy = r; this->z = z; }

    template<typename R> inline vec3( P x, const rhs2<R> &r)
        { this->x = x; this->yz = r; }

    template <typename R1, typename R2, typename R3> 
    inline vec3(R1 x, R2 y, R3 z)
        { this->x = (P) x; this->y = (P) y; this->z = (P) z; }

    inline vec3(P r)
        { this->x = r; this->y = r; this->z = r; }
	
    inline vec3<P> &operator=(const vec3<P> &r)
        { this->x = r.x; this->y = r.y; this->z = r.z; return *this; }

    // be carefull to lookup members first because the source could be a self swizzle
    template<typename R> inline  vec3<P> &operator=(const rhs3<R> &r)
        { this->x = (P) r.x; this->y = (P) r.y; this->z = (P) r.z; return *this; }

    inline  vec3<P> &operator=(P r) 
        { this->x = r; this->y = r; this->z = r; return *this; }
};

template<typename L, typename R> static inline vec3<typename L::scalar> operator+(const rhs3<L> &l, const rhs3<R> &r)
    { return vec3<typename L::scalar>(l.x + r.x, l.y + r.y, l.z + r.z); }

template<typename L> static inline vec3<typename L::scalar> operator+(const rhs3<L> &l, typename L::scalar r)
    { return vec3<typename L::scalar>(l.x + r, l.y + r, l.z + r); }

template<typename R> static inline vec3<typename R::scalar> operator+(typename R::scalar l, const rhs3<R> &r)
    { return vec3<typename R::scalar>(l + r.x, l + r.y, l + r.z); }

template<typename L, typename R> static inline vec3<typename L::scalar> operator-(const rhs3<L> &l, const rhs3<R> &r)
    { return vec3<typename L::scalar>(l.x - r.x, l.y - r.y, l.z - r.z); }

template<typename L> static inline vec3<typename L::scalar> operator-(const rhs3<L> &l, typename L::scalar r)
    { return vec3<typename L::scalar>(l.x - r, l.y - r, l.z - r); }

template<typename R> static inline vec3<typename R::scalar> operator-(typename R::scalar l, const rhs3<R> &r)
    { return vec3<typename R::scalar>(l - r.x, l - r.y, l - r.z); }

template<typename L, typename R> static inline vec3<typename L::scalar> operator*(const rhs3<L> &l, const rhs3<R> &r)
    { return vec3<typename L::scalar>(l.x * r.x, l.y * r.y, l.z * r.z); }
template<typename L> static inline vec3<typename L::scalar> operator*(const rhs3<L> &l, typename L::scalar r)
    { return vec3<typename L::scalar>(l.x * r, l.y * r, l.z * r); }
template<typename R> static inline vec3<typename R::scalar> operator*(typename R::scalar l, const rhs3<R> &r)
    { return vec3<typename R::scalar>(l * r.x, l * r.y, l * r.z); }

template<typename L, typename R> static inline vec3<typename L::scalar> operator/(const rhs3<L> &l, const rhs3<R> &r)
    { return vec3<typename L::scalar>(l.x / r.x, l.y / r.y, l.z / r.z); }
template<typename L> static inline vec3<typename L::scalar> operator/(const rhs3<L> &l, typename L::scalar r)
    { return vec3<typename L::scalar>(l.x / r, l.y / r, l.z / r); }
template<typename R> static inline vec3<typename R::scalar> operator/(typename R::scalar l, const rhs3<R> &r)
    { return vec3<typename R::scalar>(l / r.x, l / r.y, l / r.z); }



}

#endif 
