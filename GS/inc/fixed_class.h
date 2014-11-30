/*
Copyright (c) 2007, Markus Trenkwalder

All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, 
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation 
  and/or other materials provided with the distribution.

* Neither the name of the library's copyright owner nor the names of its 
  contributors may be used to endorse or promote products derived from this 
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FIXEDP_CLASS_H_INCLUDED
#define FIXEDP_CLASS_H_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif

#pragma warning(disable: 4146)
#pragma comment(lib, "libgmp-10.lib")
//#define NDEBUG

#include <gmp.h>
#include <cmath>
#include <cassert>
#include <iostream>
#include "typedefs.h"


namespace GS {

// The template argument p in all of the following functions refers to the 
// fixed point precision 

#define INT_LIMIT(p) ((1 << (sizeof(long)*8-(p)-1))-1)

template <int p>
struct fixed_point {
	mpz_t		intValue;
	int32_t		level;

#ifndef NDEBUG
	double value;
#define CALC_VALUE value=double(*this)
#else
#define CALC_VALUE
#endif

	fixed_point(): level(1)				{mpz_init(intValue); CALC_VALUE;}
	fixed_point(const fixed_point& r): level(r.level) {mpz_init_set(intValue, r.intValue);CALC_VALUE;}

	fixed_point(int32_t i): level(1)	{assert(i < INT_LIMIT(p)); mpz_init_set_si(intValue, i << p);CALC_VALUE;}
	fixed_point(double f): level(1)		{assert(f < INT_LIMIT(p)); mpz_init_set_d(intValue, f*(1L << p));CALC_VALUE;}

	~fixed_point() {mpz_clear(intValue);}

	fixed_point& operator = (const fixed_point& r)	{level = r.level;	mpz_set(intValue, r.intValue); CALC_VALUE;return *this;}
	fixed_point& operator = (double f)				{level = 1;			mpz_set_d(intValue, f*(1L << p)); CALC_VALUE;return *this;}
	fixed_point& operator = (int32_t i)				{level = 1;			mpz_set_si(intValue, i << p); CALC_VALUE;return *this;}

	fixed_point& operator += (const fixed_point&  r) { assert(level == r.level); mpz_add(intValue, intValue, r.intValue);CALC_VALUE; return *this; }
	fixed_point& operator -= (const fixed_point&  r) { assert(level == r.level); mpz_sub(intValue, intValue, r.intValue);CALC_VALUE; return *this; }
	fixed_point& operator *= (const fixed_point&  r) { mpz_mul(intValue, intValue, r.intValue); level += r.level; CALC_VALUE;return *this; }
	fixed_point& operator /= (const fixed_point&  r) { mpz_tdiv_q(intValue, intValue, r.intValue); level -= r.level; CALC_VALUE;return *this; }
	
	fixed_point& operator *= (int32_t r) { mpz_mul_si(intValue, intValue, r); CALC_VALUE;return *this; }
	fixed_point& operator /= (uint32_t r){ mpz_tdiv_q_ui(intValue, intValue, r); CALC_VALUE;return *this; }
	
	fixed_point operator - () const { fixed_point x; mpz_neg(x.intValue, intValue); x.level = level; return x; }
	fixed_point operator + (const fixed_point& r) const { fixed_point x = *this; x += r; return x;}
	fixed_point operator - (const fixed_point&  r) const { fixed_point x = *this; x -= r; return x;}
	fixed_point operator * (const fixed_point&  r) const { fixed_point x = *this; x *= r; return x;}
	fixed_point operator / (const fixed_point&  r) const { fixed_point x = *this; x /= r; return x;}
	
	bool operator == (const fixed_point&  r) const { return (mpz_cmp(intValue, r.intValue) == 0 && level == r.level); }
	bool operator != (const fixed_point&  r) const { return !(*this == r); }

	bool operator == (uint32_t r) const { return (mpz_cmp_ui(intValue, (unsigned long long(r) << p*level)) == 0); }
	bool operator != (uint32_t r) const { return !(*this == r); }

	// the operator < is not what we usually mean <.
	bool operator <  (const fixed_point&  r) const { assert(level == r.level); return (mpz_cmp(intValue, r.intValue) < 0); }
	bool operator >  (const fixed_point&  r) const { assert(level == r.level); return (mpz_cmp(intValue, r.intValue) > 0); }
	bool operator <= (const fixed_point&  r) const { return !(r > *this); }
	bool operator >= (const fixed_point&  r) const { return !(r < *this); }

	//fixed_point operator + (int32_t r) const { fixed_point x = *this; x += r; return x;}
	//fixed_point operator - (int32_t r) const { fixed_point x = *this; x -= r; return x;}
	fixed_point operator * (int32_t r) const { fixed_point x = *this; x *= r; return x;}
	fixed_point operator / (int32_t r) const { fixed_point x = *this; x /= r; return x;}

	bool isZero() const { return mpz_cmp_ui(intValue, 0u);}

	operator double() const
	{
		signed long int e; 
		auto d = mpz_get_d_2exp(&e, intValue);
		return d*pow(2.0, e-level*p);
	}

	int sign() const { return mpz_cmp_si(intValue, 0);}
};

template <int p> 
std::ostream& operator<<(std::ostream& out, const fixed_point<p>& p)
{
	return (out << double(p));
}

// Specializations for use with plain integers
template <int p>
static inline fixed_point<p> operator + (int32_t a, const fixed_point<p>& b)
{ return b + a; }

template <int p>
static inline fixed_point<p> operator - (int32_t a, const fixed_point<p>& b)
{ return -b + a; }

template <int p>
static inline fixed_point<p> operator * (int32_t a, const fixed_point<p>& b)
{ return b * a; }

////////////////////////////////////////////////////////////////////////
//////////////////////////////vector 3x3////////////////////////////////
////////////////////////////////////////////////////////////////////////

template <int p>
struct fixedvec3
{
	fixed_point<p> x, y, z;

	fixedvec3(){}
	fixedvec3(const fixed_point<p>& a, const fixed_point<p>& b, const fixed_point<p>& c):
		x(a), y(b), z(c){}
	fixedvec3(const float3& v):x(v.x), y(v.y), z(v.z){}
	fixedvec3(const double3& v):x(v.x), y(v.y), z(v.z){}

	fixedvec3& operator = (const fixedvec3& v)  { x=v.x; y=v.y; z=v.z; return *this;}

	bool operator == (const fixedvec3& v) const {return x==v.x && y==v.y && z==v.z;}
	bool operator != (const fixedvec3& v) const {return !((*this)==v);}

	fixedvec3& operator += (const fixedvec3& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
	fixedvec3& operator -= (const fixedvec3& v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
	fixedvec3& operator *= (const fixedvec3& v) { x*=v.x; y*=v.y; z*=v.z; return *this; }
	fixedvec3& operator /= (const fixedvec3& v) { x/=v.x; y/=v.y; z/=v.z; return *this; }

	fixedvec3& operator *= (int32_t  k)	{ x*=k; y*=k; z*=k; return *this; }
	fixedvec3& operator /= (uint32_t k)	{ x/=k; y/=k; z/=k; return *this; }

	fixedvec3& operator *= (const fixed_point<p>& k)	{ x*=k; y*=k; z*=k; return *this; }
	fixedvec3& operator /= (const fixed_point<p>& k)	{ x/=k; y/=k; z/=k; return *this; }

	fixedvec3 operator - () const {fixedvec3 r; r.x = -x; r.y = -y; r.z = -z; return r;}
	fixedvec3 operator + (const fixedvec3& v) const {fixedvec3 r=(*this); r += v; return r;}
	fixedvec3 operator - (const fixedvec3& v) const {fixedvec3 r=(*this); r -= v; return r;}
	fixedvec3 operator * (const fixedvec3& v) const {fixedvec3 r=(*this); r *= v; return r;}
	fixedvec3 operator / (const fixedvec3& v) const {fixedvec3 r=(*this); r /= v; return r;}

	fixedvec3 operator * (int32_t k)	const { fixedvec3 r=(*this); r *= k; return r;}
	fixedvec3 operator / (uint32_t k) const { fixedvec3 r=(*this); r /= k; return r;}

	fixedvec3 operator * (const fixed_point<p>& k) const { fixedvec3 r=(*this); r *= k; return r;}
	fixedvec3 operator / (const fixed_point<p>& k) const { fixedvec3 r=(*this); r /= k; return r;}

	bool isZero() const { return x.sign() == 0 && y.sign() == 0 && z.sign() == 0; }

	fixed_point<p>& operator[](int i)
	{
		switch (i)
		{
		case 0:	return x;	break;
		case 1:	return y;	break;
		case 2:	return z;	break;
		default:
			assert(0);	break;
		}
	}
	
	const fixed_point<p>& operator[](int i) const
	{
		switch (i)
		{
		case 0:	return x;	break;
		case 1:	return y;	break;
		case 2:	return z;	break;
		default:
			assert(0);	break;
		}
	}
};

template <int p>
fixedvec3<p> operator*(const fixed_point<p>& k, const fixedvec3<p>& v)
{return v*k;}

template <int p>
static inline fixed_point<p> dot(const fixedvec3<p>& m, const fixedvec3<p>& n)
{
	return fixed_point<p>(m.x*n.x + m.y*n.y + m.z*n.z);
}

template <int p>
static inline fixedvec3<p> cross(const fixedvec3<p>& a, const fixedvec3<p>& b)
{
	return fixedvec3<p>((a.y*b.z - a.z*b.y), (a.z * b.x - a.x * b.z), (a.x * b.y - a.y * b.x));
}

template <int p>
static inline fixedvec3<p>& operator*(int32_t k, const fixedvec3<p>& v){return v*k;}

template<int p>
static inline bool vec3_lessthan(const fixedvec3<p>& m, const fixedvec3<p>& n)
{
	if (m.x < n.x) return true;
	if (m.x == n.x)
	{
		if (m.y < n.y) return true;
		if (m.y == n.y)
		{
			if (m.z < n.z) return true;
		}
	}
	return false;
}

template <int p >
inline double3 toDouble3(const fixedvec3<p>& v)
{
	//return double3(double(v.x), double(v.y), double(v.z));
	return double3(v.x, v.y, v.z);
}

////////////////////////////////////////////////////////////////////////
//////////////////////////////matrix 3x3////////////////////////////////
////////////////////////////////////////////////////////////////////////
template <int p>
struct fixedmat3
{
	fixedvec3<p> v[3];

	fixedvec3<p>& operator[](int i){return v[i];}
	const fixedvec3<p>& operator[](int i) const {return v[i];}
};

template <int p>
static inline fixed_point<p> determinant(const fixedmat3<p>& m)
{
    return 
        m[0].x*(m[1].y*m[2].z - m[1].z*m[2].y) +
        m[1].x*(m[0].z*m[2].y - m[0].y*m[2].z) +
        m[2].x*(m[0].y*m[1].z - m[0].z*m[1].y);
}

////////////////////////////////////////////////////////////////////////
//////////////////////////////vector 4x4////////////////////////////////
////////////////////////////////////////////////////////////////////////

template <int p>
struct fixedvec4
{
	fixed_point<p> v[4];

	fixedvec4(){}
	fixedvec4(const fixed_point<p>& a, const fixed_point<p>& b,
			const fixed_point<p>& c, const fixed_point<p>& d)
			{v[0]=a; v[1]=b; v[2]=c; v[3]=d;}

	fixedvec4& operator = (const fixedvec4& r)  { v[0]=r[0]; v[1]=r[1]; v[2]=r[2]; v[3]=r[3]; return *this;}
	//fixedvec4(const float3& v):x(v.x), y(v.y), z(v.z){}
	//fixedvec4(const double3& v):x(v.x), y(v.y), z(v.z){}

	void xyz(const fixedvec3<p>& val) {v[0]=val.x; v[1]=val.y; v[2]=val.z;}
	void w(const fixed_point<p>& val) {v[3]=val;}

	fixed_point<p>& operator[](int i){return v[i];}
	const fixed_point<p>& operator[](int i) const {return v[i];}

	fixedvec4 operator * (const fixedvec4<p>& r) 
		{return fixedvec4<p>(v[0]*r[0], v[1]*r[1], v[2]*r[2], v[3]*r[3]);}
};

template <int p>
static inline fixed_point<p> dot(const fixedvec4<p>& m, const fixedvec4<p>& n)
{
	return m[0]*n[0] + m[1]*n[1] + m[2]*n[2] + m[3]*n[3];
}

////////////////////////////////////////////////////////////////////////
//////////////////////////////matrix 4x4////////////////////////////////
////////////////////////////////////////////////////////////////////////

template <int p>
struct fixedmat4
{
	fixedvec4<p> v[4];

	fixedvec4<p>& operator[](int i){return v[i];}
	const fixedvec4<p>& operator[](int i) const {return v[i];}

};

template <int p>
static inline fixed_point<p> determinant(const fixedmat4<p>& m)
{
	const fixedvec4<p> la(m[2][2]*m[3][3]-m[2][3]*m[3][2], m[2][0]*m[3][3]-m[2][3]*m[3][0],
						  m[2][0]*m[3][1]-m[2][1]*m[3][0], m[2][2]*m[3][1]-m[2][1]*m[3][2]);

	const fixedvec4<p> lb(m[2][3]*m[3][1]-m[2][1]*m[3][3], m[2][2]*m[3][0]-m[2][0]*m[3][2],
						  m[2][1]*m[3][3]-m[2][3]*m[3][1], m[2][0]*m[3][2]-m[2][2]*m[3][0]);

    //const fixedvec4<p> la = m[2].zxxz*m[3].wwyy - m[2].wwyy*m[3].zxxz;
    //const fixedvec4<p> lb = m[2].wzyx*m[3].yxwz - m[2].yxwz*m[3].wzyx;

	const fixedvec4<p> lc(m[1][1]*la[0] + m[1][2]*lb[0] - m[1][3]*la[3],
						m[1][2]*la[1] + m[1][3]*lb[1] - m[1][0]*la[0],
						m[1][3]*la[2] + m[1][0]*lb[2] - m[1][1]*la[1],
						m[1][0]*la[3] + m[1][1]*lb[3] - m[1][2]*la[2]);

    return dot(m[0], lc);
}

//fixed float vector definition
#define FIXED_PRECISION 16

typedef fixed_point<FIXED_PRECISION>	stdfixed;
typedef stdfixed									stdfixed1;
typedef fixedvec3<FIXED_PRECISION>	stdfixed3;

typedef fixedmat3<FIXED_PRECISION>		stdfixed3x3;
typedef fixedmat4<FIXED_PRECISION>		stdfixed4x4;

typedef stdfixed3						FixedPoint3D;

} // end namespace fixedpoint

#endif

