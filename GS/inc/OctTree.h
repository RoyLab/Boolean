#ifndef _OCT_TREE_H_
#define _OCT_TREE_H_
#include "typedefs.h"
#include "Box3.h"
#include "BaseMesh.h"
#include "CSGMesh.h"
#include <vector>
#include "FixedPolygon.h"
#include "FixedPlane.h"
#include "AutoPtr.h"
#include "configure.h"
#include "LineSet.h"

namespace GS{


enum OctNodeType{
    eUnkown = 0x0,
    eNormal   = 0x1, 
    eCritical = 0x2, 
    eIntered = 0x4
};

struct  PolygonObj{
    struct SharedData {
        OctNodeType meObjType;
        SharedData()
            :meObjType(eNormal)
        {
        }
    };
    AutoPtr<SharedData > mpSharedData;
    BaseMesh* mpMesh;
    int     mTriId;
public : 
    PolygonObj()
        :mpMesh(nullptr)
        ,mTriId(-1)
        ,mpSharedData(nullptr)
    {
 
    }

    PolygonObj(BaseMesh* pMesh, int triId)
        :mpMesh(pMesh)
        ,mTriId(triId)
        ,mpSharedData(nullptr)
    {
        
    }
    

    PolygonObj(PolygonObj& obj)
        :mpMesh(obj.mpMesh)
        ,mTriId(obj.mTriId)
        ,mpSharedData(obj.mpSharedData)
    {
        
    }
    ~PolygonObj()
    {
    }

    PolygonObj& operator=(PolygonObj& obj)
    {
        mpMesh = obj.mpMesh;
        mTriId = obj.mTriId;
        mpSharedData  = obj.mpSharedData;
        return *this;
    }

    inline int  GetID() const {return mpMesh->GetID();} 
    PolygonObj* Clone() {return new PolygonObj(*this); }

    void SetObjTypeFlag(OctNodeType type) 
    {
        if (mpSharedData.use_count() >= 2)
        {
            if (mpSharedData == nullptr)
                mpSharedData = new SharedData;
            if (type >  mpSharedData->meObjType)
                mpSharedData->meObjType = type;
        }
    }

    bool IsInBox(const Box3& bbox)
    {
        return   mpMesh->PrimitiveAABB(mTriId).IsWithinBox(bbox);
    }

    bool IntersectWithBox(const Box3& bbox)
    {
        if (mpMesh->PrimitiveAABB(mTriId).IntersectBias(bbox))
        {

            Point3D& v0 = mpMesh->Vertex( mpMesh->TriangleInfo(mTriId).VertexId[0]).pos;
            Point3D& v1 = mpMesh->Vertex( mpMesh->TriangleInfo(mTriId).VertexId[1]).pos;
            Point3D& v2 = mpMesh->Vertex( mpMesh->TriangleInfo(mTriId).VertexId[2]).pos;
            return TestTriangleAABBOverlap(v0, v1, v2, bbox);
            
        }
        return false ;
    }
    int RayFaceTest(const double3& st, const double3& et, const double3& dir,  const Plane<double>& splane);
    int  AddPolygon(BaseMesh& mesh, std::hash_map<IndexPair, bool, IndexPairCompare>& sharedObjMap, OctNodeType level);
    inline const Box3& AABB() const {return mpMesh->PrimitiveAABB(mTriId);}
private: 
    inline bool PointWithPlane(const double3& normal, double distance,  const double3& pos) const 
    {
        double dist = dot(pos, normal)+ distance; 
        if (dist > EPSF )
            return true;
        return false ; 
    }
    inline bool PointWithPlane(const double3& normal, const double3& v0,  const double3& pos) const 
    {
        double dist =  dot(normal, (pos-v0)); 
        if (dist > EPSF )
            return true;
        return false ; 
    }

};

struct OctTreeNode {
    OctTreeNode* child[8];
    Box3         bbox; 
    OctNodeType  type;
   
    OctTreeNode();
    virtual ~OctTreeNode();
    virtual bool IsLeaf() {return false ;}
};

struct OctLeafNode : public OctTreeNode{
 std::vector<PolygonObj*> polygons;
 virtual ~ OctLeafNode();
 virtual bool IsLeaf() {return true; }
};



class OctTree{
private : 
     struct RayCastingTest{
        double3 pt;
        double3 et;
        double3 dir;
        int nMeshId;
        int nCross; // -1: Same , -2 : Opposite // 0: pmrOutside
        Plane<double> splane;
        std::map<int, bool> mTriMap;
        RayCastingTest(const double3& p, const double3& r,  const double3& d, const Plane<double>& sp, int meshId)
            : pt(p)
            , et(r)
            , dir(d)
            , splane(sp)
            , nMeshId(meshId)
            , nCross(0)
            
        {
        }
        ~RayCastingTest() { mTriMap.clear();}
        bool Terminal() const {return nCross < 0 ; }
        void operator() (OctLeafNode* pNode);
     };
public:
    OctTree();
    ~OctTree();
    void BuildOctTree(std::vector<BaseMesh*>& meshes);
    void CarveTree() {CarveTreeNode(mpRoot);}
    void GenMeshesFromCells(std::vector<FixedPlaneMesh*>& meshes, bool bCritical = false );
    void GenMeshesFromCells (std::vector<BaseMesh*>& meshes , int NodeType = 0 );
    void CarveCriticalCelles(std::vector<CSGMesh*>& csgmeshes);
    void RayTraverse(double3 pt, double3 dir, RayCastingTest& func);
    PointMeshRelation  PointInPolyhedron(double3 pt, BaseMesh* pMesh);
    void CreateTreeGeometry(LineSet& segSet, const float4& color);
protected:
    void  CarveTreeNode(OctTreeNode* pNode);
    void DeleteChilds(OctTreeNode* pNode);
    void GetLeafNodes(OctTreeNode* pNode, std::vector<OctLeafNode*>& leaves, int NodeType);
    OctTreeNode* BuildTree(std::vector<BaseMesh*>& meshes, std::vector<PolygonObj*>& polygons, Box3& bbox, OctNodeType NodeType);
    void SplitSpaceByXYZ(const Box3& bbox, Box3 childBoxs[]);
    void DetermineCriticalCell(std::vector<BaseMesh*>& meshes, const Box3 cells[], OctNodeType CellType[]);
    OctTreeNode*  CreateLeafNode(const Box3& bbox, OctNodeType  NodeType, std::vector<PolygonObj* >& polygons );
    void FillSplittingLine(OctTreeNode* pNode, LineSet& segSet, const float4& color);
    // for ray- cast purpose 
    int FindFirstNode(double tx0, double ty0, double tz0, double txm, double tym, double tzm); 
    int GetNextNode(double txm, int x, double tym, int y, double tzm, int z);
    void ProcessSubNode ( double tx0, double ty0, double tz0, double tx1, double ty1, double tz1, OctTreeNode* pNode, int& a, RayCastingTest& func );
private: 
    OctTreeNode* mpRoot;
    std::map<int, int> mMeshOrder;
    bool mbCarved ; 
};

}



#endif 
