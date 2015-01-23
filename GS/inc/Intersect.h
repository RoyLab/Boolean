#ifndef __INTERSECT_H__
#define __INTERSECT_H__
#include "typedefs.h"
#include "arithmetic.h"

namespace GS{

#define USE_EPSILON TRUE
#define CALCULATE_COPLANAR_INTERSECTIONS

template <typename P> struct Seg3D{
	vec3<P> start;
	vec3<P> end;
};



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


// inSegment(): determine if a point is inside a segment
//    Input:  a point P, and a collinear segment S
//    Return: 1 = P is inside S
//            0 = P is  not inside S

template <typename P> 
static inline int inSegment(const vec2<P>& p, const vec2<P>& v1, const vec2<P>& v2 )
{
    if (v1.x != v2.x) {    // V is not  vertical
        if (v1.x <= p.x && p.x <= v2.x)
            return 1;
        if (v1.x >= p.x && p.x >= v2.x)
            return 1;
    }
    else {    // V is vertical, so test y  coordinate
        if (v1.y <= p.y && p.y <= v2.y)
            return 1;
        if (v1.y >= p.y && p.y >= v2.y)
            return 1;
    }
    return 0;
}

//    Output: *I0 = factor of intersect point (when it exists)
//            *I1 =  factor of endpoint of intersect segment [I0,I1] (when it exists)
//            firstend :  
//    Return: 0=disjoint (no intersect)
//            1=intersect  in unique point I0
//            2=overlap  in segment from I0 to I1
//   
template< typename P> 
static inline int LinesIntersect(const vec2<P>& p1, const vec2<P>& p2, const vec2<P>& v1, const vec2<P>& v2,bool& firstend , P& I0, P& I1 )
{
	vec2<P> A = p2 -p1;
	vec2<P> B = v2 -v1; 
	P low, high;
	if (A.x < 0)
	{
		low = p2.x;
		high = p1.x;
	}else {
		low  = p1.x;
		high = p2.x;
	}
    if (B.x > 0)
	{
		if ((high < v1.x) ||(v2.x < low)) 
			return 0 ; 
	}else {
		if ((high < v2.x) || (v1.x < low))
			return 0 ;
	}
	if (A.y < 0)
	{
		low = p2.y;
		high = p1.y;
	}else {
		low  = p1.y;
		high = p2.y;
	}
	if (B.y > 0)
	{
		if ((high < v1.y) || (v2.y < low))
			return 0 ;
	}else {
		if ((high < v2.y) || (v1.y < low))
			return 0 ;
	}
	vec2<P> W = p1- v1;
	P d = cross(B, W);
	P f = cross(A, B);
	if (fabs(f) <EPSF)
	{
		//parallel 
		 if (cross(A,W) != 0 || cross(B,W) != 0)  {
            return 0;                    // they are NOT collinear
        }
		 // they are collinear or degenerate
        // check if they are degenerate  points
        float du = dot(A,A);
        float dv = dot(B,B);
        if (du==0 && dv==0) {            // both segments are points
            if (p1 !=  v1)         // they are distinct  points
                 return 0;
            I0 = 0;                 // they are the same point
            firstend = true;
			return 1;
        }
        if (du==0) {                     // p1 is a single point
            if  (inSegment(p1, v1, v2) == 0)  // but is not in v
                 return 0;
            I0 = 0;
			 firstend = true;
            return 1;
        }
        if (dv==0) {                     // v1 single point
            if  (inSegment(v1, p1, p2) == 0)  // but is not in p
                 return 0;
            I0 = 0;
			 firstend = false;
            return 1;
        }
		 // they are collinear segments - get  overlap (or not)
        P t0, t1;                    // endpoints of S1 in eqn for S2
        vec2<P> w2 = p2 - v1;
        if (B.x != 0) {
                 t0 = W.x / B.x;
                 t1 = w2.x / B.x;
        }
        else {
                 t0 = W.y / B.y;
                 t1 = w2.y / B.y;
        }
        if (t0 > t1) {                   // must have t0 smaller than t1
                 P t=t0; t0=t1; t1=t;    // swap if not
        }
        if (t0 > 1 || t1 < 0) {
            return 0;      // NO overlap
        }
        t0 = t0<0? 0 : t0;               // clip to min 0
        t1 = t1>1? 1 : t1;               // clip to max 1
        if (t0 == t1) {                  // intersect is a point
			I0 = t0;
			firstend = false;
            return 1;
        }

        // they overlap in a valid subsegment
        I0 =  t0 ;
        I1 = t1 ;
		firstend = false;
	   return 2;

	}else if ((d >=0 && f > 0 && d <= f) || (f<0 && d<=0 && d>=f))
	{
		P e = cross(A, W);
	    if ((e >=0 && f > 0 && e <= f) || (f<0 && e<=0 && e>=f))
		{
			I0 =  d/f   ;
		    firstend = true;
			return 1;
		}
	}
    return 0 ; 

}



#ifdef CALCULATE_COPLANAR_INTERSECTIONS


template <typename P> 
static inline bool EdgeTriangleTest(const vec3<P>& v0, const vec3<P>& v1, const vec3<P>& u0, const vec3<P>& u1, const vec3<P>& u2, 
									int i0, int i1, std::vector<Seg3D<P> >& intersects )
{
	const vec3<P>* u[3] = {&u0, &u1, &u2} ;
	vec2<P> V[3];
	V[0] = vec2<P>(v0[i0], v0[i1]);
	V[1] = vec2<P>(v1[i0], v1[i1]);
	vec2<P> U[3];
	U[0] = vec2<P>(u0[i0], u0[i1]);
    U[1] = vec2<P>(u1[i0], u1[i1]);
	U[2] = vec2<P>(u2[i0], u2[i1]);
	bool bFistPoint = false ; 
	Seg3D<P> p;
	P t0, t1;
	for(int j = 0;  j < 3 ; j++)
	{
			vec2<P> I0, I1;
			bool firstEnd;
			int  result = LinesIntersect(V[0], V[1], U[j], U[(j+1)%3], firstEnd, t0, t1);
			if (result == 2)
			{
				if (!firstEnd)
				{
					p.start = *u[j] +t0*(*u[(j+1)%3]- *u[j]);
					p.end   = *u[j] +t1*(*u[(j+1)%3]- *u[j]);
				}else {
					p.start = v0 + t0*(v1-v0);
					p.end =   v0 + t1*(v1 -v0);
				}
				intersects.push_back(p);
				return true;


			}else if (result  == 1){
				if (bFistPoint  )
				{
					if (!firstEnd)
					{
						p.end = *u[j] + t0* (*u[(j+1)%3]-*u[j]);
					}else {
						p.end = v0 + t0* (v1 -v0);
					}
                    if (!vec3_equal(p.start,p.end))
                    {
					    intersects.push_back(p);
					    return true;
                    }
				}
				else 
				{
					if (!firstEnd)
					{
						p.start = *u[j] + t0* (*u[(j+1)%3]-*u[j]);
					}else {
						p.start = v0 + t0* (v1 - v0);
					}
					if (PointInTriangle(V[0], U[0], U[1], U[2]) ){
                        if (!vec3_equal(p.start,v0))
                        {
						    p.end = v0;
						    intersects.push_back(p);
						    return true;
                        }
					}
					if (PointInTriangle(V[1], U[0], U[1], U[2]))
					{
                         if (!vec3_equal(p.start,v1))
                         {
                            p.end = v1;
                            intersects.push_back(p);
                            return true;
                         }
					}
					bFistPoint = true;
				}
		}
	}
	return false ;

}

template <typename P> 
static inline bool coplanar_tri_tri(const vec3<P>& N, const vec3<P>& v0, const vec3<P>& v1, const vec3<P>& v2, 
									const vec3<P>& u0, const vec3<P>& u1, const vec3<P>& u2, std::vector<Seg3D<P> >& intersects )
{
    P A[3];
    short i0,i1;
    /* first project onto an axis-aligned plane, that maximizes the area */
    /* of the triangles, compute indices: i0,i1. */
    A[0]=fabs(N.x);
    A[1]=fabs(N.y);
    A[2]=fabs(N.z);
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
	EdgeTriangleTest(v0, v1, u0, u1, u2, i0, i1, intersects);
	EdgeTriangleTest(v1, v2, u0, u1, u2, i0, i1, intersects);
	EdgeTriangleTest(v2, v0, u0, u1, u2, i0, i1, intersects);
	EdgeTriangleTest(u0, u1, v0, v1, v2, i0, i1, intersects);
	EdgeTriangleTest(u1, u2, v0, v1, v2, i0, i1, intersects);
	EdgeTriangleTest(u2, u0, v0, v1, v2, i0, i1, intersects);
	return (intersects.size() >0);
}

//static int coplanar_tri_tri(float N[3],float V0[3],float V1[3],float V2[3],
//    float U0[3],float U1[3],float U2[3])
//{
//    float A[3];
//    short i0,i1;
//    /* first project onto an axis-aligned plane, that maximizes the area */
//    /* of the triangles, compute indices: i0,i1. */
//    A[0]=fabs(N[0]);
//    A[1]=fabs(N[1]);
//    A[2]=fabs(N[2]);
//    if(A[0]>A[1])
//    {
//        if(A[0]>A[2])  
//        {
//            i0=1;      /* A[0] is greatest */
//            i1=2;
//        }
//        else
//        {
//            i0=0;      /* A[2] is greatest */
//            i1=1;
//        }
//    }
//    else   /* A[0]<=A[1] */
//    {
//        if(A[2]>A[1])
//        {
//            i0=0;      /* A[2] is greatest */
//            i1=1;                                           
//        }
//        else
//        {
//            i0=0;      /* A[1] is greatest */
//            i1=2;
//        }
//    }               
//
//    /* test all edges of triangle 1 against the edges of triangle 2 */
//    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);
//    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);
//    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);
//
//    /* finally, test if tri1 is totally contained in tri2 or vice versa */
//    POINT_IN_TRI(V0,U0,U1,U2);
//    POINT_IN_TRI(U0,V0,V1,V2);
//
//    return 0;
//}
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
		if (d[i] > EPSF)
			result[i] =1;
		else if (d[i] < -EPSF)
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
      *isect0  = vv0 + (vv1 - vv0)*tmp;
      vec3<P> diff = (v1-v0) * tmp;
      isectpoint0 = v0 + diff;

      tmp = d0 / (d0 - d2);
      *isect1 = vv0 + (vv2 - vv0)*tmp;
      diff =(v2-v0) * tmp;
      isectpoint1 = v0 + diff;
}

template< typename P> 
static inline int compute_intervals_isectline(const vec3<P>& v0, const vec3<P>& v1,  const vec3<P>& v2, 
						                      const P& vv0, const P& vv1, const P& vv2, P d0, P d1, P d2, 
											  P* isect0, P* isect1, vec3<P>& isectpoint0,   vec3<P>& isectpoint1)
{
	if ( d0*d1 > 0 )
		 isect2(v2, v0, v1, vv2, vv0, vv1, d2, d0, d1, isect0, isect1, isectpoint0, isectpoint1);
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
						 std::vector<Seg3D<P> >& intersects)
{
	/* compute plane equation of triangle(p0,p1,p2) */
     P  d1=-dot(nv, v0);
    /* plane equation 1: N1.X+d1=0 */
	/* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
	P du[3];
	du[0] = dot(nv,u0)+d1;
    du[1] = dot(nv,u1)+d1;
    du[2] = dot(nv,u2)+d1;
	int sdu[3];
	NormalDistToSign(du, sdu);
	if(sdu[0] == 0 && sdu[1] == 0 && sdu [2] ==0)
	{
 #ifdef CALCULATE_COPLANAR_INTERSECTIONS
            return coplanar_tri_tri(nv,v0,v1,v2,u0,u1,u2,intersects);     
#else
            return false;
#endif
	}
	if ((sdu[0] == sdu[1]) && (sdu[1] == sdu[2]))
		return false ; 

	P d2=-dot(nu, u0);
	P dv[3];
	dv[0] =dot(nu,v0)+d2;
    dv[1] = dot(nu,v1)+d2;
    dv[2] = dot(nu,v2)+d2;
	int sdv[3];
	NormalDistToSign(dv, sdv);
	if ((sdv[0] == sdv[1]) && (sdv[1] == sdv[2]))
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
    P vp0=v0[index];
    P vp1=v1[index];
    P vp2=v2[index];

    P up0=u0[index];
    P up1=u1[index];
    P up2=u2[index];

   /* compute interval for triangle 1 */
    P isect1[2];   
    vec3<P>  isectpointA1,isectpointA2;
    bool isCoplanar=compute_intervals_isectline(v0,v1,v2,vp0,vp1,vp2,dv[0],dv[1],dv[2],
                                           &isect1[0],&isect1[1],isectpointA1,isectpointA2);    
//	if(isCoplanar)
//    {
//#ifdef CALCULATE_COPLANAR_INTERSECTIONS
//            return coplanar_tri_tri(nv,v0,v1,v2,u0,u1,u2,intersects);     
//#else
//            return false;
//#endif
//     }
//
      /* compute interval for triangle 2 */
	 P isect2[2]; 
	 vec3<P>  isectpointB1,isectpointB2;
     isCoplanar=compute_intervals_isectline(u0,u1,u2,up0,up1,up2,du[0],du[1],du[2],
                                  &isect2[0],&isect2[1],isectpointB1,isectpointB2);
//     if(isCoplanar)
//     {
//#ifdef CALCULATE_COPLANAR_INTERSECTIONS
//            return coplanar_tri_tri(nv,v0,v1,v2,u0,u1,u2, intersects);     
//#else
//            return false;
//#endif
//      }
	  int smallest1 = sort2(isect1[0],isect1[1]);
      int smallest2 = sort2(isect2[0],isect2[1]);
      if(isect1[1]<isect2[0] || isect2[1]<isect1[0])
		  return false;
     /* at this point, we know that the triangles intersect */
	  Seg3D<P> seg;
      if(isect2[0]<isect1[0])
      {
		  seg.start = smallest1==0 ? isectpointA1:isectpointA2;
          
		  if(isect2[1]<isect1[1])
			   seg.end = smallest2==0? isectpointB2 : isectpointB1;
          else
			   seg.end = smallest1==0? isectpointA2 : isectpointA1;
        }
        else
        {
			seg.start = smallest2==0 ? isectpointB1:isectpointB2;
           
            if(isect2[1]>isect1[1])
				seg.end = smallest1 == 0 ? isectpointA2:isectpointA1;
            else
				seg.end = smallest2 == 0 ? isectpointB2:isectpointB1;
        }
		intersects.push_back(seg);
        return true;

}


#define FINDMINMAX(x0,x1,x2,min,max) \
    min = max = x0; \
    if(x1<min) min=x1;\
    if(x1>max) max=x1;\
    if(x2<min) min=x2;\
    if(x2>max) max=x2;


template <typename P> 
bool PlaneAABBIntersectTest(const vec3<P>& normal, const P& d, const Box3& bbox )
{
    vec3<P>  e = bbox.Diagonal()*0.5;
   
    P  r = dot(e, vec3<P>(fabs(normal.x), fabs(normal.y), fabs(normal.z)));
    P  s = dot (normal,  bbox.Center()) + d; 
    return  (fabs(s) <= (r)); 
}


template <typename P>
bool  planeBoxOverlap(vec3<P> normal, vec3<P> vert, const Box3& bbox)    // -NJMP-
{
   
  int q;
  vec3<P> vmin,vmax;
  P v;
  vec3<P>  e = bbox.Diagonal()*0.5;
  for(q=0;q<= 2;q++)
  {
    v=vert[q];                  // -NJMP-
    if(normal[q]>0.0f)
    {
      vmin[q]=-e[q] - v;   // -NJMP-
      vmax[q]= e[q] - v;   // -NJMP-
    }
    else
    {
      vmin[q]= e[q] - v;   // -NJMP-
      vmax[q]=-e[q] - v;   // -NJMP-
    }
  }
  if(dot(normal,vmin)>0.0f) return false ;   // -NJMP-
  if(dot (normal,vmax)>=0.0f) return true;  // -NJMP-
  
  return false ;
}



template<typename P>
bool TriangleAABBIntersectTest(const vec3<P>& v0, const vec3<P>& v1, const vec3<P>& v2, const Box3& bbox)
{
    vec3<P> c = bbox.Center();
    vec3<P> e = bbox.Diagonal()*0.5;
    vec3<P> v00 = v0- c;
    vec3<P> v10 = v1 -c ;
    vec3<P> v20 = v2 -c ; 
    //Compute edge vector 
    vec3<P> f0 =  v10 -v00;
    f0 = vec3<P>(fabs(f0.x), fabs(f0.y), fabs(f0.z));
    vec3<P> f1 =  v20 - v10;
    f1 = vec3<P>(fabs(f1.x), fabs(f1.y), fabs(f1.z));
    vec3<P> f2 = v00 - v20;
    f2 = vec3<P>(fabs(f2.x), fabs(f2.y), fabs(f2.z));
    //Test axes a00 edge-edge test 
    P p0 = v00.z * v10.y - v00.y * v10.z;
    P p2 = v20.z * (v10.y - v00.y) -v20.y * (v10.z - v00.z);
    P r  = e.y * f0.z + e.z * f0.y ; 
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 
    // Test axes a01 edge -edge 
    p0 = v10.z * v20.y - v10.y * v20.z;
    p2 = v00.z*(v20.y - v10.y) - v00.y *( v20.z - v10.z);
    r = e.y* f1.z + e.z * f1.y;
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 
    // Test axes a02 edge  (dot (v2, a02))
    p0  = v20.z *v00.y - v20.y * v00.z;
    p2 = v10.z *(v00.y -v20.y) - v10.y *(v00.z - v20.z );
    r = e.y *  f2.z  + e.z * f2.y; 
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 

   // test axes a10 edge - edge  
    p0 = v00.x* v10.z - v00.z * v10.x ; 
    p2 = v20.x *(v10.z - v00.z) - v20.z *(v10.x - v00.x);
    r = e.x * f0.z + e.z * f0.x ; 
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 
    p0 = v10.x * v20.z- v10.z * v20.x;
    p2 = v00.x*(v20.z - v10.z) - v00.z *( v20.x - v10.x);
     r = e.x * f1.z + e.z * f1.x ; 
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 
    p0  = v20.x *v00.z - v20.z * v00.x;
    p2 = v10.x *(v00.z -v20.z) - v10.z *(v00.x - v20.x);
    r = e.x * f2.z + e.z * f2.x ; 
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 

    // test axes a20 edge 
    p0 = v00.y* v10.x - v00.x * v10.y ; 
    p2 = v20.y *(v10.x - v00.x) - v20.x *(v10.y - v00.y);
    r = e.x * f0.y + e.y* f0.x ; 
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 
    p0 = v10.y * v20.x - v10.x * v20.y;
    p2 = v00.y*(v20.x - v10.x) - v00.x *( v20.y - v10.y);
    r = e.x * f1.y + e.y* f1.x ; 
    if ( max(- max (p0, p2), min(p0, p2)) > r )
         return false ; 
    p0  = v20.y *v00.x - v20.x * v00.y;
    p2 = v10.y *(v00.x -v20.x) - v10.x *(v00.y - v20.y);
    r = e.x * f2.y + e.y* f2.x ; 
    if ( max(- max (p0, p2), min(p0, p2)) > r  )
         return false ; 

    //   /* test in X-direction */
   P min, max ; 
   FINDMINMAX(v00.x, v10.x, v20.x ,min,max);
   if(min> e.x || max<-e.x ) return false;
   FINDMINMAX(v00.y, v10.y, v20.y ,min,max);
   if(min> e.y  || max<-e.y ) return false; 
   FINDMINMAX(v00.z, v10.z, v20.z ,min,max);
   if(min> e.z || max<-e.z ) return false;
   //test 
    vec3<P> normal = cross ((v10- v00), (v20 - v10));
    P       d = - dot (normal, v0);
    return  PlaneAABBIntersectTest(normal, d, bbox);
   //return planeBoxOverlap(normal, v00, bbox);
}


template<typename P> 
bool RayAABBIntersectTest(const vec3<P>& o, const vec3<P>& d, const Box3& bbox, P& tmin, P& tmax)
{
    tmin = 0.0;
    tmax = FLT_MAX;
    for (int i = 0; i < 3; i++)
    {
        if (fabs(d[i]) <EPSF)
        {
            if (o[i] < bbox.Min()[i] || o[i] > bbox.Max()[i])
                return false;
        }else {
            P ood = 1.0/ d[i];
            P t1 = (bbox.Min()[i] - o[i])* ood;
            P t2 = (bbox.Max()[i] - o[i])* ood;
            if (t1 > t2)
                std::swap(t1, t2);
            if (t1 > tmin) tmin = t1; 
            if (t2 > tmax) tmax = t2;
            if (tmin > tmax )
                 return false ; 

        }
    }
    return true;
}


template <typename P> 
bool RayTriangleIntersectTest(const vec3<P>& o, const vec3<P>& d, const vec3<P>& v0, const vec3<P>& v1,  const vec3<P>& v2,
                              P& u, P& v, P& t)
{
    vec3<P> e1 = v1- v0;
    vec3<P> e2 = v2 -v0;
    vec3<P> p = cross(d, e2);
    P      det = dot (p, e1);
    if (det > -EPSF && det < EPSF)
        return false ;
    vec3<P> f ; 
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
   vec3<P> q = cross(f, e1);

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

#endif