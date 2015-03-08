#include "precompile.h"
#include "isect.h"

#include <vector>
#include "Box3.h"
#include "Intersect.h"

using GS::max;
using GS::min;

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
       double min, max ; 
       FINDMINMAX(v00[0], v10[0], v20[0] ,min,max);
       if(min> e[0] || max<-e[0] ) return false;
       FINDMINMAX(v00[1], v10[1], v20[1] ,min,max);
       if(min> e[1]  || max<-e[1] ) return false; 
       FINDMINMAX(v00[2], v10[2], v20[2] ,min,max);
       if(min> e[2] || max<-e[2] ) return false;
       //test 
        Vec3d normal = cross ((v10- v00), (v20 - v10));
        double       d = - dot (normal, v0);
        Vec3d  e1 = bbox.Diagonal()*0.5;
        double  r1 = dot(e1, Vec3d(fabs(normal[0]), fabs(normal[1]), fabs(normal[2])));
        double  s = dot (normal,  bbox.Center()) + d; 
        return  (fabs(s) <= (r1)); 
    }

	//bool TriTriIntersectTest(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const Vec3d& nv, 
	//						 const Vec3d& u0, const Vec3d& u1, const Vec3d& u2, const Vec3d& nu,
	//						 std::vector<GS::Seg3D<double>>& intersects)
	//{
	//	/* compute plane equation of triangle(p0,p1,p2) */
	//	 double  d1=-dot(nv, v0);
	//	/* plane equation 1: N1.X+d1=0 */
	//	/* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
	//	double du[3];
	//	du[0] = dot(nv,u0)+d1;
	//	du[1] = dot(nv,u1)+d1;
	//	du[2] = dot(nv,u2)+d1;
	//	int sdu[3];
	//	NormalDistToSign(du, sdu);
	//	if(sdu[0] == 0 && sdu[1] == 0 && sdu [2] ==0)
	//		return false;

	//	if ((sdu[0] == sdu[1]) && (sdu[1] == sdu[2]))
	//		return false ; 

	//	double d2=-dot(nu, u0);
	//	double dv[3];
	//	dv[0] =dot(nu,v0)+d2;
	//	dv[1] = dot(nu,v1)+d2;
	//	dv[2] = dot(nu,v2)+d2;
	//	int sdv[3];
	//	NormalDistToSign(dv, sdv);
	//	if ((sdv[0] == sdv[1]) && (sdv[1] == sdv[2]))
	//		return false ; 
	//	/* compute direction of intersection line */
	//	Vec3d LineDir = cross(nv, nu);

	//	/* compute and index to the largest component of D */
	//	double max=fabs(LineDir.x);
	//	int index=0;
	//	double b=fabs(LineDir.y);
	//	double c=fabs(LineDir.z);
	//	if(b>max) max=b,index=1;
	//	if(c>max) max=c,index=2;

	//	/* this is the simplified projection onto L*/
	//	double vp0=v0[index];
	//	double vp1=v1[index];
	//	double vp2=v2[index];

	//	double up0=u0[index];
	//	double up1=u1[index];
	//	double up2=u2[index];

	//	/* compute interval for triangle 1 */
	//	double isect1[2];   
	//	Vec3d  isectpointA1,isectpointA2;
	//	bool isCoplanar=compute_intervals_isectline(v0,v1,v2,vp0,vp1,vp2,dv[0],dv[1],dv[2],
	//										   &isect1[0],&isect1[1],isectpointA1,isectpointA2);

	//	/* compute interval for triangle 2 */
	//	double isect2[2]; 
	//	Vec3d  isectpointB1,isectpointB2;
	//	isCoplanar=compute_intervals_isectline(u0,u1,u2,up0,up1,up2,du[0],du[1],du[2],
	//								&isect2[0],&isect2[1],isectpointB1,isectpointB2);

	//	int smallest1 = sort2(isect1[0],isect1[1]);
	//	int smallest2 = sort2(isect2[0],isect2[1]);
	//	if(isect1[1]<isect2[0] || isect2[1]<isect1[0])
	//		return false;

	//	/* at this point, we know that the triangles intersect */
	//	GS::Seg3D<double> seg;
	//	if(isect2[0]<isect1[0])
	//	{
	//		seg.start = smallest1==0 ? isectpointA1:isectpointA2;
 //         
	//		if(isect2[1]<isect1[1])
	//			seg.end = smallest2==0? isectpointB2 : isectpointB1;
	//		else
	//			seg.end = smallest1==0? isectpointA2 : isectpointA1;
	//	}
	//	else
	//	{
	//		seg.start = smallest2==0 ? isectpointB1:isectpointB2;
 //          
	//		if(isect2[1]>isect1[1])
	//			seg.end = smallest1 == 0 ? isectpointA2:isectpointA1;
	//		else
	//			seg.end = smallest2 == 0 ? isectpointB2:isectpointB1;
	//	}
	//	intersects.push_back(seg);
	//	return true;

	}


}
