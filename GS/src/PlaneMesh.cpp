#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_conformer_2.h>
#include "hashlookup.h"
#include "configure.h"
#include "PlaneMesh.h"
#include "BaseMesh.h"
#include "topology.h"


namespace GS{




/////////////////////////////////////////////////////////////////////
 PlaneMesh::PlaneMesh(const BaseMesh& mesh)
 {
     // generate  polygons 

 }


PlaneMesh::PlaneMesh(BSPTree* pTree)
{
    pTree->GetPolygons(mPolygons);
}
 
PlaneMesh::~PlaneMesh()
{
    mPolygons.clear();
    mAABB.Clear();
}

void PlaneMesh::AddPolygon(const Polygon& poly)
{
    if (poly.points.size() < 3)
        return ; 
    mAABB.IncludeBox(poly.bbox);
 
    PlanePolygon planePoly(poly);
    mPolygons.push_back(planePoly);
   
  /*  AddPolygon(splane, poly.color,  poly.bbox);
   
    int nPoints = poly.points.size();
    bool bCCW = poly.IsCCW();
    for (int j = 0; j< nPoints; j++)
    {
        vec3<double> edge = poly.points[(j+1)%nPoints] - poly.points[j];
        vec3<double> normal =  normalize( cross(poly.normal, edge));
        if (!bCCW)
        {
            normal.x = - normal.x;
            normal.y = - normal.y;
            normal.z = - normal.z;
        }
        Plane<double> bplane( normal, poly.points[j]);
        ClipPolygonByPlane(mPolygons.size()-1, bplane);
    }*/

}

void PlaneMesh::AddPolygon(const PlanePolygon& poly)
{
    mPolygons.push_back(poly);
}

void PlaneMesh::AddPolygon_Roy(const Plane<double>& plane, const float4& color, const Box3& bbox)
{
    PlanePolygon poly;
    poly.splane = plane ;
    poly.color = color;
    mPolygons.push_back(poly);
}

 void PlaneMesh::AddPolygon(const Plane<double>& plane, const float4& color, const Box3& bbox)
{
    Plane<double> xMin(vec3<double>(1, 0, 0),  bbox.Min());
    Plane<double> xMax(vec3<double>(-1, 0, 0),  bbox.Max());
    Plane<double> yMin(vec3<double>(0, 1, 0),  bbox.Min());
    Plane<double> yMax(vec3<double>(0, -1, 0),  bbox.Max());
    Plane<double> zMin(vec3<double>(0, 0, 1),  bbox.Min());
    Plane<double> zMax(vec3<double>(0, 0, -1),  bbox.Max());
    if ((plane.IsParallel(xMin) && plane.IsParallel(yMin)) || 
        (plane.IsParallel(xMin) && plane.IsParallel(zMin)) ||
        (plane.IsParallel(yMin) && plane.IsParallel(zMin)))
        assert (0);
   
    if (plane.IsParallel(xMin)){
        CreatePolygon(plane, yMin, yMax, zMin, zMax, color);
    }else if (plane.IsParallel(yMin)){
        CreatePolygon(plane, xMin, xMax, zMin, zMax, color);
    }else if (plane.IsParallel(zMin)) {
        CreatePolygon(plane, xMin, xMax, yMin, yMax, color);
    }else if (IsPointVaild(plane, xMin, yMin)){
        CreatePolygon(plane, xMin, xMax, yMin, yMax, color);
        ClipPolygonByPlane(mPolygons.size()-1, zMin);
        ClipPolygonByPlane(mPolygons.size()-1, zMax);
    }else if (IsPointVaild(plane, xMin, zMin)){
        CreatePolygon(plane, xMin, xMax, zMin, zMax, color);
        ClipPolygonByPlane(mPolygons.size()-1, yMin);
        ClipPolygonByPlane(mPolygons.size()-1, yMax);
    }else {
    
        CreatePolygon(plane, yMin, yMax, zMin, zMax, color);
        ClipPolygonByPlane(mPolygons.size()-1, xMin);
        ClipPolygonByPlane(mPolygons.size()-1, xMax);
    }
}

void PlaneMesh::ClipPolygonByPlane(int polygonId, const Plane<double>& plane)
{
    PlanePolygon front;
    RelationToPlane rp = mPolygons[polygonId].ClipByPlane(plane, front);
    if (rp == On)
    {
        if (!plane.IsSimilarlyOrientation(mPolygons[polygonId].splane))
            mPolygons.erase(mPolygons.begin()+ polygonId);
        return;
    }
    mPolygons[polygonId].bplanes = front.bplanes;
   
}

//void PlaneMesh::ClipPolygonByPlane(int polygonId, const Plane<double>& plane)
//{
//    //if (mPolygons[polygonId].splane.IsCoincidence(plane))
//    //{
//    //    if (!mPolygons[polygonId].splane.IsSimilarlyOrientation(plane))
//    //    {
//    //        mPolygons.erase(mPolygons.begin()+ polygonId);
//    //    }
//    //    return ;
//    //}
//    //std::vector<Plane<double> > bplanes;
//    //int nBPlanes = mPolygons[polygonId].bplanes.size();
//    //bool bPlaneInserted = false ; 
//    //for (int i = 0 ; i < mPolygons[polygonId].bplanes.size(); i++)
//    //{
//    //    int ppBpId = i  > 1 ? i -2 : (i+nBPlanes -2)% nBPlanes;
//    //    int pBpId = i > 0? i -1: nBPlanes -1;
//    //    int aBpId = i +1 >= nBPlanes? 0: i +1; 
//    //    // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
//    //    RelationToPlane  prevPtPos = plane.ClassifyPointToPlane(mPolygons[polygonId].splane, mPolygons[polygonId].bplanes[ppBpId], mPolygons[polygonId].bplanes[pBpId]);
//    //    RelationToPlane currentPtPos= plane.ClassifyPointToPlane(mPolygons[polygonId].splane, mPolygons[polygonId].bplanes[pBpId], mPolygons[polygonId].bplanes[i]);
//    //    RelationToPlane nextPtPos = plane.ClassifyPointToPlane(mPolygons[polygonId].splane, mPolygons[polygonId].bplanes[i], mPolygons[polygonId].bplanes[aBpId]);
//    //    OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
//    //    switch (signal)
//    //    {
//    //        case B:
//    //            bplanes.push_back(mPolygons[polygonId].bplanes[i]);
//    //            break;
//    //        case HB:
//    //            if (!bPlaneInserted)
//    //            {
//    //                bplanes.push_back(plane);
//    //                bPlaneInserted = true;
//    //            }
//    //            bplanes.push_back(mPolygons[polygonId].bplanes[i]);
//    //            break;
//    //        default:
//    //            break;
//    //    }
//    //}
//    //mPolygons[polygonId].bplanes = bplanes;
//}


void PlaneMesh::CreatePolygon(const Plane<double>& splane, const Plane<double>& xMinPlane, 
                      const Plane<double>& xMaxPlane, const Plane<double>& yMinPlane,
                      const Plane<double>& yMaxPlane, const float4& color )
{
    PlanePolygon poly;
    poly.splane = splane ;
    poly.color = color;
    poly.bplanes.push_back(yMinPlane);
    poly.bplanes.push_back(xMaxPlane);
    poly.bplanes.push_back(yMaxPlane);
    poly.bplanes.push_back(xMinPlane);
    mPolygons.push_back(poly);
}


//OutputSymbol   PlaneMesh::LookupEncodingTable(RelationToPlane prevPtPos, RelationToPlane currentPtPos, RelationToPlane nextPtPos)
//{
//    if (currentPtPos == Front)
//        return B;
//    if (currentPtPos == Behind)
//    {
//        if (nextPtPos == Front)
//            return HB;
//        return Empty;
//    }
//    if (nextPtPos ==Front)
//    {
//        if (prevPtPos == Front )
//            return B;
//        return  HB;
//    }else 
//        return Empty;
//}

 BaseMesh* PlaneMesh::ToBaseMesh() 
 {
     std::vector<Point3D > pts; 
     ListOfvertices results;
     BaseMesh* pMesh = new BaseMesh;
     for (int i = 0 ; i < mPolygons.size(); i++)
     {
        for (int j = 0 ; j < mPolygons[i].bplanes.size(); j++)
        {
            int prevPtIdx = j == 0? mPolygons[i].bplanes.size() -1 : j -1;
            Point3D pt = ComputePoint(mPolygons[i].splane, mPolygons[i].bplanes[prevPtIdx], mPolygons[i].bplanes[j] );
			//if (pt.x > 35 || pt.x < 24.8 || pt.y > 17.1 || pt.y < 7 || pt.z > 5.1 || pt.z < -5.1) assert(0);
            pts.push_back(pt);
        }

        TrianglatePolygon(mPolygons[i], pts, results);
        for (int k = 0 ; k < results.size(); k+=3)
        {   
            pMesh->Add(results[k], results[k+1], results[k+2], mPolygons[i].splane.Normal());
        }
        pts.clear();
		results.clear();
     }
     pMesh->GenID();
     pMesh->GenAABB(true);
     return pMesh;
 }

 void PlaneMesh::TrianglatePolygon(const PlanePolygon& poly, std::vector<Point3D>& pts, ListOfvertices& results)
 {
    if (pts.size() < 3)
        return ; 
    if (pts.size() ==3)
    {
       VertexInfo vi1(pts[0], poly.splane.Normal(), poly.color);
       VertexInfo vi2(pts[1], poly.splane.Normal(), poly.color);
       VertexInfo vi3(pts[2], poly.splane.Normal(), poly.color);
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

    vec3<double> origin = pts[0];
    vec3<double>  N = poly.splane.Normal();
    vec3<double>  U = normalize(pts[1] - origin);
    vec3<double>  V = cross(N, U);
    CDT cdt;
    CDT::Vertex_handle vh1, vh2, vh3;
    vec3<double> v0 = PosToLocal(U, V, N, origin, pts[0]);
    CDT::Point p0(v0.x, v0.y);
    vh1 = vh3 = cdt.insert(p0);
    for ( int i = 1; i< pts.size() ; i++)
    {
        vec3<double> v1 = PosToLocal(U, V, N, origin, pts[i]);
        CDT::Point p1(v1.x, v1.y);
        vh2 = cdt.insert(p1);
        cdt.insert_constraint(vh1, vh2);
        vh1 = vh2;
    }
    cdt.insert_constraint(vh2, vh3);
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

       VertexInfo vi1(p0, N, poly.color);
       VertexInfo vi2(p1, N, poly.color);
       VertexInfo vi3(p2, N, poly.color);
       results.push_back(vi1);
       results.push_back(vi2);
       results.push_back(vi3);
   }
  
 }



 BSPTree*  PlaneMesh::ToBSPTree()
 {
     BSPTree* bsp = new BSPTree();
     bsp->BuildBSPTree(mPolygons);
     return bsp;
 }




} 