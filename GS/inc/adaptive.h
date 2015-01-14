#pragma once
#pragma warning(disable:4101)
#include "typedefs.h"
#include <cassert>

/* On some machines, the exact arithmetic routines might be defeated by the  */
/*   use of internal extended precision floating-point registers.  Sometimes */
/*   this problem can be fixed by defining certain values to be volatile,    */
/*   thus forcing them to be stored to memory and rounded off.  This isn't   */
/*   a great solution, though, as it slows the arithmetic down.              */
/*                                                                           */
/* To try this out, write "#define INEXACT volatile" below.  Normally,       */
/*   however, INEXACT should be defined to be nothing.  ("#define INEXACT".) */

#define INEXACT                          /* Nothing */
/* #define INEXACT volatile */

#define REAL double                      /* float or double */

/* Which of the following two methods of finding the absolute values is      */
/*   fastest is compiler-dependent.  A few compilers can inline and optimize */
/*   the fabs() call; but most will incur the overhead of a function call,   */
/*   which is disastrously slow.  A faster way on IEEE machines might be to  */
/*   mask the appropriate bit, but that's difficult to do in C.              */

#define Absolute(a)  ((a) >= 0.0 ? (a) : -(a))
/* #define Absolute(a)  fabs(a) */

/* Many of the operations are broken up into two pieces, a main part that    */
/*   performs an approximate operation, and a "tail" that computes the       */
/*   roundoff error of that operation.                                       */
/*                                                                           */
/* The operations Fast_Two_Sum(), Fast_Two_Diff(), Two_Sum(), Two_Diff(),    */
/*   Split(), and Two_Product() are all implemented as described in the      */
/*   reference.  Each of these macros requires certain variables to be       */
/*   defined in the calling routine.  The variables `bvirt', `c', `abig',    */
/*   `_i', `_j', `_k', `_l', `_m', and `_n' are declared `INEXACT' because   */
/*   they store the result of an operation that may incur roundoff error.    */
/*   The input parameter `x' (or the highest numbered `x_' parameter) must   */
/*   also be declared `INEXACT'.                                             */

#define Fast_Two_Sum_Tail(a, b, x, y) \
  bvirt = x - a; \
  y = b - bvirt

#define Fast_Two_Sum(a, b, x, y) \
  x = (REAL) (a + b); \
  Fast_Two_Sum_Tail(a, b, x, y)

#define Fast_Two_Diff_Tail(a, b, x, y) \
  bvirt = a - x; \
  y = bvirt - b

#define Fast_Two_Diff(a, b, x, y) \
  x = (REAL) (a - b); \
  Fast_Two_Diff_Tail(a, b, x, y)

#define Two_Sum_Tail(a, b, x, y) \
  bvirt = (REAL) (x - a); \
  avirt = x - bvirt; \
  bround = b - bvirt; \
  around = a - avirt; \
  y = around + bround

#define Two_Sum(a, b, x, y) \
  x = (REAL) (a + b); \
  Two_Sum_Tail(a, b, x, y)

#define Two_Diff_Tail(a, b, x, y) \
  bvirt = (REAL) (a - x); \
  avirt = x + bvirt; \
  bround = bvirt - b; \
  around = a - avirt; \
  y = around + bround

#define Two_Diff(a, b, x, y) \
  x = (REAL) (a - b); \
  Two_Diff_Tail(a, b, x, y)

#define Split(a, ahi, alo) \
  c = (REAL) (splitter * a); \
  abig = (REAL) (c - a); \
  ahi = c - abig; \
  alo = a - ahi

#define Two_Product_Tail(a, b, x, y) \
  Split(a, ahi, alo); \
  Split(b, bhi, blo); \
  err1 = x - (ahi * bhi); \
  err2 = err1 - (alo * bhi); \
  err3 = err2 - (ahi * blo); \
  y = (alo * blo) - err3

#define Two_Product(a, b, x, y) \
  x = (REAL) (a * b); \
  Two_Product_Tail(a, b, x, y)

/* Two_Product_Presplit() is Two_Product() where one of the inputs has       */
/*   already been split.  Avoids redundant splitting.                        */

#define Two_Product_Presplit(a, b, bhi, blo, x, y) \
  x = (REAL) (a * b); \
  Split(a, ahi, alo); \
  err1 = x - (ahi * bhi); \
  err2 = err1 - (alo * bhi); \
  err3 = err2 - (ahi * blo); \
  y = (alo * blo) - err3

/* Two_Product_2Presplit() is Two_Product() where both of the inputs have    */
/*   already been split.  Avoids redundant splitting.                        */

#define Two_Product_2Presplit(a, ahi, alo, b, bhi, blo, x, y) \
  x = (REAL) (a * b); \
  err1 = x - (ahi * bhi); \
  err2 = err1 - (alo * bhi); \
  err3 = err2 - (ahi * blo); \
  y = (alo * blo) - err3

/* Square() can be done more quickly than Two_Product().                     */

#define Square_Tail(a, x, y) \
  Split(a, ahi, alo); \
  err1 = x - (ahi * ahi); \
  err3 = err1 - ((ahi + ahi) * alo); \
  y = (alo * alo) - err3

#define Square(a, x, y) \
  x = (REAL) (a * a); \
  Square_Tail(a, x, y)

/* Macros for summing expansions of various fixed lengths.  These are all    */
/*   unrolled versions of Expansion_Sum().                                   */

#define Two_One_Sum(a1, a0, b, x2, x1, x0) \
  Two_Sum(a0, b , _i, x0); \
  Two_Sum(a1, _i, x2, x1)

#define Two_One_Diff(a1, a0, b, x2, x1, x0) \
  Two_Diff(a0, b , _i, x0); \
  Two_Sum( a1, _i, x2, x1)

#define Two_Two_Sum(a1, a0, b1, b0, x3, x2, x1, x0) \
  Two_One_Sum(a1, a0, b0, _j, _0, x0); \
  Two_One_Sum(_j, _0, b1, x3, x2, x1)

#define Two_Two_Diff(a1, a0, b1, b0, x3, x2, x1, x0) \
  Two_One_Diff(a1, a0, b0, _j, _0, x0); \
  Two_One_Diff(_j, _0, b1, x3, x2, x1)

#define Four_One_Sum(a3, a2, a1, a0, b, x4, x3, x2, x1, x0) \
  Two_One_Sum(a1, a0, b , _j, x1, x0); \
  Two_One_Sum(a3, a2, _j, x4, x3, x2)

#define Four_Two_Sum(a3, a2, a1, a0, b1, b0, x5, x4, x3, x2, x1, x0) \
  Four_One_Sum(a3, a2, a1, a0, b0, _k, _2, _1, _0, x0); \
  Four_One_Sum(_k, _2, _1, _0, b1, x5, x4, x3, x2, x1)

#define Four_Four_Sum(a3, a2, a1, a0, b4, b3, b1, b0, x7, x6, x5, x4, x3, x2, \
                      x1, x0) \
  Four_Two_Sum(a3, a2, a1, a0, b1, b0, _l, _2, _1, _0, x1, x0); \
  Four_Two_Sum(_l, _2, _1, _0, b4, b3, x7, x6, x5, x4, x3, x2)

#define Eight_One_Sum(a7, a6, a5, a4, a3, a2, a1, a0, b, x8, x7, x6, x5, x4, \
                      x3, x2, x1, x0) \
  Four_One_Sum(a3, a2, a1, a0, b , _j, x3, x2, x1, x0); \
  Four_One_Sum(a7, a6, a5, a4, _j, x8, x7, x6, x5, x4)

#define Eight_Two_Sum(a7, a6, a5, a4, a3, a2, a1, a0, b1, b0, x9, x8, x7, \
                      x6, x5, x4, x3, x2, x1, x0) \
  Eight_One_Sum(a7, a6, a5, a4, a3, a2, a1, a0, b0, _k, _6, _5, _4, _3, _2, \
                _1, _0, x0); \
  Eight_One_Sum(_k, _6, _5, _4, _3, _2, _1, _0, b1, x9, x8, x7, x6, x5, x4, \
                x3, x2, x1)

#define Eight_Four_Sum(a7, a6, a5, a4, a3, a2, a1, a0, b4, b3, b1, b0, x11, \
                       x10, x9, x8, x7, x6, x5, x4, x3, x2, x1, x0) \
  Eight_Two_Sum(a7, a6, a5, a4, a3, a2, a1, a0, b1, b0, _l, _6, _5, _4, _3, \
                _2, _1, _0, x1, x0); \
  Eight_Two_Sum(_l, _6, _5, _4, _3, _2, _1, _0, b4, b3, x11, x10, x9, x8, \
                x7, x6, x5, x4, x3, x2)

/* Macros for multiplying expansions of various fixed lengths.               */

#define Two_One_Product(a1, a0, b, x3, x2, x1, x0) \
  Split(b, bhi, blo); \
  Two_Product_Presplit(a0, b, bhi, blo, _i, x0); \
  Two_Product_Presplit(a1, b, bhi, blo, _j, _0); \
  Two_Sum(_i, _0, _k, x1); \
  Fast_Two_Sum(_j, _k, x3, x2)

#define Four_One_Product(a3, a2, a1, a0, b, x7, x6, x5, x4, x3, x2, x1, x0) \
  Split(b, bhi, blo); \
  Two_Product_Presplit(a0, b, bhi, blo, _i, x0); \
  Two_Product_Presplit(a1, b, bhi, blo, _j, _0); \
  Two_Sum(_i, _0, _k, x1); \
  Fast_Two_Sum(_j, _k, _i, x2); \
  Two_Product_Presplit(a2, b, bhi, blo, _j, _0); \
  Two_Sum(_i, _0, _k, x3); \
  Fast_Two_Sum(_j, _k, _i, x4); \
  Two_Product_Presplit(a3, b, bhi, blo, _j, _0); \
  Two_Sum(_i, _0, _k, x5); \
  Fast_Two_Sum(_j, _k, x7, x6)

#define Two_Two_Product(a1, a0, b1, b0, x7, x6, x5, x4, x3, x2, x1, x0) \
  Split(a0, a0hi, a0lo); \
  Split(b0, bhi, blo); \
  Two_Product_2Presplit(a0, a0hi, a0lo, b0, bhi, blo, _i, x0); \
  Split(a1, a1hi, a1lo); \
  Two_Product_2Presplit(a1, a1hi, a1lo, b0, bhi, blo, _j, _0); \
  Two_Sum(_i, _0, _k, _1); \
  Fast_Two_Sum(_j, _k, _l, _2); \
  Split(b1, bhi, blo); \
  Two_Product_2Presplit(a0, a0hi, a0lo, b1, bhi, blo, _i, _0); \
  Two_Sum(_1, _0, _k, x1); \
  Two_Sum(_2, _k, _j, _1); \
  Two_Sum(_l, _j, _m, _2); \
  Two_Product_2Presplit(a1, a1hi, a1lo, b1, bhi, blo, _j, _0); \
  Two_Sum(_i, _0, _n, _0); \
  Two_Sum(_1, _0, _i, x2); \
  Two_Sum(_2, _i, _k, _1); \
  Two_Sum(_m, _k, _l, _2); \
  Two_Sum(_j, _n, _k, _0); \
  Two_Sum(_1, _0, _j, x3); \
  Two_Sum(_2, _j, _i, _1); \
  Two_Sum(_l, _i, _m, _2); \
  Two_Sum(_1, _k, _i, x4); \
  Two_Sum(_2, _i, _k, x5); \
  Two_Sum(_m, _k, x7, x6)

/* An expansion of length two can be squared more quickly than finding the   */
/*   product of two different expansions of length two, and the result is    */
/*   guaranteed to have no more than six (rather than eight) components.     */

#define Two_Square(a1, a0, x5, x4, x3, x2, x1, x0) \
  Square(a0, _j, x0); \
  _0 = a0 + a0; \
  Two_Product(a1, _0, _k, _1); \
  Two_One_Sum(_k, _1, _j, _l, _2, x1); \
  Square(a1, _j, _1); \
  Two_Two_Sum(_j, _1, _l, _2, x5, x4, x3, x2)

#define Declare_Var c, abig, ahi, alo, bhi,\
			blo, err1, err2, err3, \
			bvirt, avirt, bround, around

#define Declare_VarEX _i, _j, _k, _l, _0, _1, _2

#define Det2x2_Tail(r, i, j, p, q, d, tail)\
	Two_Product_Tail(mat[r][i], mat[r+1][j], p, _i);\
	Two_Product_Tail(mat[r+1][i], mat[r][j], q, _j);\
	Two_Diff_Tail(p, q, d, tail);\
	tail += (_i - _j)

#define Det2x2_Head(r, i, j, a0, a1, b)\
	a0 = mat[r][i] * mat[r+1][j];\
	a1 = mat[r+1][i] * mat[r][j];\
	b = a0 - a1\

#define Error_2x2(a0, a1, b, err)\
	err = (::abs(a0) + ::abs(a1) + ::abs(b))


namespace GS
{
	extern const REAL STATIC_FILTER;
	extern REAL splitter;     /* = 2^ceiling(p / 2) + 1.  Used to split floats in half. */
	extern REAL epsilon;                /* = 2^(-p).  Used to estimate roundoff errors. */
	extern REAL err3dot, err2x2, 
		err3x3A0, err3x3A1, err3x3A2, err3x3B1, err3x3B2,
		err4x4A0, err4x4A1, err4x4A2, err4x4A3, err4x4B;
	void exactinit();
	double round(double);
	/*****************************************************************************/
	/*                                                                           */
	/*  compress()   Compress an expansion.                                      */
	/*                                                                           */
	/*  See the long version of my paper for details.                            */
	/*                                                                           */
	/*  Maintains the nonoverlapping property.  If round-to-even is used (as     */
	/*  with IEEE 754), then any nonoverlapping expansion is converted to a      */
	/*  nonadjacent expansion.                                                   */
	/*                                                                           */
	/*****************************************************************************/

	inline int compress(int elen, REAL *e,	REAL *h)
	{
	  REAL Q, q;
	  INEXACT REAL Qnew;
	  int eindex, hindex;
	  INEXACT REAL bvirt;
	  REAL enow, hnow;
	  int top, bottom;

	  bottom = elen - 1;
	  Q = e[bottom];
	  for (eindex = elen - 2; eindex >= 0; eindex--) {
		enow = e[eindex];
		Fast_Two_Sum(Q, enow, Qnew, q);
		if (q != 0) {
		  h[bottom--] = Qnew;
		  Q = q;
		} else {
		  Q = Qnew;
		}
	  }
	  top = 0;
	  for (hindex = bottom + 1; hindex < elen; hindex++) {
		hnow = h[hindex];
		Fast_Two_Sum(hnow, Q, Qnew, q);
		if (q != 0) {
		  h[top++] = q;
		}
		Q = Qnew;
	  }
	  h[top] = Q;
	  return top + 1;
	}

	/*****************************************************************************/
	/*                                                                           */
	/*  estimate()   Produce a one-word estimate of an expansion's value.        */
	/*                                                                           */
	/*  See either version of my paper for details.                              */
	/*                                                                           */
	/*****************************************************************************/

	inline REAL estimate(int elen,	REAL *e)
	{
	  REAL Q;
	  int eindex;

	  Q = e[0];
	  for (eindex = 1; eindex < elen; eindex++) {
		Q += e[eindex];
	  }
	  return Q;
	}

	inline int fast_expansion_sum_zeroelim(int elen, REAL *e, int flen, REAL *f, REAL *h)  /* h cannot be e or f. */
	{
	  REAL Q;
	  INEXACT REAL Qnew;
	  INEXACT REAL hh;
	  INEXACT REAL bvirt;
	  REAL avirt, bround, around;
	  int eindex, findex, hindex;
	  REAL enow, fnow;

	  enow = e[0];
	  fnow = f[0];
	  eindex = findex = 0;
	  if ((fnow > enow) == (fnow > -enow)) {
		Q = enow;
		enow = e[++eindex];
	  } else {
		Q = fnow;
		fnow = f[++findex];
	  }
	  hindex = 0;
	  if ((eindex < elen) && (findex < flen)) {
		if ((fnow > enow) == (fnow > -enow)) {
		  Fast_Two_Sum(enow, Q, Qnew, hh);
		  enow = e[++eindex];
		} else {
		  Fast_Two_Sum(fnow, Q, Qnew, hh);
		  fnow = f[++findex];
		}
		Q = Qnew;
		if (hh != 0.0) {
		  h[hindex++] = hh;
		}
		while ((eindex < elen) && (findex < flen)) {
		  if ((fnow > enow) == (fnow > -enow)) {
			Two_Sum(Q, enow, Qnew, hh);
			enow = e[++eindex];
		  } else {
			Two_Sum(Q, fnow, Qnew, hh);
			fnow = f[++findex];
		  }
		  Q = Qnew;
		  if (hh != 0.0) {
			h[hindex++] = hh;
		  }
		}
	  }
	  while (eindex < elen) {
		Two_Sum(Q, enow, Qnew, hh);
		enow = e[++eindex];
		Q = Qnew;
		if (hh != 0.0) {
		  h[hindex++] = hh;
		}
	  }
	  while (findex < flen) {
		Two_Sum(Q, fnow, Qnew, hh);
		fnow = f[++findex];
		Q = Qnew;
		if (hh != 0.0) {
		  h[hindex++] = hh;
		}
	  }
	  if ((Q != 0.0) || (hindex == 0)) {
		h[hindex++] = Q;
	  }
	  return hindex;
	}

/*****************************************************************************/
/*                                                                           */
/*  scale_expansion_zeroelim()   Multiply an expansion by a scalar,          */
/*                               eliminating zero components from the        */
/*                               output expansion.                           */
/*                                                                           */
/*  Sets h = be.  See either version of my paper for details.                */
/*                                                                           */
/*  Maintains the nonoverlapping property.  If round-to-even is used (as     */
/*  with IEEE 754), maintains the strongly nonoverlapping and nonadjacent    */
/*  properties as well.  (That is, if e has one of these properties, so      */
/*  will h.)                                                                 */
/*                                                                           */
/*****************************************************************************/

	inline int scale_expansion_zeroelim(int elen, REAL *e, REAL b, REAL *h)
	{
	  INEXACT REAL Q, sum;
	  REAL hh;
	  INEXACT REAL product1;
	  REAL product0;
	  int eindex, hindex;
	  REAL enow;
	  INEXACT REAL bvirt;
	  REAL avirt, bround, around;
	  INEXACT REAL c;
	  INEXACT REAL abig;
	  REAL ahi, alo, bhi, blo;
	  REAL err1, err2, err3;

	  Split(b, bhi, blo);
	  Two_Product_Presplit(e[0], b, bhi, blo, Q, hh);
	  hindex = 0;
	  if (hh != 0) {
		h[hindex++] = hh;
	  }
	  for (eindex = 1; eindex < elen; eindex++) {
		enow = e[eindex];
		Two_Product_Presplit(enow, b, bhi, blo, product1, product0);
		Two_Sum(Q, product0, sum, hh);
		if (hh != 0) {
		  h[hindex++] = hh;
		}
		Fast_Two_Sum(product1, sum, Q, hh);
		if (hh != 0) {
		  h[hindex++] = hh;
		}
	  }
	  if ((Q != 0.0) || (hindex == 0)) {
		h[hindex++] = Q;
	  }
	  return hindex;
	}

	inline double static_filter(double x)
	{
		//assert((x < -1.0) == (x > 1.0));
		return round(x * STATIC_FILTER) / STATIC_FILTER;
	}

	inline double3 static_filter(const double3& v)
	{
		//assert((v.x < -1.0) == (v.x > 1.0));
		//assert((v.y < -1.0) == (v.y > 1.0));
		//assert((v.z < -1.0) == (v.z > 1.0));

		return double3(static_filter(v.x), static_filter(v.y), static_filter(v.z));
	}

	inline double inexactDet3x3(const double3x3& m)
	{
		// non-robust
		return 
			m[0].x*(m[1].y*m[2].z - m[1].z*m[2].y) +
			m[1].x*(m[0].z*m[2].y - m[0].y*m[2].z) +
			m[2].x*(m[0].y*m[1].z - m[0].z*m[1].y);
	}

	inline int exactDot3Sign(const double3&v1, const double3& v2, double* res)
	{
		REAL Declare_Var, Declare_VarEX, x[2], y[2], z[2], k[4], m[6];
		Two_Product(v1.x, v2.x, x[1], x[0]);
		Two_Product(v1.y, v2.y, y[1], y[0]);
		Two_Product(v1.z, v2.z, z[1], z[0]);

		Two_Two_Sum(x[1], x[0], y[1], y[0], k[3], k[2], k[1], k[0]);
		Four_Two_Sum(k[3], k[2], k[1], k[0], z[1], z[0], 
			m[5], m[4], m[3], m[2], m[1], m[0]);

        return compress(6, m, res);
	}


    inline double adaptivePointClassify(const double3& normal, double distance, const double3& point)
    {
		REAL Declare_Var, Declare_VarEX, xe, ye, ze, x[2], y[2], z[2], k[4], m[6];
		Two_Product(normal.x, point.x, x[1], x[0]);
		Two_Product(normal.y, point.y, y[1], y[0]);
		Two_Product(normal.z, point.z, z[1], z[0]);

		Two_Two_Sum(x[1], x[0], y[1], y[0], k[3], k[2], k[1], k[0]);
		Four_Two_Sum(k[3], k[2], k[1], k[0], z[1], z[0], 
			m[5], m[4], m[3], m[2], m[1], m[0]);

        REAL d[6], res[6];
        int l1 =  compress(6, m, d);
        int l2 = fast_expansion_sum_zeroelim(l1, d, 1, &distance, res);
        return res[l2-1];
    }


	inline double adaptiveDot3Sign(const double3&v1, const double3& v2)
	{
		REAL x, y, z, det;
		x = v1.x * v2.x;
		y = v1.y * v2.y;
		z = v1.z * v2.z;
		
		REAL eb = 0.0;
		if ((x >= 0.0 && y >= 0.0 && z >= 0.0)||
			(x <= 0.0 && y <= 0.0 && z <= 0.0))
		{
			det = x + y + z;
			//REAL det2 = exactDot3Sign(v1, v2);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		if (x >= 0.0)
		{
			if (y >= 0.0)
			{
				if (z >= 0.0)
				{
					det = x + y + z;
					//REAL det2 = exactDot3Sign(v1, v2);
					//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
					return det;
				}
				eb = x + y - z;
			}
			else eb = x - y + ::abs(z);
		}
		else
		{
			if (y <= 0.0)
			{
				if (z <= 0.0)
				{
					det = - (x + y + z);
					//REAL det2 = exactDot3Sign(v1, v2);
					//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
					return det;
				}
				eb = - x - y + z;
			}
			else eb = - x + y + ::abs(z);
		}

		eb += (det = x + y);
		eb += (det += z);
		eb *= err3dot;
		if ((det < -eb) != (det > eb))
		{
			//REAL det2 = exactDot3Sign(v1, v2);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		REAL Declare_Var, xe, ye, ze;
		Two_Product_Tail(v1.x, v2.x, x, xe);
		Two_Product_Tail(v1.y, v2.y, y, ye);
		Two_Product_Tail(v1.z, v2.z, z, ze);

		REAL det1 = xe + ye + ze;
		REAL a1, a2, a3, a4;
		Two_Sum(x, y, a1, a2);
		Two_Sum(z, det1, a3, a4);

		REAL Declare_VarEX, b[4];
		Two_Two_Sum(a1, a2, a3, a4, b[3], b[2], b[1], b[0]);
		det = estimate(4, b);

		{
			//REAL det2 = exactDot3Sign(v1, v2);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}
	}

	inline double exactDet2x2Sign(double m00, double m01, double m10, double m11)
	{
		REAL Declare_Var, Declare_VarEX, xe, ye, ze, x[2], y[2], z[2], k[4];
		Two_Product(m00, m11, x[1], x[0]);
		Two_Product(m10, m01, y[1], y[0]);

		Two_Two_Diff(x[1], x[0], y[1], y[0], k[3], k[2], k[1], k[0]);

		return estimate(4, k);
	}

	inline double adaptiveDet2x2Sign(double m00, double m01, double m10, double m11)
	{
		REAL x1 = m00 * m11;
		REAL x2 = m10 * m01;

		REAL sumBound;
		REAL det = x1 - x2;
		if (x1 >= 0.0)
		{
			if (x2 <= 0.0) 
			{
				//REAL det2 = exactDet2x2Sign(m00, m01, m10, m11);
				//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
				return det;
			}
			else
				sumBound = x1 + x2;
		}
		else
		{
			if (x2 >= 0.0)
			{
				//REAL det2 = exactDet2x2Sign(m00, m01, m10, m11);
				//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
				return det;
			}
			else
				sumBound = -(x1 + x2);
		}

		REAL eb = sumBound * err2x2;
		if ((det < -eb) != (det > eb))
		{
			//REAL det2 = exactDet2x2Sign(m00, m01, m10, m11);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}		
		REAL Declare_Var, y1, y2, det1, res;

		Two_Diff_Tail(x1, x2, det, det1);

		Two_Product_Tail(m00, m11, x1, y1);
		Two_Product_Tail(m10, m01, x2, y2);

		det1 += (y1 - y2);
		
		det = det + det1;
		{
			//REAL det2 = exactDet2x2Sign(m00, m01, m10, m11);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}
	}

	inline double exactDet2x2Sign(const double2x2 &mat)
	{
		return exactDet2x2Sign(
			mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
	}

	inline double adaptiveDet2x2Sign(const double2x2 &mat)
	{
		return adaptiveDet2x2Sign(
			mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
	}
	inline double inexactDet3x3(const double3x3& m, double& err)
	{
		// non-robust
		return 
			m[0].x*(m[1].y*m[2].z - m[1].z*m[2].y) +
			m[1].x*(m[0].z*m[2].y - m[0].y*m[2].z) +
			m[2].x*(m[0].y*m[1].z - m[0].z*m[1].y);
	}

	inline void exactDet2x2Sign(double m00, double m01, double m10, double m11, double* k)
	{
		REAL Declare_Var, Declare_VarEX, xe, ye, ze, x[2], y[2], z[2];
		Two_Product(m00, m11, x[1], x[0]);
		Two_Product(m10, m01, y[1], y[0]);

		Two_Two_Diff(x[1], x[0], y[1], y[0], k[3], k[2], k[1], k[0]);
	}

	inline double exactDet3x3Sign(const double3x3& mat, int *klen = NULL, double* k = NULL)
	{
		REAL Declare_Var, Declare_VarEX, det2x2[12], det2x3[24];
		exactDet2x2Sign(mat[1][1], mat[1][2], mat[2][1], mat[2][2], det2x2);
		exactDet2x2Sign(mat[1][2], mat[1][0], mat[2][2], mat[2][0], det2x2+4);
		exactDet2x2Sign(mat[1][0], mat[1][1], mat[2][0], mat[2][1], det2x2+8);

		Four_One_Product(det2x2[3], det2x2[2], det2x2[1], det2x2[0], mat[0][0],
			det2x3[7], det2x3[6], det2x3[5], det2x3[4], det2x3[3], det2x3[2], det2x3[1], det2x3[0]);
		Four_One_Product(det2x2[7], det2x2[6], det2x2[5], det2x2[4], mat[0][1],
			det2x3[15], det2x3[14], det2x3[13], det2x3[12], det2x3[11], det2x3[10], det2x3[9], det2x3[8]);
		Four_One_Product(det2x2[11], det2x2[10], det2x2[9], det2x2[8], mat[0][2],
			det2x3[23], det2x3[22], det2x3[21], det2x3[20], det2x3[19], det2x3[18], det2x3[17], det2x3[16]);

		REAL suma[16], sumb[24];
		int la = fast_expansion_sum_zeroelim(8, det2x3, 8, det2x3+8, suma);
		int lb = fast_expansion_sum_zeroelim(la, suma, 8, det2x3+16, sumb);

		if (k && klen)
		{
			memcpy(k, sumb, lb*sizeof(REAL));
			*klen = lb;
		}
		return estimate(lb, sumb);
	}

	inline double adaptiveDet3x3Sign(const double3x3& mat)
	{
		REAL v[6], w[3], w2[3];
		
		Det2x2_Head(1, 1, 2, v[0], v[1], w[0]);
		Det2x2_Head(1, 2, 0, v[2], v[3], w[1]);
		Det2x2_Head(1, 0, 1, v[4], v[5], w[2]);

		REAL e[3];
		e[0] = ::abs(w[0]) + (::abs(v[0]) + ::abs(v[1]));
		e[1] = ::abs(w[1]) + (::abs(v[2]) + ::abs(v[3]));
		e[2] = ::abs(w[2]) + (::abs(v[4]) + ::abs(v[5]));

		w2[0] = mat[0][0] * w[0];
		w2[1] = mat[0][1] * w[1];
		w2[2] = mat[0][2] * w[2];

		REAL eb0 = 0.0;
		REAL det;
		eb0 += (det = (w2[0] + w2[1]));
		eb0 += (det += w2[2]);

		REAL eb1 = (::abs(w2[0]) + ::abs(w2[1]) + ::abs(w2[2]));
		REAL eb2 = (::abs(e[0] * mat[0][0]) + ::abs(e[1] * mat[0][1]) + ::abs(e[2] * mat[0][2]));
		REAL eb = eb0 * err3x3A0 + eb1 * err3x3A1 + eb2 * err3x3A2;
		if ((det > eb) != (det < -eb))
		{
			//REAL det2 = exactDet3x3Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		REAL Declare_Var, Declare_VarEX, w2tail[3], sum[4], sum2[6];

		Two_Product_Tail(mat[0][0], w[0], w2[0], w2tail[0]);
		Two_Product_Tail(mat[0][1], w[1], w2[1], w2tail[1]);
		Two_Product_Tail(mat[0][2], w[2], w2[2], w2tail[2]);
		
		Two_Two_Sum(w2[0], w2tail[0], w2[1], w2tail[1],
			sum[3], sum[2], sum[1], sum[0]);
		Four_Two_Sum(sum[3], sum[2], sum[1], sum[0], w2[2], w2tail[2], 
			sum2[5], sum2[4], sum2[3], sum2[2], sum2[1], sum2[0]);

		estimate(6, sum2);

		eb = eb1 * err3x3B1 + eb2 * err3x3B2;
		if ((det > eb) != (det < -eb))
		{
			//REAL det2 = exactDet3x3Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		REAL wtail[3];
		Det2x2_Tail(1, 1, 2, v[0], v[1], w[0], wtail[0]);
		Det2x2_Tail(1, 2, 0, v[2], v[3], w[1], wtail[1]);
		Det2x2_Tail(1, 0, 1, v[4], v[5], w[2], wtail[2]);

		if (wtail[0] == 0.0 && wtail[1] == 0.0 && wtail[2] == 0.0)
		{
			//REAL det2 = exactDet3x3Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		REAL wtailmul[6];
		Two_Product(mat[0][0], wtail[0], wtailmul[0], wtailmul[1]);
		Two_Product(mat[0][1], wtail[1], wtailmul[2], wtailmul[3]);
		Two_Product(mat[0][2], wtail[2], wtailmul[4], wtailmul[5]);

		REAL sum3[6];
		int l3 = compress(6, sum2, sum3);

		REAL sum4[8], sum5[10], sum6[12];
		int l4 = fast_expansion_sum_zeroelim(l3, sum3, 2, wtailmul, sum4);
		int l5 = fast_expansion_sum_zeroelim(l4, sum4, 2, wtailmul+2, sum5);
		int l6 = fast_expansion_sum_zeroelim(l5, sum5, 2, wtailmul+4, sum6);
		det = sum6[l6-1];
		{
			//REAL det2 = exactDet3x3Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}
	}

	inline void getElement(const double4x4& src, double3x3& dest, 
		int r1, int r2, int r3, int c1, int c2, int c3)
	{
		dest[0][0] = src[r1][c1];
		dest[0][1] = src[r1][c2];
		dest[0][2] = src[r1][c3];

		dest[1][0] = src[r2][c1];
		dest[1][1] = src[r2][c2];
		dest[1][2] = src[r2][c3];

		dest[2][0] = src[r3][c1];
		dest[2][1] = src[r3][c2];
		dest[2][2] = src[r3][c3];
	}

	inline double exactDet4x4Sign(const double4x4& mat)
	{
		REAL sum[96];
		int xa, xb, xc, xd;
		double3x3 tmp;
		getElement(mat, tmp, 1, 2, 3, 1, 2, 3);
		exactDet3x3Sign(tmp, &xa, sum);

		getElement(mat, tmp, 1, 2, 3, 3, 2, 0);
		exactDet3x3Sign(tmp, &xb, sum+24);

		getElement(mat, tmp, 1, 2, 3, 3, 0, 1);
		exactDet3x3Sign(tmp, &xc, sum+48);

		getElement(mat, tmp, 1, 2, 3, 0, 2, 1);
		exactDet3x3Sign(tmp, &xd, sum+72);

		REAL suma[96];
		int la = scale_expansion_zeroelim(xa, sum, mat[0][0], suma);
		assert(la <= 24);
		int lb = scale_expansion_zeroelim(xb, sum+24, mat[0][1], suma+24);
		assert(lb <= 24);
		int lc = scale_expansion_zeroelim(xc, sum+48, mat[0][2], suma+48);
		assert(lc <= 24);
		int ld = scale_expansion_zeroelim(xd, sum+72, mat[0][3], suma+72);
		assert(ld <= 24);

		REAL det1[48], det2[48], det3[96];
		int lx = fast_expansion_sum_zeroelim(la, suma, lb, suma+24, det1);
		int ly = fast_expansion_sum_zeroelim(lc, suma+48, ld, suma+72, det2);

		int lz = fast_expansion_sum_zeroelim(lx, det1, ly, det2, det3);
		return det3[lz-1];
	}

	inline double adaptiveDet4x4Sign(const double4x4 &mat)
	{
	//mark1:
		REAL det2x2[12], det2x2a[12], det2x2b[12], e[12];
		Det2x2_Head(0, 0, 1, det2x2a[0], det2x2b[0], det2x2[0]);
		Det2x2_Head(0, 0, 2, det2x2a[1], det2x2b[1], det2x2[1]);
		Det2x2_Head(0, 0, 3, det2x2a[2], det2x2b[2], det2x2[2]);
		Det2x2_Head(0, 2, 3, det2x2a[3], det2x2b[3], det2x2[3]);
		Det2x2_Head(0, 3, 1, det2x2a[4], det2x2b[4], det2x2[4]);
		Det2x2_Head(0, 1, 2, det2x2a[5], det2x2b[5], det2x2[5]);
		Det2x2_Head(2, 2, 3, det2x2a[6], det2x2b[6], det2x2[6]);
		Det2x2_Head(2, 3, 1, det2x2a[7], det2x2b[7], det2x2[7]);
		Det2x2_Head(2, 1, 2, det2x2a[8], det2x2b[8], det2x2[8]);
		Det2x2_Head(2, 0, 1, det2x2a[9], det2x2b[9], det2x2[9]);
		Det2x2_Head(2, 0, 2, det2x2a[10], det2x2b[10], det2x2[10]);
		Det2x2_Head(2, 0, 3, det2x2a[11], det2x2b[11], det2x2[11]);
		
		e[0] = ::abs(det2x2[0]) + (::abs(det2x2a[0]) + ::abs(det2x2b[0]));
		e[1] = ::abs(det2x2[1]) + (::abs(det2x2a[1]) + ::abs(det2x2b[1]));
		e[2] = ::abs(det2x2[2]) + (::abs(det2x2a[2]) + ::abs(det2x2b[2]));

		e[3] = ::abs(det2x2[3]) + (::abs(det2x2a[3]) + ::abs(det2x2b[3]));
		e[4] = ::abs(det2x2[4]) + (::abs(det2x2a[4]) + ::abs(det2x2b[4]));
		e[5] = ::abs(det2x2[5]) + (::abs(det2x2a[5]) + ::abs(det2x2b[5]));

		e[6] = ::abs(det2x2[6]) + (::abs(det2x2a[6]) + ::abs(det2x2b[6]));
		e[7] = ::abs(det2x2[7]) + (::abs(det2x2a[7]) + ::abs(det2x2b[7]));
		e[8] = ::abs(det2x2[8]) + (::abs(det2x2a[8]) + ::abs(det2x2b[8]));

		e[9] = ::abs(det2x2[9]) + (::abs(det2x2a[9]) + ::abs(det2x2b[9]));
		e[10] = ::abs(det2x2[10]) + (::abs(det2x2a[10]) + ::abs(det2x2b[10]));
		e[11] = ::abs(det2x2[11]) + (::abs(det2x2a[11]) + ::abs(det2x2b[11]));

		REAL det = 0.0, det2x2mul[6];
		for (int i = 0; i < 6; i++)
		{
			det2x2mul[i] = det2x2[i] * det2x2[i+6];
		}

		REAL ta = det2x2mul[0] + det2x2mul[1];
		REAL tb = det2x2mul[2] + det2x2mul[3];
		REAL tc = det2x2mul[4] + det2x2mul[5];
		det = ta + tb + tc;
		REAL eb0 = ::abs(ta + tb) + ::abs(det);
		REAL eb1 = ::abs(ta) + ::abs(tb) + ::abs(tc);

#define DetxErr(i) (::abs(det2x2[i]*e[i+6]) + :: abs(det2x2[i+6]*e[i]))
		REAL eb2 = (DetxErr(0) + DetxErr(1))
			+ (DetxErr(2) + DetxErr(3))
			+ (DetxErr(4) + DetxErr(5));

		REAL eb3 = (e[0]*e[6] + e[1]*e[7])
			+ (e[2]*e[8] + e[3]*e[9]) 
			+ (e[4]*e[10] + e[5]*e[11]);


		REAL eb = eb0 * err4x4A0 + eb1 * err4x4A1 + 
			eb2 * err4x4A2 + eb3 * err4x4A3;
		if ((det > eb) != (det < -eb))
		{
			//REAL det2 = exactDet4x4Sign(mat);
			//if (!((det2*det > 0.0) || (det2 == 0.0 && det == 0.0)))
			//{
			//	assert(0);
			//	goto mark1;
			//}

			return det;
		}

		REAL Declare_Var, Declare_VarEX, det2x2multail[6];
		for (int i = 0; i < 6; i++)
		{
			Two_Product_Tail(det2x2[i], det2x2[i+6], det2x2mul[i], det2x2multail[i]);
		}

		REAL suma[4], sumb[4], sumc[4], sum1[8], sum2[12];
		Two_Two_Sum(det2x2mul[0], det2x2multail[0], det2x2mul[1], det2x2multail[1],
			suma[3], suma[2], suma[1], suma[0]);
		Two_Two_Sum(det2x2mul[2], det2x2multail[2], det2x2mul[3], det2x2multail[3],
			sumb[3], sumb[2], sumb[1], sumb[0]);
		Two_Two_Sum(det2x2mul[4], det2x2multail[4], det2x2mul[5], det2x2multail[5],
			sumc[3], sumc[2], sumc[1], sumc[0]);

		int l1 = fast_expansion_sum_zeroelim(4, suma, 4, sumb, sum1);
		int l2 = fast_expansion_sum_zeroelim(l1, sum1, 4, sumc, sum2);

		det = estimate(l2, sum2);
		eb = eb1 * err4x4A1 + eb2 * err4x4A2 + eb3 * err4x4A3;
		if ((det > eb) != (det < -eb))
		{
			//REAL det2 = exactDet4x4Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		REAL det2x2tail[12];
		Det2x2_Tail(0, 0, 1, det2x2a[0], det2x2b[0], det2x2[0], det2x2tail[0]);
		Det2x2_Tail(0, 0, 2, det2x2a[1], det2x2b[1], det2x2[1], det2x2tail[1]);
		Det2x2_Tail(0, 0, 3, det2x2a[2], det2x2b[2], det2x2[2], det2x2tail[2]);
		Det2x2_Tail(0, 2, 3, det2x2a[3], det2x2b[3], det2x2[3], det2x2tail[3]);
		Det2x2_Tail(0, 3, 1, det2x2a[4], det2x2b[4], det2x2[4], det2x2tail[4]);
		Det2x2_Tail(0, 1, 2, det2x2a[5], det2x2b[5], det2x2[5], det2x2tail[5]);
		Det2x2_Tail(2, 2, 3, det2x2a[6], det2x2b[6], det2x2[6], det2x2tail[6]);
		Det2x2_Tail(2, 3, 1, det2x2a[7], det2x2b[7], det2x2[7], det2x2tail[7]);
		Det2x2_Tail(2, 1, 2, det2x2a[8], det2x2b[8], det2x2[8], det2x2tail[8]);
		Det2x2_Tail(2, 0, 1, det2x2a[9], det2x2b[9], det2x2[9], det2x2tail[9]);
		Det2x2_Tail(2, 0, 2, det2x2a[10], det2x2b[10], det2x2[10], det2x2tail[10]);
		Det2x2_Tail(2, 0, 3, det2x2a[11], det2x2b[11], det2x2[11], det2x2tail[11]);

		if (det2x2tail[0] == 0.0 && det2x2tail[1] == 0.0
			&& det2x2tail[2] == 0.0 && det2x2tail[3] == 0.0
			&& det2x2tail[4] == 0.0 && det2x2tail[5] == 0.0
			&& det2x2tail[6] == 0.0 && det2x2tail[7] == 0.0
			&& det2x2tail[8] == 0.0 && det2x2tail[9] == 0.0
			&& det2x2tail[10] == 0.0 && det2x2tail[11] == 0.0)
		{
			//REAL det2 = exactDet4x4Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		for (int i = 0; i < 6; i++)
		{
			det += (det2x2[i] * det2x2tail[i+6]
				+ det2x2[i+6] * det2x2tail[i]);
		}
		eb = eb2 * err4x4B + eb3 * err4x4A3;
		if ((det > eb) != (det < -eb))
		{
			//REAL det2 = exactDet4x4Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}

		REAL det2x2tailsum[36], tmp1[2], tmp2[2], tmp3[4];
		for (int i = 0; i < 6; i++)
		{
			Two_Product(det2x2[i], det2x2tail[i+6], tmp1[1], tmp1[0]);
			Two_Product(det2x2[i+6], det2x2tail[i], tmp2[1], tmp2[0]);
			Two_Two_Sum(tmp1[1], tmp1[0], tmp2[1], tmp2[0],	tmp3[3], tmp3[2], tmp3[1], tmp3[0]);
			Two_Product(det2x2tail[i], det2x2tail[i+6], tmp2[1], tmp2[0]);
			auto ptr = det2x2tailsum+6*i;
			Four_Two_Sum(tmp3[3], tmp3[2], tmp3[1], tmp3[0], tmp2[1], tmp2[0],
				ptr[5], ptr[4], ptr[3], ptr[2], ptr[1], ptr[0]);
		}

		REAL sum3[12];
		int l3 = compress(l2, sum2, sum3);
		
		REAL twotwosuma[12], twotwosumb[12], twotwosumc[12];
		int la = fast_expansion_sum_zeroelim(6, det2x2tailsum, 6, det2x2tailsum+6, twotwosuma);
		int lb = fast_expansion_sum_zeroelim(6, det2x2tailsum+12, 6, det2x2tailsum+18, twotwosumb);
		int lc = fast_expansion_sum_zeroelim(6, det2x2tailsum+24, 6, det2x2tailsum+30, twotwosumc);

		REAL sum4[24], sum5[36], sum6[48];
		int l4 = fast_expansion_sum_zeroelim(l3, sum3, la, twotwosuma, sum4);
		int l5 = fast_expansion_sum_zeroelim(l4, sum4, lb, twotwosumb, sum5);
		int l6 = fast_expansion_sum_zeroelim(l5, sum5, lc, twotwosumc, sum6);

		det = sum6[l6-1];
		{
			//REAL det2 = exactDet4x4Sign(mat);
			//assert((det2*det > 0.0) || (det2 == 0.0 && det == 0.0));
			return det;
		}
	}
}