#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_conformer_2.h>
#include "configure.h"
#include "FixedPlaneMesh.h"
#include "BaseMesh.h"
#include "topology.h"
#include "adaptive.h"
#include "fixedBSPTree.h"


namespace GS{


static double3 denormalize(double3 &v, double3 &center, double3 &scale)
{
	return v /2 * scale + center;
}

static inline bool IsSimilar(const double3& a, const double3& b)
{
	return (fabs(a.x-b.x) < 1e-7 && 
		fabs(a.y-b.y) < 1e-7 && 
		fabs(a.z-b.z) < 1e-7);
}

/////////////////////////////////////////////////////////////////////
FixedPlaneMesh::FixedPlaneMesh(FixedBSPTree* pTree, const float4 &color):
	mColor(color)
{
    pTree->GetPolygons(mPolygons);
}
FixedPlaneMesh::FixedPlaneMesh(const Box3& bbox, const float4 &color):
	mAABB(bbox), mColor(color){}

void FixedPlaneMesh::AddPolygon(const Polygon& poly)
{
    if (poly.points.size() < 3)
        return ; 
    mAABB.IncludeBox(poly.bbox);
	mColor = poly.color;
    FixedPlanePolygon planePoly(poly);
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

void FixedPlaneMesh::AddPolygon(const FixedPlanePolygon& poly)
{
    mPolygons.push_back(poly);
}



 BaseMesh* FixedPlaneMesh::ToBaseMesh() 
 {
     std::vector<Point3D> pts; 
     ListOfvertices results;
     BaseMesh* pMesh = new BaseMesh;
     pMesh->SetTransformedAABB(AABB());
	//auto center = AABB().Center();
	//auto scale = AABB().Diagonal();

     for (int i = 0 ; i < mPolygons.size(); i++)
     {
        for (int j = 0 ; j < mPolygons[i].bplanes.size(); j++)
        {
            int prevPtIdx = j == 0? mPolygons[i].bplanes.size() -1 : j -1;
			Point3D pt = InexactComputePoint(mPolygons[i].splane, mPolygons[i].bplanes[prevPtIdx], mPolygons[i].bplanes[j] );
			//pt = denormalize(pt, center, scale);
			if (!pts.size() || 
				!IsSimilar(pt, pts.back())) 
				pts.push_back(pt);
        }
        auto normal = normalize(mPolygons[i].splane.Normal());
		TrianglatePolygon(normal, pts, results);
		//catch (...){
		//	wchar_t ch[64];
		//	swprintf(ch, 64, L"Error, %u", i);
		//	OutputDebugString(ch);
		//	results.clear();
		//}

        for (int k = 0 ; k < results.size(); k+=3)
        {   
            pMesh->Add(results[k], results[k+1], results[k+2], normal);
        }
        pts.clear();
		results.clear();
     }
     pMesh->GenID();
     return pMesh;
 }

 void FixedPlaneMesh::TrianglatePolygon(const double3& normal, std::vector<Point3D>& pts, ListOfvertices& results)
 {
    if (pts.size() < 3)
        return ; 
    if (pts.size() ==3)
    {
       VertexInfo vi1(pts[0], normal, mColor);
       VertexInfo vi2(pts[1], normal, mColor);
       VertexInfo vi3(pts[2], normal, mColor);
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
    vec3<double>  N = normal;
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
       VertexInfo vi1(p0, N, mColor);
       VertexInfo vi2(p1, N, mColor);
       VertexInfo vi3(p2, N, mColor);
       results.push_back(vi1);
       results.push_back(vi2);
       results.push_back(vi3);
   }
  
 }

 FixedBSPTree*  FixedPlaneMesh::ToBSPTree()
 {
     FixedBSPTree* bsp = new FixedBSPTree();
     bsp->BuildBSPTree(mPolygons);
     return bsp;
 }

 FixedPlaneMesh* ToFixedPlaneMesh(BaseMesh* mesh)
{
	assert(mesh->Implement()->m_bCoordNormalized);
    FixedPlaneMesh* result = 
		new FixedPlaneMesh(mesh->Implement()->m_TransformBBox, 
		mesh->Implement()->mColorTable[0]);

	int n = mesh->VertexCount();
	double3 *vert = new double3[n];

	for (int i = 0; i < n; i++)
		vert[i] = static_filter(mesh->Vertex(i).pos);

	auto &polygons = result->Ploygons();
	
	
	n = mesh->PrimitiveCount();
	for (int i = 0; i < n; i++)
	{
		auto &v1 = vert[mesh->TriangleInfo(i).VertexId[0]];
		auto &v2 = vert[mesh->TriangleInfo(i).VertexId[1]];
		auto &v3 = vert[mesh->TriangleInfo(i).VertexId[2]];
		FixedPlanePolygon poly(v1, v2, v3);
		polygons.push_back(poly);
	}
	delete [] vert;
	return result;
} 

} 