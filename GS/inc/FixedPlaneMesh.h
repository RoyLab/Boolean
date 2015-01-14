#pragma once
#include <vector>
#include "typedefs.h"
#include "Box3.h"
#include "FixedPlane.h"
#include "Surface.h"
#include "FixedPolygon.h"

namespace GS{

class FixedBSPTree;
class BaseMesh;

class FixedPlaneMesh {
   
public:
    FixedPlaneMesh(): mColor(1.0f, 1.0f, 1.0f, 1.0f){}
    FixedPlaneMesh(const Box3& bbox, const float4 &color);
    FixedPlaneMesh(FixedBSPTree* pTree, const float4 &color);
	void AddPolygon(const Polygon& poly);
    void AddPolygon(const FixedPlanePolygon& poly);
    //void AddPolygon(const FixedPlane& plane);
    //void ClipPolygonByPlane(int polygonId, const FixedPlane& plane);
    int PrimitiveCount() const { return mPolygons.size();}
    std::vector<FixedPlanePolygon>& Ploygons()  {return mPolygons;}
    const Box3& AABB() const {return mAABB;}
	void SetAABB(const Box3& bbox) {mAABB = bbox;}
    BaseMesh* ToBaseMesh() ;
    FixedBSPTree*  ToBSPTree(); 
	const float4& Color() const {return mColor;}
protected : 
    void  CreatePolygon(const FixedPlane& splane,  const FixedPlane* bbox, FixedPlanePolygon& output);
     void TrianglatePolygon(const double3&, std::vector<Point3D>& pts, ListOfvertices& result);
private : 
    std::vector<FixedPlanePolygon>   mPolygons;
    std::vector<Box3>           mPolyAABBList;
    Box3                mAABB;
	float4				mColor;
};

FixedPlaneMesh* ToFixedPlaneMesh(BaseMesh* mesh);


}
