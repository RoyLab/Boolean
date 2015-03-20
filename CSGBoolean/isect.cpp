#include "precompile.h"
#include "isect.h"
#include <vector>
#include "Box3.h"
#include "Intersect.h"
#include "IsectTriangle.h"

#ifdef FINDMINMAX
#undef FINDMINMAX
#endif

#define FINDMINMAX(x0,x1,x2,min,max) \
    min = max = x0; \
    if(x1<min) min=x1;\
    if(x1>max) max=x1;\
    if(x2<min) min=x2;\
    if(x2>max) max=x2;

#ifndef min
#define min(i, j) (((i)<(j))?(i):(j))
#endif

#ifndef max
#define max(i, j) (((i)>(j))?(i):(j))
#endif

#define EPSF_2 (EPSF*EPSF)

namespace CSG
{
	bool TriangleAABBIntersectTest(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const AABBmp& bbox)
	    {
        // 我认为，这里的不等号加上等于号之后，可以作为开集的相交测试
        Vec3d c = bbox.Center();
        Vec3d e = bbox.Diagonal()*0.5;
        Vec3d v00 = v0- c;
        Vec3d v10 = v1 -c ;
        Vec3d v20 = v2 -c ; 
        //Compute edge vector 
        Vec3d f0 =  v10 -v00;
        f0 = Vec3d(fabs(f0[0]), fabs(f0[1]), fabs(f0[2]));
        Vec3d f1 =  v20 - v10;
        f1 = Vec3d(fabs(f1[0]), fabs(f1[1]), fabs(f1[2]));
        Vec3d f2 = v00 - v20;
        f2 = Vec3d(fabs(f2[0]), fabs(f2[1]), fabs(f2[2]));
        //Test axes a00 edge-edge test 
        double p0 = v00[2] * v10[1] - v00[1] * v10[2];
        double p2 = v20[2] * (v10[1] - v00[1]) -v20[1] * (v10[2] - v00[2]);
        double r  = e[1] * f0[2] + e[2] * f0[1] ; 
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 
        // Test axes a01 edge -edge 
        p0 = v10[2] * v20[1] - v10[1] * v20[2];
        p2 = v00[2]*(v20[1] - v10[1]) - v00[1] *( v20[2] - v10[2]);
        r = e[1]* f1[2] + e[2] * f1[1];
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 
        // Test axes a02 edge  (dot (v2, a02))
        p0  = v20[2] *v00[1] - v20[1] * v00[2];
        p2 = v10[2] *(v00[1] -v20[1]) - v10[1] *(v00[2] - v20[2] );
        r = e[1] *  f2[2]  + e[2] * f2[1]; 
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 

       // test axes a10 edge - edge  
        p0 = v00[0]* v10[2] - v00[2] * v10[0] ; 
        p2 = v20[0] *(v10[2] - v00[2]) - v20[2] *(v10[0] - v00[0]);
        r = e[0] * f0[2] + e[2] * f0[0] ; 
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 
        p0 = v10[0] * v20[2]- v10[2] * v20[0];
        p2 = v00[0]*(v20[2] - v10[2]) - v00[2] *( v20[0] - v10[0]);
         r = e[0] * f1[2] + e[2] * f1[0] ; 
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 
        p0  = v20[0] *v00[2] - v20[2] * v00[0];
        p2 = v10[0] *(v00[2] -v20[2]) - v10[2] *(v00[0] - v20[0]);
        r = e[0] * f2[2] + e[2] * f2[0] ; 
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 

        // test axes a20 edge 
        p0 = v00[1]* v10[0] - v00[0] * v10[1] ; 
        p2 = v20[1] *(v10[0] - v00[0]) - v20[0] *(v10[1] - v00[1]);
        r = e[0] * f0[1] + e[1]* f0[0] ; 
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 
        p0 = v10[1] * v20[0] - v10[0] * v20[1];
        p2 = v00[1]*(v20[0] - v10[0]) - v00[0] *( v20[1] - v10[1]);
        r = e[0] * f1[1] + e[1]* f1[0] ; 
        if ( max(- max (p0, p2), min(p0, p2)) > r )
             return false ; 
        p0  = v20[1] *v00[0] - v20[0] * v00[1];
        p2 = v10[1] *(v00[0] -v20[0]) - v10[0] *(v00[1] - v20[1]);
        r = e[0] * f2[1] + e[1]* f2[0] ; 
        if ( max(- max (p0, p2), min(p0, p2)) > r  )
             return false ; 

        //   /* test in X-direction */
       double minVal, maxVal ; 
       FINDMINMAX(v00[0], v10[0], v20[0] ,minVal,maxVal);
       if(minVal> e[0] || maxVal<-e[0] ) return false;
       FINDMINMAX(v00[1], v10[1], v20[1] ,minVal,maxVal);
       if(minVal> e[1]  || maxVal<-e[1] ) return false; 
       FINDMINMAX(v00[2], v10[2], v20[2] ,minVal,maxVal);
       if(minVal> e[2] || maxVal<-e[2] ) return false;
       //test 
        Vec3d normal = cross ((v10- v00), (v20 - v10));
        double       d = - dot (normal, v0);
        Vec3d  e1 = bbox.Diagonal()*0.5;
        double  r1 = dot(e1, Vec3d(fabs(normal[0]), fabs(normal[1]), fabs(normal[2])));
        double  s = dot (normal,  bbox.Center()) + d; 
        return  (fabs(s) <= (r1)); 
    }



static inline void isect2(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const double vv0, const double vv1, const double vv2,
						  const double d0, const double d1, const double d2, 
						  double* isect0, double* isect1, Vec3d& isectpoint0, Vec3d& isectpoint1)
						  
{
		double tmp = d0 / (d0 - d1);
		*isect0  = vv0 + (vv1 - vv0)*tmp;
		Vec3d diff = (v1-v0) * tmp;
		isectpoint0 = v0 + diff;

		tmp = d0 / (d0 - d2);
		*isect1 = vv0 + (vv2 - vv0)*tmp;
		diff =(v2-v0) * tmp;
		isectpoint1 = v0 + diff;
}

static inline void isect(const Vec3d& v0, const Vec3d& v1, const double vv0, const double vv1, double d0, double d1,
							double* isect0, Vec3d& isectpoint0)
						  
{
		double tmp = d0 / (d0 - d1);
		*isect0  = vv0 + (vv1 - vv0)*tmp;
		Vec3d diff = (v1-v0) * tmp;
		isectpoint0 = v0 + diff;
}

static inline int compute_intervals_isectline(const Vec3d& v0, const Vec3d& v1,  const Vec3d& v2, 
												int index, const double* d, const int* sd, 
												double* isect0, double* isect1, Vec3d& isectpoint0, Vec3d& isectpoint1,
												int& isectType0, int& isectType1)
{
	if (sd[0] == 0)
	{
		if (sd[1] == 0)
		{
#ifdef _DEBUG
			if (sd[2] == 0) assert(0);
#endif
			*isect0 = v0[index];
			*isect1 = v1[index];
			isectpoint0 = v0;
			isectpoint1 = v1;
			isectType0 = VER_0;
			isectType1 = VER_1;
			return EDGE_2;
		}
		else if (sd[2] == 0)
		{
#ifdef _DEBUG
			if (sd[1] == 0) assert(0);
#endif
			*isect0 = v0[index];
			*isect1 = v2[index];
			isectpoint0 = v0;
			isectpoint1 = v2;
			isectType0 = VER_0;
			isectType1 = VER_2;
			return EDGE_1;
		}
		else if (sd[1]*sd[2] > 0)
		{
			*isect0 = v0[index];
			*isect1 = v0[index];
			isectpoint0 = v0;
			isectpoint1 = v0;
			isectType0 = VER_0;
			isectType1 = VER_0;
			return VER_0;
			//return -1; // point intersection
		}
		else
		{
			*isect0 = v0[index];
			isectpoint0 = v0;
			isectType0 = VER_0;

			isect(v1, v2, v1[index], v2[index], d[1], d[2], isect1, isectpoint1);
			isectType1 = EDGE_0;
		}
	}
	else if (sd[1] == 0)
	{
		if (sd[2] == 0)
		{
			*isect0 = v1[index];
			*isect1 = v2[index];
			isectpoint0 = v1;
			isectpoint1 = v2;
			isectType0 = VER_1;
			isectType1 = VER_2;
			return EDGE_0;
		}
		else if (sd[0]*sd[2] > 0)
		{
			*isect0 = v1[index];
			*isect1 = v1[index];
			isectpoint0 = v1;
			isectpoint1 = v1;
			isectType0 = VER_1;
			isectType1 = VER_1;
			return VER_1;
			//return -1; // point intersection
		}
		else
		{
			*isect0 = v1[index];
			isectpoint0 = v1;
			isectType0 = VER_1;

			isect(v0, v2, v0[index], v2[index], d[0], d[2], isect1, isectpoint1);
			isectType1 = EDGE_1;
		}
	}
	else if (sd[2] == 0)
	{
		if (sd[0]*sd[1] > 0)
		{
			*isect0 = v2[index];
			*isect1 = v2[index];
			isectpoint0 = v2;
			isectpoint1 = v2;
			isectType0 = VER_2;
			isectType1 = VER_2;
			return VER_2;
			//return -1; // point intersection
		}
		else
		{
			*isect0 = v2[index];
			isectpoint0 = v2;
			isectType0 = VER_2;

			isect(v0, v1, v0[index], v1[index], d[0], d[1], isect1, isectpoint1);
			isectType1 = EDGE_2;
		}
	}
	else
	{
		if (sd[0]*sd[1] > 0)
		{
			isect2(v2, v0, v1, v2[index], v0[index], v1[index], d[2], d[0], d[1],
				isect0, isect1, isectpoint0, isectpoint1);
			isectType0 = EDGE_1;		isectType1 = EDGE_0;
		}
		else if (sd[0]*sd[2] > 0)
		{
			isect2(v1, v0, v2, v1[index], v0[index], v2[index], d[1], d[0], d[2],
				isect0, isect1, isectpoint0, isectpoint1);
			isectType0 = EDGE_2;		isectType1 = EDGE_0;
		}
		else if (sd[1]*sd[2] > 0)
		{
			isect2(v0, v1, v2, v0[index], v1[index], v2[index], d[0], d[1], d[2],
				isect0, isect1, isectpoint0 , isectpoint1);
			isectType0 = EDGE_2;		isectType1 = EDGE_1;
		}
	}

	return 0;
}

template <typename P>
static inline int sort2(P& a, P& b)
{
	if (a > b)
	{
		std::swap(a, b);
		return 1;
	}
	return 0;
}


int TriTriIntersectTest(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const Vec3d& nv, 
							const Vec3d& u0, const Vec3d& u1, const Vec3d& u2, const Vec3d& nu,
							int& startType, int& endType, Vec3d& start, Vec3d& end)
{
	/* compute plane equation of triangle(p0,p1,p2) */
	double  d1=-dot(nv, v0);
	/* plane equation 1: N1.X+d1=0 */
	/* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
	double du[3];
	du[0] = dot(nv,u0)+d1;
	du[1] = dot(nv,u1)+d1;
	du[2] = dot(nv,u2)+d1;
	int sdu[3];
	GS::NormalDistToSign(du, sdu);

	if ((sdu[0] == sdu[1]) && (sdu[1] == sdu[2]))
		if (sdu[0] = 0) return 0;
		else return -1; 

	double d2=-dot(nu, u0);
	double dv[3];
	dv[0] =dot(nu,v0)+d2;
	dv[1] = dot(nu,v1)+d2;
	dv[2] = dot(nu,v2)+d2;
	int sdv[3];
	GS::NormalDistToSign(dv, sdv);
	if ((sdv[0] == sdv[1]) && (sdv[1] == sdv[2]))
		if (sdu[0] = 0) return 0;
		else return -1; 

	/* compute direction of intersection line */
	Vec3d LineDir = cross(nv, nu);

	/* compute and index to the largest component of D */
	double max=fabs(LineDir[0]);
	int index=0;
	double b=fabs(LineDir[1]);
	double c=fabs(LineDir[2]);
	if(b>max) max=b,index=1;
	if(c>max) max=c,index=2;

	int type00(0), type01(0), type10(0), type11(0);
	
	/* compute interval for triangle 1 */
	double isect1[2];   
	Vec3d  isectpointA1,isectpointA2;
	int res1 = compute_intervals_isectline(v0,v1,v2,index,dv,sdv,&isect1[0],&isect1[1],isectpointA1,isectpointA2, type00, type01);

	/* compute interval for triangle 2 */
	double isect2[2]; 
	Vec3d  isectpointB1,isectpointB2;
	int res2 = compute_intervals_isectline(u0,u1,u2,index,du,sdu,&isect2[0],&isect2[1],isectpointB1,isectpointB2, type10, type11);

	res2 <<= 16;
	type10 <<= 16;
	type11 <<= 16;

	int smallest1 = sort2(isect1[0],isect1[1]);
	int smallest2 = sort2(isect2[0],isect2[1]);
	if(isect1[1]<isect2[0] || isect2[1]<isect1[0]) // 要不要用epsf?
		return -1;

	/* at this point, we know that the triangles intersect */
	double dstart = isect1[0] - isect2[0];
	if(dstart > EPSF)
	{
		if (smallest1 == 0)
		{
			start = isectpointA1;
			startType ^= type00;
		}
		else
		{
			start = isectpointA2;
			startType ^= type01;
		}

		if (res2) startType ^= res2;
	}
	else if (dstart < -EPSF)
	{
		if (smallest2 == 0)
		{
			start = isectpointB1;
			startType ^= type10;
		}
		else 
		{
			start = isectpointB2;
			startType ^= type11;
		}
		if (res1) startType ^= res1; // 这里可能会出问题
	}
	else 
	{
		if (smallest1 == 0)
		{
			start = isectpointA1;
			startType ^= type00;
		}
		else
		{
			start = isectpointA2;
			startType ^= type01;
		}

		if (smallest2 == 0)
		{
			start = (isectpointB1+start)/2.0;
			startType ^= type10;
		}
		else 
		{
			start = (isectpointB2+start)/2.0;
			startType ^= type11;
		}
	}

	double dend = isect2[1] - isect1[1];
	if(dend > EPSF)
	{
		if (smallest1 == 0)
		{
			end = isectpointA2;
			endType ^= type01;
		}
		else
		{
			end = isectpointA1;
			endType ^= type00;
		}
		if (res2) endType ^= res2;
	}
	else if (dend < -EPSF)
	{
		if (smallest2 == 0)
		{
			end = isectpointB2;
			endType ^= type11;
		}
		else 
		{
			end = isectpointB1;
			endType ^= type10;
		}
		if (res1) endType ^= res1;
	}
	else 
	{
		if (smallest1 == 0)
		{
			end = isectpointA2;
			endType ^= type01;
		}
		else
		{
			end = isectpointA1;
			endType ^= type00;
		}

		if (smallest2 == 0)
		{
			end = (isectpointB2+end)/2.0;
			endType ^= type11;
		}
		else 
		{
			end = (isectpointB1+end)/2.0;
			endType ^= type10;
		}
	}

	return true;
}

bool RayTriangleIntersectTest(const Vec3d& o, const Vec3d& d, const Vec3d& v0, const Vec3d& v1,  const Vec3d& v2,
                              double& u, double& v, double& t)
{
    Vec3d e1 = v1- v0;
    Vec3d e2 = v2 -v0;
    Vec3d p = cross(d, e2);
    double det = dot (p, e1);

    if (det > -EPSF && det < EPSF)
        return false;

    Vec3d f ; 
    if( det >0 )
        f = o -v0;
     else {
           f = v0 - o;
          det = -det;
     }
        // If determinant is near zero, ray lies in plane of triangle
       // Calculate u and make sure u <= 1
      u = dot(f, p);
     if( u < 0.0f || u > det )
         return false;

   // Q
   Vec3d q = cross(f, e1);

    // Calculate v and make sure u + v <= 1
   v = dot(q, d);
    if( v < 0.0f || u + v > det )
       return false;

   // Calculate t, scale parameters, ray intersects triangle
    t = dot(q, e2);

   float fInvDet = 1.0f / det;
    t *= fInvDet;
    u *= fInvDet;
    v *= fInvDet;
    return true;
}
}
