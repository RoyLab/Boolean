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
}
