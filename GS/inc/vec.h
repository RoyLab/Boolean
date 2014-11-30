#ifndef _VEC_H_
#define _VEC_H_
namespace GS{

#define _CG_SWIZZLE(hand, sfix, T, P, name)  hand##sfix< cg_##name<T, P> > name


template<typename T, typename P> struct cg_xy {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   x;
            mutable T   y;
        };
        mutable P       p;
    };
    inline operator const scalar *() const
        { return &x; }
};
template<typename T, typename P> struct cg_yx {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   y;
            mutable T   x;
        };
        mutable P       p;
    };
};

#define CG_SWIZZLE2(sfix, T, P) \
    _CG_SWIZZLE(l, sfix##2, T, P, xy); \
    _CG_SWIZZLE(l, sfix##2, T, P, yx)

template<typename T, typename P> struct cg_xz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   x;
            T v1;
            mutable T   y;
        };
        mutable P               p;
    };
};
template<typename T, typename P> struct cg_yz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            T v0;
            mutable T   x;
            mutable T   y;
        };
        mutable P               p;
    };
    inline operator const scalar *() const
        { return &x; }
};
template<typename T, typename P> struct cg_yw {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            T v0;
            mutable T   x;
            T v3;
            mutable T   y;
        };
        mutable P               p;
    };
};
template<typename T, typename P> struct cg_zw {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            T v0;
            T v1;
            mutable T   x;
            mutable T   y;
        };
        mutable P       p;
    };
    inline operator const scalar *() const
        { return &x; }
};
template<typename T, typename P> struct cg_xyz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   x;
            mutable T   y;
            mutable T   z;
        };
        mutable P               p;
    };
    inline operator const scalar *() const
        { return &x; }
};
template<typename T, typename P> struct cg_yzx {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   z;
            mutable T   x;
            mutable T   y;
        };
        mutable P               p;
    };
};
template<typename T, typename P> struct cg_yzw {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            T v0;
            mutable T   x;
            mutable T   y;
            mutable T   z;
        };
        mutable P               p;
    };
    inline operator const scalar *() const
        { return &x; }
};
template<typename T, typename P> struct cg_zxy {
    typedef T       scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   y;
            mutable T   z;
            mutable T   x;
        };
        mutable P               p;
    };
};

#define CG_SWIZZLE3(sfix, T, P) \
    CG_SWIZZLE2(sfix, T, P); \
    _CG_SWIZZLE(l, sfix##2, T, P, yz); \
    _CG_SWIZZLE(l, sfix##2, T, P, xz); \
    _CG_SWIZZLE(l, sfix##3, T, P, xyz); \
    _CG_SWIZZLE(l, sfix##3, T, P, yzx); \
    _CG_SWIZZLE(l, sfix##3, T, P, zxy)

template<typename T, typename P> struct cg_xyzw {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   x;
            mutable T   y;
            mutable T   z;
            mutable T   w;
        };
        mutable P       p;
    };
    inline operator const scalar *() const
        { return &x; }
};
template<typename T, typename P> struct cg_xzwy {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   x;
            mutable T   w;
            mutable T   y;
            mutable T   z;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_xwyz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   x;
            mutable T   z;
            mutable T   w;
            mutable T   y;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_yxwz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   y;
            mutable T   x;
            mutable T   w;
            mutable T   z;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_yzxw {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   z;
            mutable T   x;
            mutable T   y;
            mutable T   w;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_yzwx {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   w;
            mutable T   x;
            mutable T   y;
            mutable T   z;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_ywzx {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   w;
            mutable T   x;
            mutable T   z;
            mutable T   y;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_zxxz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            union {
                mutable T   y;
                mutable T   z;
            };
            T _v1;
            public: union {
                mutable T   x;
                mutable T   w;
            };
        };
        mutable P           p;
    };
};
template<typename T, typename P> struct cg_zxyw {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   y;
            mutable T   z;
            mutable T   x;
            mutable T   w;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_zywx {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   w;
            mutable T   y;
            mutable T   x;
            mutable T   z;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_zwxy {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   z;
            mutable T   w;
            mutable T   x;
            mutable T   y;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_wxyz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   y;
            mutable T   z;
            mutable T   w;
            mutable T   x;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_wxzy {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   y;
            mutable T   w;
            mutable T   z;
            mutable T   x;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_wyxz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   z;
            mutable T   y;
            mutable T   w;
            mutable T   x;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_wzyx {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            mutable T   w;
            mutable T   z;
            mutable T   y;
            mutable T   x;
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_wwzz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            T _v0;
            T _v1;
            union {
                mutable T       z;
                mutable T       w;
            };
            union {
                mutable T       x;
                mutable T       y;
            };
        };
        mutable P       p;
    };
};
template<typename T, typename P> struct cg_wwyy {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            T _v0;
            union {
                mutable T       z;
                mutable T       w;
            };
            T _v2;
            union {
                mutable T       x;
                mutable T       y;
            };
        };
        mutable P               p;
    };
};
template<typename T, typename P> struct cg_wwwz {
    typedef T   scalar;
    typedef P   packed;
    union {
        struct {
            T _v0;
            T _v1;
            mutable T           w;
            union {
                mutable T       x;
                mutable T       y;
                mutable T       z;
            };
        };
        mutable P               p;
    };
};

#define CG_SWIZZLE4(sfix, T, P) \
    _CG_SWIZZLE(l, sfix##2, T, P, yw); \
    _CG_SWIZZLE(l, sfix##2, T, P, zw); \
    \
    CG_SWIZZLE3(sfix, T, P); \
    _CG_SWIZZLE(l, sfix##3, T, P, yzw); \
    \
    _CG_SWIZZLE(l, sfix##4, T, P, xyzw); \
    _CG_SWIZZLE(l, sfix##4, T, P, xzwy); \
    _CG_SWIZZLE(l, sfix##4, T, P, xwyz); \
    _CG_SWIZZLE(l, sfix##4, T, P, yxwz); \
    _CG_SWIZZLE(l, sfix##4, T, P, yzxw); \
    _CG_SWIZZLE(l, sfix##4, T, P, yzwx); \
    _CG_SWIZZLE(l, sfix##4, T, P, ywzx); \
    _CG_SWIZZLE(l, sfix##4, T, P, zxyw); \
    _CG_SWIZZLE(l, sfix##4, T, P, zywx); \
    _CG_SWIZZLE(l, sfix##4, T, P, zwxy); \
    _CG_SWIZZLE(l, sfix##4, T, P, wxyz); \
    _CG_SWIZZLE(l, sfix##4, T, P, wxzy); \
    _CG_SWIZZLE(l, sfix##4, T, P, wyxz); \
    _CG_SWIZZLE(l, sfix##4, T, P, wzyx); \
    _CG_SWIZZLE(r, sfix##4, T, P, zxxz); \
    _CG_SWIZZLE(r, sfix##4, T, P, wwyy); \
    _CG_SWIZZLE(r, sfix##4, T, P, wwzz); \
    _CG_SWIZZLE(r, sfix##4, T, P, wwwz)

}



#endif 