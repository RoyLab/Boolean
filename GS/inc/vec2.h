#ifndef _VEC2_H_
#define _VEC2_H_
#include "vec.h"
namespace GS{

template<typename L> struct rhs2 : public L {
    inline const rhs2 &operator+() const
        { return *this; }
    inline rhs2 operator-() const
        { rhs2 r; r.x = -this->x; r.y = -this->y; return r; }
};

template<typename L> struct lhs2 : rhs2<L> {

    template<typename R> inline const lhs2 &operator=(const rhs2<R> &r) const
        { const typename L::scalar x = r.x, y = r.y; this->x = x; this->y = y; return *this; }
    inline const lhs2 &operator=(typename L::scalar r) const
        { this->x = r; this->y = r; return *this; }

    // prefix increment
    inline const lhs2 &operator++() const
        { ++this->x; ++this->y; return *this; }
    // postfix increment
    inline lhs2 operator++(int) const
        { lhs2 r; r.x = this->x++; r.y = this->y++; return r; }

    // prefix decrement
    inline const lhs2 &operator--() const
        { --this->x; --this->y; return *this; }
    // postfix decrement
    inline lhs2 operator--(int) const
        { lhs2 r; r.x = this->x--; r.y = this->y--; return r; }

    template<typename R> inline const lhs2 &operator+=(const rhs2<R> &r) const
        { const typename L::scalar x = r.x, y = r.y; this->x += x; this->y += y; return *this; }
    inline const lhs2 &operator+=(typename L::scalar r) const
        { this->x += r; this->y += r; return *this; }

    template<typename R> inline const lhs2 &operator-=(const rhs2<R> &r) const
        { const typename L::scalar x = r.x, y = r.y; this->x -= x; this->y -= y; return *this; }
    inline const lhs2 &operator-=(typename L::scalar r) const
        { this->x -= r; this->y -= r; return *this; }

    template<typename R> inline const lhs2 &operator*=(const rhs2<R> &r) const
        { const typename L::scalar x = r.x, y = r.y; this->x *= x; this->y *= y; return *this; }
    inline const lhs2 &operator*=(typename L::scalar r) const
        { this->x *= r; this->y *= r; return *this; }

    template<typename R> inline const lhs2 &operator/=(const rhs2<R> &r) const
        { const typename L::scalar x = r.x, y = r.y; this->x /= x; this->y /= y; return *this; }
    inline const lhs2 &operator/=(typename L::scalar r) const
        { this->x /= r; this->y /= r; return *this; }
};

template<typename T, typename P> struct XY2S {
    typedef T       scalar;
#if !defined(__INTEL_COMPILER) && !defined(__ICL) && defined(_MSC_VER)
    inline XY2S() {}
#endif
    union {
        mutable P                   p;
        struct {
            mutable T               x;
            mutable T               y;
        };
		CG_SWIZZLE2(hs, T, P);
    };
    inline operator const scalar *() const
        { return (const scalar *) &p; }
    inline scalar &operator[](int i) 
        { return ((scalar *) &p)[i]; }
};

template <typename P = float>
struct vec2 : lhs2< XY2S<P, P[2]> > {

    inline vec2()
        {}
    inline vec2(const vec2<P> &r)
        { this->x = r.x; this->y = r.y; }

    template<typename R> inline vec2(const rhs2<R> &r)
        { this->x = r.x; this->y = r.y; }

    inline  vec2(P x, P y)
        { this->x = x; this->y = y; };

    inline  vec2(P r)
        { this->x = r; this->y = r; };

    inline vec2 &operator=(const vec2<P> &r) 
        { this->x = r.x; this->y = r.y; return *this; }

#ifndef __INTEL_COMPILER // fix ICC build break via ICC 11.0.066
    template<typename R> inline  vec2<P> &operator=(const rhs2<R> &r) 
        { this->x = (P) r.x; this->y = (P) r.y; return *this; }
#endif
    inline  vec2<P> &operator=(const P r) 
        { this->x = r; this->y = r; return *this; }

};

template<typename L, typename R> static inline vec2<typename L::scalar> operator+(const rhs2<L> &l, const rhs2<R> &r)
    { return vec2<typename L::scalar>(l.x + r.x, l.y + r.y); }

template<typename L> static inline vec2<typename L::scalar> operator+(const rhs2<L> &l, typename L::scalar r)
    { return vec2<typename L::scalar>(l.x + r, l.y + r); }

template<typename R> static inline vec2<typename R::scalar> operator+(typename R::scalar l, const rhs2<R> &r)
    { return vec2<typename R::scalar>(l + r.x, l + r.y); }

template<typename L, typename R> static inline vec2<typename L::scalar> operator-(const rhs2<L> &l, const rhs2<R> &r)
    { return vec2<typename L::scalar>(l.x - r.x, l.y - r.y); }

template<typename L> static inline vec2<typename L::scalar> operator-(const rhs2<L> &l, typename L::scalar r)
    { return vec2<typename L::scalar>(l.x - r, l.y - r); }

template<typename R> static inline vec2<typename R::scalar> operator-(typename R::scalar l, const rhs2<R> &r)
    { return vec2<typename R::scalar>(l - r.x, l - r.y); }

template<typename L, typename R> static inline vec2<typename L::scalar> operator*(const rhs2<L> &l, const rhs2<R> &r)
    { return vec2<typename L::scalar>(l.x * r.x, l.y * r.y); }

template<typename L> static inline vec2<typename L::scalar> operator*(const rhs2<L> &l, typename L::scalar r)
    { return vec2<typename L::scalar>(l.x * r, l.y * r); }

template<typename R> static inline vec2<typename R::scalar> operator*(typename R::scalar l, const rhs2<R> &r)
    { return vec2<typename R::scalar>(l * r.x, l * r.y); }

template<typename L, typename R> static inline vec2<typename L::scalar> operator/(const rhs2<L> &l, const rhs2<R> &r)
    { return vec2<typename L::scalar>(l.x / r.x, l.y / r.y); }

template<typename L> static inline vec2<typename L::scalar> operator/(const rhs2<L> &l, typename L::scalar r)
    { return vec2<typename L::scalar>(l.x / r, l.y / r); }

template<typename R> static inline vec2<typename R::scalar> operator/(typename R::scalar l, const rhs2<R> &r)
    { return vec2<typename R::scalar>(l / r.x, l / r.y); }

} 
#endif 