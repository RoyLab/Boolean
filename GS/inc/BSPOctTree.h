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
    SimpleMesh(const BaseMesh*);
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
    NormalMesh(const BaseMesh*);
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

struct MeshData
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

    BSPOctree(FixedBSPTree::SET_OP op): mOperation(op){}
    virtual ~BSPOctree(void);

    void BSPOperation(const BaseMesh*, const BaseMesh*, BaseMesh**);

protected:
    void PerformIteration(PolygonPtrList&, PolygonPtrList&, Box3&);
    void DetermineCriticalCell(const Box3[], bool[]);
    void PerformBoolean(PolygonPtrList&, PolygonPtrList&, const Box3&);
    void SplitSpaceByXYZ(const Box3& bbox,  Box3 childBoxes[]);

    PolygonList         mMesh1, mMesh2;
    Box3                mAABB1, mAABB2;
    std::vector<FixedPlanePolygon> mPlanePolygon;
    FixedBSPTree::SET_OP mOperation;
};

}

