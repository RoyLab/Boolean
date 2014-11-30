#ifndef _LINE_SEGMENT_H_
#define _LINE_SEGMENT_H_
#include <math.h>
#include "typedefs.h"
#include "arithmetic.h"
#include "configure.h"
namespace GS{

enum LineRelation{
		Parallel, 
		Colinear, 
		EndIntersect, 
		InteralIntersect,
		NonIntersect
};

template < typename P = float> class LineSeg2D {

public:
	LineSeg2D(const vec2<P>& start, const vec2<P>& end, const vec2<P>& dir, P len)
		:mStart(start)
		,mEnd(end)
		,mDir(dir)
		,mLen(len)
	{
	}
	const vec2<P>& Start() const {return mStart;}
	const vec2<P>& End() const {return mEnd;}
	const vec2<P>& Dir() const {return mDir;}
	P              Length() const {return mLen;}
	LineRelation   IntersectTest(const LineSeg2D& lineSeg,  P& s, P& t, P DIR_EPS, P DIST_EPS ) const;   

private:
	vec2<P> mStart;
	vec2<P> mEnd;
	vec2<P> mDir; 
	P       mLen;
};

template <typename P>
LineRelation   LineSeg2D<P>::IntersectTest(const LineSeg2D<P>& lineSeg,  P& s, P& t, P DIR_EPS, P DIST_EPS ) const
{
	vec2<P> A = mEnd - mStart;
	vec2<P> B = lineSeg.mEnd - lineSeg.mStart; 
	P low, high;
	if (A.x < 0)
	{
		low = mEnd.x;
		high = mStart.x;
	}else {
		low  = mStart.x;
		high = mEnd.x;
	}
    if (B.x > 0)
	{
		if ((high < lineSeg.mStart.x) ||(lineSeg.mEnd.x < low)) 
			return NonIntersect ; 
	}else {
		if ((high < lineSeg.mEnd.x) || (lineSeg.mStart.x < low))
			return NonIntersect ;
	}
	if (A.y < 0)
	{
		low = mEnd.y;
		high = mStart.y;
	}else {
		low  = mStart.y;
		high = mEnd.y;
	}
	if (B.y > 0)
	{
		if ((high < lineSeg.mStart.y) ||(lineSeg.mEnd.y < low)) 
			return NonIntersect ; 
	}else {
		if ((high < lineSeg.mEnd.y) || (lineSeg.mStart.y < low))
			return NonIntersect ;
	}

	vec2<P> W = lineSeg.mStart - mStart;
	P f = cross(mDir, lineSeg.mDir);
	if (IsEqual(f, (P)0. , (P)DIR_EPS))// parallel
	{
		//parallel 
		if (!IsEqual(cross(A,W), (P)0., (P)DIST_EPS) || !IsEqual(cross(B,W), (P)0.,  (P)DIST_EPS))
			return Parallel;  // they are NOT collinear
        
		vec2<P> w2 = lineSeg.mEnd - mStart;
		 s = dot(W, mDir);
		 t = dot(w2, mDir);
         if (s > t)
			std::swap(s, t);

		//vec2<P> w2 = lineSet.mEnd - mstart;
  //      if (A.x != 0) {
  //               s = W.x / A.x;
  //               t = w2.x / A.x;
  //      }
  //      else {
  //               s = W.y / A.y;
  //               t = w2.y / A.y;
  //      }
  //      if (s > t) 
		//	std::swap(s,t);
  //      if (s > 1 || t < 0) {
  //          return NonIntersect;      // NO overlap
  //      }
  //      s = s<0? 0 : s;               // clip to min 0
  //      t = t >1? 1 : t;               // clip to max 1
		//if (!IsEqual(s, t, EPSF)    // intersect is a point
		//	onePoint = false ;
		//s*=mLen;
		//t*=mLen;
		return Colinear;
        
	}
	if (vec2_equal(Start(), lineSeg.Start(), DIST_EPS))
	{
		s = t = 0;
		return EndIntersect;
	}
	if (vec2_equal(Start(), lineSeg.End(), DIST_EPS)){
		s =0 ; 
		t = lineSeg.Length(); 
		return EndIntersect;
	} 
	if (vec2_equal(End(), lineSeg.Start(), DIST_EPS))
	{
		s= Length();
		t= 0; 
		return EndIntersect;
	}
	if (vec2_equal(End(), lineSeg.End(), DIST_EPS))
	{
		s= Length();
		t = lineSeg.Length();
		return EndIntersect;
	}
	s = cross(W, lineSeg.mDir)/f;
	if ((-DIST_EPS < s) && (s < Length() + DIST_EPS))
	{
		t = cross(W, mDir)/f;
		if ((-DIST_EPS < t) && (t < lineSeg.Length() + DIST_EPS)) 
				return InteralIntersect;
	}
	return NonIntersect;


	//	vec2<P> n1(-mDir.y, mDir.x);
	//	vec2<P> dist1 = lineSeg.Start() - Start();
	//	vec2<P> dist2 = lineSeg.End() -Start();
	//	P ProjLen = std::abs(dot(dist2, n1))- std::abs(dot( dist1, n1));
 //       if ((-DIST_EPS < ProjLen) && (ProjLen < DIST_EPS))  //colinear
	//	{
	//		s = dot(dist1, mDir);
 //           t = dot(dist2, mDir);
 //           if (s > t)
	//		 std::swap(s, t);
	//		return Colinear; 
	//	}
	//	return Parallel;
	//}
	//if (vec2_equal(Start(), lineSeg.Start(), DIST_EPS))
	//{
	//	s = t = 0;
	//	return EndIntersect;
	//}
	//if (vec2_equal(Start(), lineSeg.End(), DIST_EPS)){
	//	s =0 ; 
	//	t = 1; 
	//	return EndIntersect;
	//} 
	//if (vec2_equal(End(), lineSeg.Start(), DIST_EPS))
	//{
	//	s= 1;
	//	t= 0; 
	//	return EndIntersect;
	//}
	//if (vec2_equal(End(), lineSeg.End(), DIST_EPS))
	//{
	//	s=t=1;
	//	return EndIntersect;
	//}
	//vec2<P> n1(-Dir().y, Dir().x);
	//vec2<P> n2(-lineSeg.Dir().y, lineSeg.Dir().x);
	//s = (dot(lineSeg.Start(), n2) - dot(Start(), n2)) / dot(Dir(), n2);
 //   if ((-DIST_EPS < s) && (s < Length() + DIST_EPS))
	//{
	//	t = (dot(Start(), n1) - dot(lineSeg.Start(), n1)) / dot(lineSeg.Dir(), n1);
 //       if ((-DIST_EPS < t) && (t < lineSeg.Length() + DIST_EPS)) 
	//		return InteralIntersect;
	//}
	//return NonIntersect;
}


}

#endif 