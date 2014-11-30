#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_conformer_2.h>
#include "Polygon.h"
#include <iostream>
#include "topology.h"

namespace GS{

//////////////////////////////////////////////////////////////////////

 bool Polygon::IsCCW() const
 {
      vec3<double> U = normalize( points[1] - points[0]);
      vec3<double> V  = cross(normal, U);
      
      std::vector<vec2<double> > vertices(points.size());
      vertices[0] = 0.0; 
      for (int i = 1 ; i < points.size(); i++)
        vertices[i] = PosToLocal(U, V, normal, points[0], points[i]).xy;
     
      //double x = fabs(normal.x);
      //double y = fabs (normal.y);
      //double z = fabs (normal.z);
      //int ProjFlag = 0;  // project to XY plane 
      //if ( x >= y && x >= z) // project to YZ plane 
      //     ProjFlag = 1; 
      //else if ( y >= x && y >= z ) // project to XZ plane 
      //    ProjFlag = 2;

      //std::vector<vec2<double> > vertices(points.size());
      //for (int i = 0 ; i < points.size(); i++)
      //{
      //    if ( ProjFlag == 0 )  
      //          vertices[i] = points[i].xy;
      //    else if (ProjFlag ==1 )
      //        vertices[i]= points[i].yz;
      //    else 
      //        vertices[i] = points[i].xz;
      //}
       // find y max 
    int k = 0 ; 
    double ymax = vertices[0].y;
    int n = vertices.size();
    for (int i = 1; i < vertices.size() ; i++)
    {
        if ((vertices[i].y - ymax) > EPSF)
        {
            k = i; 
            ymax = vertices[i].y;
        }
    }

    int prevk = (k -1 + n )% n;
    int nextk = (k+1 +n)% n;
    double s = cross(vertices[k] - vertices[prevk], vertices[k] - vertices[nextk]);
   // vec2<Precision> p = (vertices[prevk] + vertices[nextk])*0.5;
    if (IsEqual(s , (double)0., (double)EPSF))
    {
        vec2<double> base(0., 1.0);
        vec2<double>  p = vertices[k] - length(vertices[k] - vertices[prevk]) *base;
        s = cross(p - vertices[prevk], p - vertices[nextk]);
    } 
   
     if ( s >= EPSF) 
        return false ;
    return true;
 }


RelationToPlane  Polygon::ClassifyPloygonToPlane(const Plane<double>& plane) const
{

    int numInFront = 0 ; 
    int numInBack = 0; 
    
    for (int i = 0; i < points.size(); i++)
    {
         RelationToPlane PtToPlane= plane.ClassifyPointToPlane(points[i]);
         if (PtToPlane == Front)
             numInFront++;
         else if (PtToPlane == Behind)
             numInBack ++;
    }
    if (numInFront != 0 && numInBack != 0)
        return Straddling;
    if (numInFront != 0)
        return Front;
    if (numInBack != 0)
        return Behind;
    return On;

}

void Polygon::Clear()
{
    points.clear();
}

bool Polygon::IntersectWithBox(const Box3& bbox) const 
{
    if (!bbox.Intersects(bbox))
        return false ; 
    int numInside = 0;
    int numOutside = 0; 
    for (int i = 0 ; i < points.size(); i++)
    {
        if (bbox.IsInBox(points[i].x, points[i].y, points[i].z))
             numInside ++;
        else 
            numOutside ++;
    }
    if (numInside != 0 && numOutside != 0)
        return true;
    return false ;
}

//// only output the polygon in the front of  clipping plane
RelationToPlane Polygon::ClipByPlane(const Plane<double>& bp, Polygon& front)
{
    assert(points.size() > 2);
    Plane<double> sp(normal, points[0]);
    if (bp.IsCoincidence(sp))
        return On;
    front.points.clear();
    int prevIndex = points.size() - 1;
    RelationToPlane aSide = bp.ClassifyPointToPlane(points[prevIndex]);
    vec3<double> intersect; 
    for (int i = 0 ; i < points.size(); i++)
    {
          RelationToPlane bSide = bp.ClassifyPointToPlane(points[i]);
          if (bSide == Front )
          {
              if (aSide == Behind )
              {
                  
                  IntersectSegmentWithPlane(points[prevIndex], points[i], bp.Normal(), bp.Distance(), intersect);
                  front.points.push_back(intersect);
              }
              front.points.push_back(points[i]);
          }else if (bSide == Behind) {
              if (aSide == Front )
              {
                   IntersectSegmentWithPlane(points[prevIndex], points[i], bp.Normal(), bp.Distance(), intersect);
                   front.points.push_back(intersect);
              }
                   
          }else
                front.points.push_back(points[i]);
          aSide = bSide ;
          prevIndex = i; 
    }
    return Front;
}


RelationToPlane Polygon::ClipByPlane(const Plane<double>& bp, Polygon& front, Polygon& back)
{
    int numFront  = 0, numBack = 0; 
    assert(points.size() > 2);
    Plane<double> sp(normal, points[0]);
    if (bp.IsCoincidence(sp))
        return On;

    int prevIndex = points.size() - 1;
    RelationToPlane aSide = bp.ClassifyPointToPlane(points[prevIndex]);
    vec3<double> intersect; 
    for (int i = 0; i < points.size(); i++)
    {
        RelationToPlane bSide = bp.ClassifyPointToPlane(points[i]);
        if (bSide == Front )
        {
            if (aSide == Behind)
            {
                 IntersectSegmentWithPlane(points[prevIndex], points[i], bp.Normal(), bp.Distance(), intersect);
                 front.points.push_back(intersect);
                 back.points.push_back(intersect);
            }
             front.points.push_back(points[i]);
        }else if (bSide == Behind) {
              if (aSide == Front )
              {
                   IntersectSegmentWithPlane(points[prevIndex], points[i], bp.Normal(), bp.Distance(), intersect);
                   front.points.push_back(intersect);
                   back.points.push_back(intersect);
              }else if (aSide == On)
                  back.points.push_back(points[prevIndex]);
              back.points.push_back(points[i]);
                   
        }else {
            
              front.points.push_back(points[i]);
              if (aSide == Behind)
                  back.points.push_back(points[i]);
        }
        prevIndex = i;
        aSide = bSide;
    }
    RelationToPlane rp = On; 
    if (front.points.size() && back.points.size())
    {
          assert(front.points.size() >=3);
           assert(back.points.size() >=3);
        rp = Straddling;
        front.color = color;
        back.color = color;
        front.normal = normal;
        back.normal = normal;
        front.GenAABB();
        back.GenAABB();
    }
    else if (front.points.size() )
    {
        assert(front.points.size() >=3);
        front.normal = normal;
        rp = Front;
        front.bbox = bbox;
    }
    else if (back.points.size())
    {
         assert(back.points.size() >=3);
         back.bbox = bbox;
         back.color = color;
         back.normal = normal;
        rp = Behind;
    }
    return rp;
}

void Polygon::GenAABB()
{
    bbox.Clear();
    for (int i = 0; i < points.size(); i++)
    {
        bbox.IncludePoint(points[i]);
    }
}


bool Polygon::IsConvex() const 
{
    assert (points.size() >=3);
    if (points.size() == 3)
        return true; 
    if (points.size() == 4)
    {
        double3 db = points[3] - points[1]; 
        double3 ab = points[0] - points[1];
        double3 cb = points[2] - points[1];
        if (dot (cross (db, ab), cross(db, cb)) >= EPSF)
            return false ; 
        double3 ca = points[2] - points[0];
        double3 da = points[3] - points[0];
        double3 ba = points[1] - points[0];
        return dot(cross (ca, da), cross(ca, ba)) < EPSF;
    }
    double3 lastEdge =  points[0] - points[points.size() -1];
    for (int i=0; i<points.size(); i++) 
    {
        double3 edge = points[(i+1) % points.size()] - points[i];   
        if (dot(normal, cross(lastEdge,edge)) <= EPSF) 
            return false ; 
        lastEdge = edge;
    }
    return true;
}


void Polygon::TriangulatePloygon(std::vector<int>& tri) const 
{
    if (points.size() < 3)
        return; 
    tri.resize((points.size() -2)*3);
    if (points.size() ==3)
    {
       
       tri[0] = 0;
       tri[1] = 1;
       tri[2] = 2;
       return ;
    }

     double3 faceNormal(0.0f, 0.0f, 0.0f);
   
    double3 lastEdge = points[1] - points[0];
    int i;
    for (i=2; i< points.size(); i++)
    {
           
        double3  diff = points[i] - points[0];
        faceNormal += cross(lastEdge,diff);
        lastEdge = diff;
    }
    faceNormal = normalize(faceNormal);
    if (points.size() == 4)
    {
        int diagStart = -1;
        lastEdge = points[0] -  points[3];
          
        for (int i=0; i<4; i++)
        {
            double3 edge = points[(i+1) % points.size()] - points[i];   
            if (dot(faceNormal, cross(lastEdge,edge)) <= 0.0) 
            {
                // Nonconvex corner - use it in the diagonal.
                diagStart = i;
                break;
            }
            lastEdge = edge;
        }
        if (diagStart < 0)
        {
            double3 diff0 = points[2] - points[0];
            double3 diff1 = points[3] - points[1];
            if (dot(diff0,diff0)*.99f < dot(diff1,diff1))
                diagStart = 0;
            else diagStart = 1;
        }

        if (diagStart%2==1)
        {
            tri[0] = 0;
            tri[1] = 1;
            tri[2] = 3;
            tri[3] = 1;
            tri[4] = 2;
            tri[5] = 3;
        }
        else
        {
            tri[0] = 0;
            tri[1] = 1;
            tri[2] = 2;
            tri[3] = 0;
            tri[4] = 2;
            tri[5] = 3;
        }
        return; 
    }
    lastEdge =  points[0] - points[points.size() -1]; 
    for (i=0; i< points.size() ; i++) 
    {
        double3 edge = points[(i+1) % points.size()] - points[i];   
        if (dot(faceNormal, cross(lastEdge,edge)) <= 0.0) break;
        lastEdge = edge;
    }
    if (i==points.size())
         TriangulateConvexPolygon(tri);
    else 
        TriangulateConcavePolygon(tri);
}


void Polygon::TriangulateConvexPolygon(std::vector<int>& tri) const 
{
    int*  next = new int[points.size()];
    for (int i=1; i<points.size(); i++) 
        next[i-1] = i;
    next[points.size()-1] = 0;

    int triangleCount = points.size()-2;
    int curr = 0;
    int tpos=0;
    for (int i=0; i<triangleCount-2; i++, tpos+=3)
    {
        int ncur = next[curr];
        int nncur = next[ncur];
        tri[tpos] = curr;
        tri[tpos+1] = ncur;
        tri[tpos+2] = nncur;

        // Cut out the middle of this triangle from the remaining polygon:
        next[curr] = nncur;
        // And go to the end of it.
        curr = nncur;
    }

    int pb = next[curr];
    int pc = next[pb];
    int pd = next[pc];

    const double3& va = points[curr];
    const double3& vb = points[pb];
    const double3& vc = points[pc];
    const double3& vd = points[pd];

    float3 d0 = va - vc;
    float3 d1 = vb - vd;
    if (dot(d0,d0)*.99f < dot(d1,d1))
    {
        tri[tpos] =   curr;
        tri[tpos+1] = pb;
        tri[tpos+2] = pc;
        tri[tpos+3] = curr;
        tri[tpos+4] = pc;
        tri[tpos+5] = pd;
            
    }
    else
    {     
        tri[tpos] = curr;
        tri[tpos+1] = pb;
        tri[tpos+2] = pd;
        tri[tpos+3] = pb;
        tri[tpos+4] = pc;
        tri[tpos+5] = pd;
    }
    delete[] next;
}

void Polygon::TriangulateConcavePolygon(std::vector<int>& tri) const
{
    int*  next = new int[points.size()];
    for (int i=1; i<points.size(); i++) 
        next[i-1] = i;
    next[points.size()-1] = 0;

    int numRemaining = points.size();
    int curr = 0, npos=1, nnpos=2;  
    int tpos=0;

    double3 pt[3];
    double3 edgeNormal[3];
    double edgeOffset[3];

    int i ; 
    while (numRemaining > 3)
    {
        for (int pass=0; pass<3; pass++) 
        {
            float epsilon = (1-pass)* EPSF;

            for (i=0; i<numRemaining; i++, curr=npos)
            {
                npos = next[curr];
                nnpos = next[npos];

                pt[0] = points[curr];
                pt[1] = points[npos];
                pt[2] = points[nnpos];
            

                // Check if this corner's convex:
                double3 cornerCP = cross (pt[1] - pt[0], pt[2] - pt[0]);
                if (dot (normal,cornerCP) < epsilon) continue;   // not convex.

                // Use "edge-normals", which point into the face from each edge,
                // to test whether other points are in triangle.
                int k;
                for (k=0; k<3; k++) 
                {
                    double3 edgeDir = pt[(k+1)%3] - pt[k];
                    float dp = dot (normal, edgeDir);
                    if (dp!=0.0f) edgeDir -= dp*normal; // Project into plane.
                    edgeDir = normalize(edgeDir);
                    edgeNormal[k] = cross (normal,edgeDir); // inward-pointing edge-normal in plane.
                    edgeOffset[k] = dot(edgeNormal[k], pt[k]);
                }

                // Now a point is inside the triangle if it's on the positive side of these three planes.

                // Are any points inside triangle?
                int j;
                for (j=next[nnpos]; j!=curr; j=next[j]) 
                {
                    for (k=0; k<3; k++) 
                    {
                        if ( (dot (points[j], edgeNormal[k]) - edgeOffset[k]) < -epsilon) break;
                    }
                    if (k==3) break;    // point is on inside of all three edges.
                }
                if (j!=curr) continue; // No good - some other part of mesh intersects this tri.
                // Otherwise, we win -- chop off this corner.
                break;
            }
            if (i<numRemaining) break;
        }

        // Split off this corner as a face.
        tri[tpos] = curr;
        tri[tpos+1] = npos;
        tri[tpos+2] = nnpos;
        tpos += 3;

        // Remove the corner from the polygon:
        next[curr] = nnpos;
        numRemaining--;
        curr = nnpos;
    }

    // Only 3 uneliminated verts left -- we're done.
    npos = next[curr];
    nnpos = next[npos];

    tri[tpos] =  curr;
    tri[tpos+1] = npos;
    tri[tpos+2] = nnpos;
    delete[] next; 


}

void Polygon::Trianglate(ListOfvertices& results) const
{
    std::vector<int> tri; 
    TriangulatePloygon(tri);
    if (tri.size() == 0)
        return ; 
    results.reserve(tri.size());
    for (int i = 0 ; i < tri.size(); i++)
    {
       VertexInfo vi(points[tri[i]], normal,  color);
       results.push_back(vi);
    }
    tri.clear();
    std::vector<int>().swap(tri);
}
 
void Polygon::Trianglate1(ListOfvertices& results) const
{
    if (points.size() < 3)
        return; 
    if (points.size() ==3)
    {
       VertexInfo vi1(points[0], normal,  color);
       VertexInfo vi2(points[1], normal,  color);
       VertexInfo vi3(points[2], normal,  color);
       results.push_back(vi1);
       results.push_back(vi2);
       results.push_back(vi3);
       return ;
    }
    typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

    typedef CGAL::Triangulation_vertex_base_2<K>                     Vb;
    typedef CGAL::Constrained_triangulation_face_base_2<K>           Fb;
    typedef CGAL::Triangulation_data_structure_2<Vb,Fb>              TDS;
     //typedef CGAL::Exact_predicates_tag                               Itag;
    typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, CGAL::No_intersection_tag> CDT;

    vec3<double> origin = points[0];
    const vec3<double>&  N = normal;
    vec3<double>  U = normalize(points[1] - origin);
    vec3<double>  V = cross(N, U);
    CDT cdt;
    CDT::Vertex_handle vh1, vh2, vh3;
    vec3<double> v0 = PosToLocal(U, V, N, origin, points[0]);
    CDT::Point p0(v0.x, v0.y);
    vh1 = vh3 = cdt.insert(p0);
    for ( int i = 1; i< points.size() ; i++)
    {
        vec3<double> v1 = PosToLocal(U, V, N, origin, points[i]);
        CDT::Point p1(v1.x, v1.y);
        vh2 = cdt.insert(p1);
        //cdt.insert_constraint(vh1, vh2);
        //vh1 = vh2;
    }
    //cdt.insert_constraint(vh2, vh3);
    int count = cdt.number_of_faces() ; 
    results.reserve(count*3);
    for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin();
       fit != cdt.finite_faces_end(); ++fit)
   {
	   vec2<double> v0(fit->vertex(2)->point().x(),fit->vertex(2)->point().y() );
	   vec2<double> v1(fit->vertex(1)->point().x(),fit->vertex(1)->point().y() );
	   vec2<double> v2(fit->vertex(0)->point().x(),fit->vertex(0)->point().y() );   
	   if (IsEqual(cross(v0- v2, v1-v2), (double)0.,  (double)EPSF ))
		   continue; //
       vec3<double > p0(v0, 0.0);
       vec3<double > p1(v1, 0.0);
       vec3<double > p2(v2, 0.0);
       p0 = PosToGlobal(U, V, N, origin, p0);
       p1 = PosToGlobal(U, V, N, origin, p1);
       p2 = PosToGlobal(U, V, N, origin, p2);
   
         VertexInfo vi1(p0, N, color);
         VertexInfo vi2(p1, N, color);
         VertexInfo vi3(p2, N, color);

       results.push_back(vi1);
       results.push_back(vi2);
       results.push_back(vi3);
   }

}

/////////////////////////////////////////////////////////////////////

PlanePolygon::PlanePolygon(const Polygon& poly)
    :splane(poly.normal, poly.points[0])
    , color(poly.color)
{
   int nPoints = poly.points.size();
    bool bCCW = poly.IsCCW();
    for (int j = 0; j< nPoints; j++)
    {
        vec3<double> edge = poly.points[(j+1)%nPoints] - poly.points[j];
        vec3<double> normal =  cross(poly.normal, edge);//normalize( cross(poly.normal, edge));
        if (!bCCW)
        {
            normal.x = - normal.x;
            normal.y = - normal.y;
            normal.z = - normal.z;
        }
        Plane<double> bplane( normal, poly.points[j]);
        bplanes.push_back(bplane);
    }


}

std::ostream& operator<<(std::ostream& out, const PlanePolygon& poly)
{
	out << "splane: \n";
	out << poly.splane;
	out << "bplane: \n";
	for (int i = 0; i < poly.bplanes.size(); i++)
	{
		out << poly.bplanes[i];
	}
	return out;
}

RelationToPlane PlanePolygon::ClassifyPloygonToPlane(const Plane<double>& plane) const
{
    int numInFront = 0 ; 
    int numInBack = 0; 
    
    for (int i = 0; i < bplanes.size(); i++)
    {
         int pBpId = i > 0? i -1: bplanes.size() -1;
         RelationToPlane PtToPlane= plane.ClassifyPointToPlane(splane, bplanes[pBpId], bplanes[i]);
         if (PtToPlane == Front)
             numInFront++;
         else if (PtToPlane == Behind)
             numInBack ++;
    }
    if (numInFront != 0 && numInBack != 0)
        return Straddling;
    if (numInFront != 0)
        return Front;
    if (numInBack != 0)
        return Behind;
    return On;

}


RelationToPlane PlanePolygon::ClassifyEdgeToPlane(const Plane<double>& plane, int idx) const
{
     int nSize = bplanes.size();
     int pBpId = idx > 0? idx -1: nSize -1;
     int aBpId = idx +1 >= nSize? 0: idx +1; 
     // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
     RelationToPlane currentPtPos= plane.ClassifyPointToPlane(splane, bplanes[pBpId], bplanes[idx]);
     RelationToPlane nextPtPos = plane.ClassifyPointToPlane(splane,   bplanes[idx], bplanes[aBpId]);
     if (nextPtPos == On)
         return currentPtPos;
     if (currentPtPos == On)
         return nextPtPos;
     if (currentPtPos != nextPtPos)
         return Straddling;
     return currentPtPos;    
}


// Clip polygon and only output the polygon in the front of plane 
RelationToPlane PlanePolygon::ClipByPlane(const Plane<double>& bp, PlanePolygon& front) const
{

   if (splane.IsCoincidence(bp))
        return On;
   RelationToPlane rp = bp.ClassifyPlaneToPlane(splane);
   if (rp == Front)
   {
       front = *this;
        return Front; 
   }
   else  if (rp == Behind)
       return Behind;
   else if (rp == On)
       return On; 
    std::map<Plane<double>, bool, PlaneCompare<double> > PlaneMap;
    int nBPlanes =  bplanes.size();
    bool bPlaneInserted = false ; 
    int ppBpId =nBPlanes -2;
    int pBpId = ppBpId +1;
    // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
    RelationToPlane  prevPtPos = bp.ClassifyPointToPlane( splane, bplanes[ppBpId], bplanes[pBpId]);
    RelationToPlane currentPtPos= bp.ClassifyPointToPlane(splane, bplanes[pBpId],  bplanes[0]);
    for (int i = 0 ; i <  bplanes.size(); i++)
    {
        int aBpId = i +1 >= nBPlanes? 0: i +1; 
        RelationToPlane nextPtPos = bp.ClassifyPointToPlane(splane, bplanes[i], bplanes[aBpId]);
        OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
        switch (signal)
        {
            case B:
                front.InsertBPlane(bplanes[i],PlaneMap) ;
                break;
            case HB:
                front.InsertBPlane(bp,PlaneMap) ;
                front.InsertBPlane(bplanes[i],PlaneMap) ;
                break;
           case PBB:
                 if (!bp.IsSimilarlyOrientation(splane))
                    break;
             case PB:
                front.InsertBPlane(bplanes[pBpId],PlaneMap) ;
                front.InsertBPlane(bplanes[i],PlaneMap) ;
            default:
                break;
        }
        prevPtPos = currentPtPos;
        currentPtPos = nextPtPos;
        pBpId = i;


    }
    PlaneMap.clear();
      assert(front.bplanes.size() >=3);
    front.splane = splane;
    front.color = color;
    return Front;

    // if (splane.IsCoincidence(bp))
    //    return On;

    //int nBPlanes =  bplanes.size();
    //bool bPlaneInserted = false ; 
    //for (int i = 0 ; i <  bplanes.size(); i++)
    //{
    //    int ppBpId = i  > 1 ? i -2 : (i+nBPlanes -2)% nBPlanes;
    //    int pBpId = i > 0? i -1: nBPlanes -1;
    //    int aBpId = i +1 >= nBPlanes? 0: i +1; 
    //    // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
    //    RelationToPlane  prevPtPos = bp.ClassifyPointToPlane( splane, bplanes[ppBpId], bplanes[pBpId]);
    //    RelationToPlane currentPtPos= bp.ClassifyPointToPlane(splane, bplanes[pBpId],  bplanes[i]);
    //    RelationToPlane nextPtPos = bp.ClassifyPointToPlane(splane, bplanes[i], bplanes[aBpId]);
    //    OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
    //    switch (signal)
    //    {
    //        case B:
    //            front.bplanes.push_back(bplanes[i]);
    //            break;
    //        case HB:
    //            if (!bPlaneInserted)
    //            {
    //                front.bplanes.push_back(bp);
    //                bPlaneInserted = true;
    //            }
    //            front.bplanes.push_back(bplanes[i]);
    //            break;
    //        default:
    //            break;
    //    }
    //}
    //  assert(front.bplanes.size() >=3);
    //front.splane = splane;
    //front.color = color;
    //return Front;

}


RelationToPlane PlanePolygon::ClipByPlane(const Plane<double>& bp, PlanePolygon& front, PlanePolygon& back) const
{
     if (splane.IsCoincidence(bp))
        return On;
   RelationToPlane rp = bp.ClassifyPlaneToPlane(splane);
   if (rp == Front )
   {
       front = *this;
        return Front; 
   }
   else  if (rp == Behind)
   {
       back = *this;
       return Behind;
   } else if ( rp == On)
       return On; 
    int nBPlanes =  bplanes.size();
    bool bFrontInserted = false ; 
    bool bBackInserted = false ; 

    int ppBpId =nBPlanes -2;
    int pBpId = ppBpId +1;
    std::map<Plane<double>, bool, PlaneCompare<double> > FrontPlaneMap;
    std::map<Plane<double>, bool, PlaneCompare<double> > BackPlaneMap;
    // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
    RelationToPlane  prevPtPos = bp.ClassifyPointToPlane( splane, bplanes[ppBpId], bplanes[pBpId]);
    RelationToPlane currentPtPos= bp.ClassifyPointToPlane(splane, bplanes[pBpId],  bplanes[0]);
    for (int i = 0 ; i <  bplanes.size(); i++)
    {
   
        int aBpId = i +1 >= nBPlanes? 0: i +1; 
        // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
        RelationToPlane nextPtPos = bp.ClassifyPointToPlane(splane, bplanes[i], bplanes[aBpId]);

        OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
        switch (signal)
        {
            case B:
                front.InsertBPlane(bplanes[i],FrontPlaneMap) ;
                break;
            case HB:
                front.InsertBPlane(bp,FrontPlaneMap) ;
                front.InsertBPlane(bplanes[i],FrontPlaneMap) ;
                break;
            case PBB:
                 if (!bp.IsSimilarlyOrientation(splane))
                    break;
             case PB:
                front.InsertBPlane(bplanes[pBpId],FrontPlaneMap) ;
                front.InsertBPlane(bplanes[i],FrontPlaneMap) ;

            default:
                break;
        }
       
        // output back plane
        OutputSymbol backSignal =  LookupEncodingTable(ReverseRelation(prevPtPos), ReverseRelation(currentPtPos), ReverseRelation(nextPtPos));
        switch (backSignal)
        {
            case B:
                back.InsertBPlane(bplanes[i],BackPlaneMap) ;
                break;
            case HB:
                {
                Plane<double> hp (-bp.Normal(), -bp.Distance());
                back.InsertBPlane(hp,BackPlaneMap) ;
                back.InsertBPlane(bplanes[i],BackPlaneMap) ;
                break;
                }
            case PBB:
                if (bp.IsSimilarlyOrientation(splane))
                    break;
             case PB:
               
                    back.InsertBPlane(bplanes[pBpId],BackPlaneMap) ;
                    back.InsertBPlane(bplanes[i],BackPlaneMap);

            default:
                break;
        }
        prevPtPos = currentPtPos;
        currentPtPos = nextPtPos;
        pBpId = i;
    }
    FrontPlaneMap.clear();
    BackPlaneMap.clear();
    rp = On; 
    if (front.bplanes.size() )
    {
        int frontsize = front.bplanes.size();
        assert(frontsize >=3);
        front.splane = splane;
        front.color = color;
        rp = Front;
    }
    if (back.bplanes.size())
    {
        int backsize = back.bplanes.size();
         assert(backsize >=3);
        back.splane = splane;
        back.color = color;
        rp = Behind;
    }
    if (front.bplanes.size() && back.bplanes.size() )
        return Straddling;
    return rp;

}

OutputSymbol   PlanePolygon::LookupEncodingTable(RelationToPlane prevPtPos, RelationToPlane currentPtPos, RelationToPlane nextPtPos) const
{
    if (currentPtPos == Front)
        return B;

    if (currentPtPos == Behind)
    {
        if (nextPtPos == Front)
            return HB;
        return Empty;
    } 
    if ( currentPtPos == On &&
         prevPtPos == On )
    {
        if ( nextPtPos == On)
            return PBB;
        else if (nextPtPos == Front)
            return PB;
    }
    if (nextPtPos ==Front)
    {
        if (prevPtPos == Front )
            return B;
        return  HB;
    }else 
        return Empty;
}


RelationToPlane PlanePolygon::ReverseRelation(RelationToPlane relation) const
{
    switch (relation)
    {
        case Front: 
             return Behind;
        case Behind:
             return Front;
        default:
            return relation;
    }

}

void PlanePolygon::Negate()
{
	splane.Negate();
	auto copy = bplanes;
	bplanes.clear();
	int n = copy.size();
	for (int i = 0; i < n; i++)
	{
		bplanes.push_back(copy.back());
		copy.pop_back();
	}
}


void PlanePolygon::InsertBPlane(const Plane<double>& bp, std::map<Plane<double>, bool, PlaneCompare<double> >& PlaneMap)
{
    if (PlaneMap.find(bp) == PlaneMap.end())
    {
        bplanes.push_back(bp);
        PlaneMap[bp] =true;
    }

}


}