#include "typedefs.h"
#include "arithmetic.h"

namespace GS{

#define USE_EPSILON TRUE

    /* this edge to edge test is based on Franklin Antonio's gem:
    "Faster Line Segment Intersection", in Graphics Gems III,
    pp. 199-202 */ 
#define EDGE_EDGE_TEST(V0,U0,U1)                        \
    Bx=U0[i0]-U1[i0];                                   \
    By=U0[i1]-U1[i1];                                   \
    Cx=V0[i0]-U0[i0];                                   \
    Cy=V0[i1]-U0[i1];                                   \
    f=Ay*Bx-Ax*By;                                      \
    d=By*Cx-Bx*Cy;                                      \
    if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
    {                                                   \
        e=Ax*Cy-Ay*Cx;                                  \
        if(f>0)                                         \
        {                                               \
            if(e>=0 && e<=f) return 1;                  \
        }                                               \
        else                                            \
        {                                               \
            if(e<=0 && e>=f) return 1;                  \
        }                                               \
    }                                

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2)       \
    {                                                \
        float Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
        Ax=V1[i0]-V0[i0];                            \
        Ay=V1[i1]-V0[i1];                            \
        /* test edge U0,U1 against V0,V1 */          \
        EDGE_EDGE_TEST(V0,U0,U1);                    \
        /* test edge U1,U2 against V0,V1 */          \
        EDGE_EDGE_TEST(V0,U1,U2);                    \
        /* test edge U2,U1 against V0,V1 */          \
        EDGE_EDGE_TEST(V0,U2,U0);                    \
    }

#define POINT_IN_TRI(V0,U0,U1,U2)                 \
    {                                             \
        float a,b,c,d0,d1,d2;                     \
        /* is T1 completly inside T2? */          \
        /* check if V0 is inside tri(U0,U1,U2) */ \
        a=U1[i1]-U0[i1];                          \
        b=-(U1[i0]-U0[i0]);                       \
        c=-a*U0[i0]-b*U0[i1];                     \
        d0=a*V0[i0]+b*V0[i1]+c;                   \
                                                  \
        a=U2[i1]-U1[i1];                          \
        b=-(U2[i0]-U1[i0]);                       \
        c=-a*U1[i0]-b*U1[i1];                     \
        d1=a*V0[i0]+b*V0[i1]+c;                   \
                                                  \
        a=U0[i1]-U2[i1];                          \
        b=-(U0[i0]-U2[i0]);                       \
        c=-a*U2[i0]-b*U2[i1];                     \
        d2=a*V0[i0]+b*V0[i1]+c;                   \
        if(d0*d1>0.0)                             \
        {                                         \
            if(d0*d2>0.0) return 1;               \
        }                                         \
    }

#ifdef CALCULATE_COPLANAR_INTERSECTIONS
static int coplanar_tri_tri(float N[3],float V0[3],float V1[3],float V2[3],
    float U0[3],float U1[3],float U2[3])
{
    float A[3];
    short i0,i1;
    /* first project onto an axis-aligned plane, that maximizes the area */
    /* of the triangles, compute indices: i0,i1. */
    A[0]=fabs(N[0]);
    A[1]=fabs(N[1]);
    A[2]=fabs(N[2]);
    if(A[0]>A[1])
    {
        if(A[0]>A[2])  
        {
            i0=1;      /* A[0] is greatest */
            i1=2;
        }
        else
        {
            i0=0;      /* A[2] is greatest */
            i1=1;
        }
    }
    else   /* A[0]<=A[1] */
    {
        if(A[2]>A[1])
        {
            i0=0;      /* A[2] is greatest */
            i1=1;                                           
        }
        else
        {
            i0=0;      /* A[1] is greatest */
            i1=2;
        }
    }               

    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(V0,U0,U1,U2);
    POINT_IN_TRI(U0,V0,V1,V2);

    return 0;
}
#endif


/* sort so that a<=b */
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

template < typename P> 
static inline void NormalDistToSign( P d[3], int result[3])
{

	for (int i  = 0; i< 3; i++)
	{
#if USE_EPSILON ==TRUE
		if (d[i] > PLANE_THICK_EPS)
			result[i] =1;
		else if (d[i] < -PLANE_THICK_EPS)
			result[i] = -1;
#else 
	    if (d[i] > 0)
			result[i] =1;
		else if (d[i] < 0)
			result[i] = -1;
#endif 
		else 
		{
		    d[i] =   0;
			result[i] = 0; 
		}
	}

}


template<typename P>
static inline void isect2(const vec3<P>& v0, const vec3<P>& v1,  const vec3<P>& v2, 
						  const P& vv0, const P& vv1, const P& vv2, P d0, P d1, P d2,
						   P* isect0, P* isect1, vec3<P>& isectpoint0,   vec3<P>& isectpoint1)
						  
{
      P tmp = d0 / (d0 - d1);
      isect0  = vv0 + (vv1 - vv0)*tmp;
      vec3<P> diff = (v1-v0) * tmp;
      isectpoint0 = v0 + diff;

      tmp = d0 / (d0 - d2);
      isect1 = vv0 + (vv2 - vv0)*tmp;
      diff =(v2-v0) * tmp;
      isectpoint1 = v0 + diff;
}

template< typename P> 
static inline int compute_intervals_isectline(const vec3<P>& v0, const vec3<P>& v1,  const vec3<P>& v2, 
						                      const P& vv0, const P& vv1, const P& vv2, P d0, P d1, P d2, 
											  P* isect0, P* isect1, vec3<P>& isectpoint0,   vec3<P>& isectpoint1)
{
	if ( d0*d1 > 0 )
		 isect2(v2, v0, v1, vv2, vv0, vv1, d2, d0, d1, isect0, isect1, isectpoint0, isectpoint0);
	else if (d0*d2 > 0)
		isect2(v1, v0, v2, vv1, vv0, vv2, d1, d0, d2, isect0, isect1, isectpoint0, isectpoint1);
    else if ((d1*d2 > 0) || (d0 != 0) )
		isect2(v0, v1, v2, vv0, vv1, vv2, d0, d1, d2,isect0, isect1, isectpoint0 , isectpoint1);
    else if (d1 !=0 )
		isect2(v1, v0, v2, vv1, vv0, vv2, d1, d0, d2,isect0, isect1, isectpoint0, isectpoint1);       
	else if (d2 != 0 )
		isect2(v2, v0, v1, vv2, vv0, vv1, d2, d0, d1,isect0, isect1, isectpoint0, isectpoint1);
	else                                               
    {                                                   
        /* triangles are coplanar */    
        return 1;
    }
    return 0;
}

template<typename P> 
bool TriTriIntersectTest(const vec3<P>& v0, const vec3<P>& v1, const vec3<P>& v2, const vec3<P>& nv, 
						 const vec3<P>& u0, const vec3<P>& u1, const vec3<P>& u2, const vec3<P>& nu,
						 vec3<P>& startPos, const vec3<P>& endPos)
{
	/* compute plane equation of triangle(p0,p1,p2) */
     P  d1=-dot(nv, v0);
    /* plane equation 1: N1.X+d1=0 */
	/* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
	P du[3];
	du[0] =dot(nv,u0)+d1;
    du[1] = dot(nv,u1)+d1;
    du[2] = dot(nv,u2)+d1;
	int sdu[3];
	NormalDistToSign(du, sdu);
	if ((sdu[0] = sdu[1]) && (sdu[1] = sdu[2]))
		return false ; 

	P d2=-dot(nu, u0);
	P dv[3];
	dv[0] =dot(nv,v0)+d1;
    dv[1] = dot(nv,v1)+d1;
    dv[2] = dot(nv,v2)+d1;
	int sdv[3];
	NormalDistToSign(du, sdv);
	if ((sdv[0] = sdv[1]) && (sdv[1] = sdv[2]))
		return false ; 
    /* compute direction of intersection line */
	 vec3<P> LineDir = cross(nv, nu);

    /* compute and index to the largest component of D */
    P max=fabs(LineDir.x);
    int index=0;
    P b=fabs(LineDir.y);
    P c=fabs(LineDir.z);
    if(b>max) max=b,index=1;
    if(c>max) max=c,index=2;

    /* this is the simplified projection onto L*/
    P& vp0=v0[index];
    P& vp1=v1[index];
    P& vp2=v2[index];

    P& up0=u0[index];
    P& up1=u1[index];
    P& up2=u2[index];

   /* compute interval for triangle 1 */
    P isect1[2];   
    vec3<P>  isectpointA1,isectpointA2;
    bool isCoplanar=compute_intervals_isectline(v0,v1,v2,vp0,vp1,vp2,dv[0],dv[1],dv[2],
                                           &isect1[0],&isect1[1],isectpointA1,isectpointA2);    
	if(isCoplanar)
    {
#ifdef CALCULATE_COPLANAR_INTERSECTIONS
            return coplanar_tri_tri(nv,v0,v1,v2,u0,u1,u2);     
#else
            return false;
#endif
     }

      /* compute interval for triangle 2 */
	 P isect2[2]; 
	 vec3<P>  isectpointB1,isectpointB2;
     isCoplanar=compute_intervals_isectline(U0,U1,U2,up0,up1,up2,du0,du1,du2,
                                  &isect2[0],&isect2[1],isectpointB1,isectpointB2);
     if(isCoplanar)
     {
#ifdef CALCULATE_COPLANAR_INTERSECTIONS
            return coplanar_tri_tri(nv,v0,v1,v2,u0,u1,u2);     
#else
            return false;
#endif
      }
	  int smallest1 = sort2(isect1[0],isect1[1]);
      int smallest2 = sort2(isect2[0],isect2[1]);
      if(isect1[1]<isect2[0] || isect2[1]<isect1[0])
		  return 0;
     /* at this point, we know that the triangles intersect */
      if(isect2[0]<isect1[0])
      {
		  startPos = smallest1==0 ? isectpointA1:isectpointA2;
          
		  if(isect2[1]<isect1[1])
			   endPos = smallest2==0? isectpointB2 : isectpointB1£»
          else
			   endPos = smallest1==0? isectpointA2 : isectpointA1;
        }
        else
        {
			startPos = smallest2==0 ? isectpointB1:isectpointB2;
           
            if(isect2[1]>isect1[1])
				endPos = smallest1 == 0 ? isectpointA2:isectpointA1;
            else
				endPos = smallest2 == 0 ? isectpointB2:isectpointB1;
        }
        return 1;

}

}