#pragma once
#include "octtree.h"
#include "BaseMesh.h"
#include "FixedBSPTree.h"
#include "FixedPlaneMesh.h"

namespace GS
{

class SimpleMesh
{
public:
    enum {PARA_NEGATE = 1};
    SimpleMesh(const BaseMesh*, int = 0);
    virtual ~SimpleMesh(){}

    const std::vector<double3>& Vertex() const {return mVertex;}
    const std::vector<int3>& Triangle() const {return mTriangle;}

protected:
    std::vector<double3> mVertex;
    std::vector<int3> mTriangle;
};

class NormalMesh:
    public SimpleMesh
{
public:
    NormalMesh(const BaseMesh*, int = 0);
    virtual ~NormalMesh(){}

    void NormalizeCoord(const Box3* = 0);
    void DenormalizeCoord();
    void FilterVertex(unsigned);

    const Box3& GetTransAABB() const 
    {assert(m_bCoordNormalized); return mTransAABB;}

protected:
    Box3 mCurAABB, mOrigAABB, mTransAABB;
    bool m_bCoordNormalized;
};

struct MeshData:
    public PolygonObj
{
    MeshData(const double3& pos1, const double3& pos2,
        const double3& pos3):
        bSplitted(false)
    {
        Position[0] = pos1;
        Position[1] = pos2;
        Position[2] = pos3;
    }

    virtual ~MeshData() 
    {
        for (auto &itr: Fragments)
            delete itr;
    }


    double3 Position[3];
    bool    bSplitted;
    std::list<FixedPlanePolygon*> Fragments;
    FixedPlanePolygon OrigPolygon;
    
    // for ray-crossing test jxd
    unsigned index;
};


struct BSPOctreeNode
{
};

class BSPOctree :
    public OctTree
{
public:
    typedef std::vector<unsigned>  PolygonPtrList;
    typedef std::vector<MeshData>   PolygonList;

    BSPOctree(FixedBSPTree::SET_OP op):
        mOperation(op)/*, mpBaseMesh1(nullptr), mpBaseMesh2(nullptr)*/{}
    virtual ~BSPOctree(void);

    void BSPOperation(BaseMesh*, BaseMesh*, BaseMesh**);

protected:
    enum {MINUEND = 0, SUBTRAHEND = 1};
    void PerformIteration(PolygonPtrList&, PolygonPtrList&, Box3&, OctTreeNode**);
    void DetermineCriticalCell(const Box3[], bool[]);
    void PerformBoolean(PolygonPtrList&, PolygonPtrList&, const Box3&, OctTreeNode**);
    void SplitSpaceByXYZ(const Box3& bbox,  Box3 childBoxes[]);
    bool CheckValid(const double3&, BaseMesh*, int = MINUEND);
    void FillOctreeLeafNode(PolygonPtrList&, PolygonPtrList&, OctLeafNode*);
    bool LookUpRelation(bool judge, bool flag);
    BaseMesh* CollectPolygons(BaseMesh* mesh1, BaseMesh* mesh2);
    bool NeedInserted(bool isCellInMesh, int = MINUEND);

    PolygonList         mMesh1, mMesh2;
    Box3                mAABB1, mAABB2;
    std::vector<FixedPlanePolygon> mPlanePolygon;
    FixedBSPTree::SET_OP mOperation;

    //BaseMesh *mpBaseMesh1, *mpBaseMesh2;
};

}

