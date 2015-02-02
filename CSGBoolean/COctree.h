#pragma once
#include "AABB.h"
#include <vector>
#include "Box3.h"
#include "Intersect.h"
#include "Surface.h"
#include <set>
#include <map>
#include <list>


namespace GS
{
    template <typename P> class Surface;
}


namespace CSG
{
    enum NodeType
    {
        NODE_UNKNOWN = 0,
        NODE_MIDSIDE,
        NODE_SIMPLE,
        NODE_COMPOUND
    };

    enum Relation
    {
        REL_UNKNOWN = 0,
        REL_INSIDE = 1,
        REL_OUTSIDE = 2,
        REL_SAME = 4,
        REL_OPPOSITE = 8
    };

    typedef unsigned uint;
    struct CSGMesh;

    struct DiffMeshInfo
    {
        unsigned ID;
        Relation Rela;

        DiffMeshInfo(unsigned i, Relation rel = REL_UNKNOWN):
            ID(i), Rela(rel){}
    };

    struct OctreeNode
    {
        AABB BoundingBox;
        NodeType Type;

        OctreeNode *Child, *Parent;

        std::vector<DiffMeshInfo> DiffMeshIndex;
        std::map<unsigned, std::vector<unsigned>>  TriangleTable;
        unsigned TriangleCount;

        std::vector<uint> ValidTable;

        OctreeNode():Child(0), Parent(0)
        , TriangleCount(0){}
    };



    struct CarvedInfo
    {
        GS::Surface<double>* Surface;
        //std::list<GS::Seg3D<double>> Carved;
        GS::ListOfvertices Triangulated;
        unsigned TriangulateCount;
        std::set<unsigned> ToBeTest;

        CarvedInfo():TriangulateCount(0), Surface(0){}
    };

    typedef std::vector<std::map<unsigned, CarvedInfo>> TriangleRecord; // may be better as a vector

    struct Octree
    {
        OctreeNode *Root;
        
        CSGMesh**   pMesh;
        unsigned    nMesh;
        int**       ppRelationTable;
        TriangleRecord CarvedTriangleInfo;
    };

    Octree* BuildOctree(CSGMesh** meshList, unsigned nMesh, int** reltab);
    Relation PolyhedralInclusionTest(GS::double3& point, Octree* pOctree, uint meshId, bool = false);

    inline bool IsLeaf(OctreeNode* node) {return !node->Child;}

}// namespace CSG

