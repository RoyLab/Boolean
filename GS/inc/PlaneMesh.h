#ifndef _PLANE_MESH_H
#define _PLANE_MESH_H
#include <vector>
#include "typedefs.h"
#include "Box3.h"
#include "Plane.h"
#include "Surface.h"
#include "Polygon.h"
#include "BSPTree.h"

namespace GS{

class BaseMesh;

class PlaneMesh {
   
public:
    PlaneMesh(const Box3& bbox)
        :mAABB(bbox){}
    PlaneMesh(const BaseMesh& );
    PlaneMesh(BSPTree* pTree);
    ~PlaneMesh();
    void AddPolygon(const Polygon& poly);
    void AddPolygon(const PlanePolygon& poly);
    void AddPolygon(const Plane<double>& plane, const float4& color, const Box3& bbox);
    void AddPolygon_Roy(const Plane<double>& plane, const float4& color, const Box3& bbox);
    void ClipPolygonByPlane(int polygonId, const Plane<double>& plane);
    int PrimitiveCount() const { return mPolygons.size();}
    std::vector<PlanePolygon>& Ploygons()  {return mPolygons; }
    const Box3& AABB() const {return mAABB;}
    BaseMesh* ToBaseMesh() ;
    BSPTree*  ToBSPTree(); 
protected : 
    void  CreatePolygon(const Plane<double>& splane,    const Plane<double>& xMinPlane, 
                        const Plane<double>& xMaxPlane, const Plane<double>& yMinPlane,
                        const Plane<double>& yMaxPlane, const float4& color);
     void PlaneMesh::TrianglatePolygon(const PlanePolygon& poly, std::vector<Point3D>& pts, ListOfvertices& result);
private : 
    std::vector<PlanePolygon>   mPolygons;
    std::vector<Box3>           mPolyAABBList;
//    std::vector<Plane<double> > mBoundPlanes;
    Box3                mAABB;
  
};



}


#endif 