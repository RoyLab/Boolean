#pragma once
#include "AABB.h"
#include <map>
#include <vector>
#include <hash_map>

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
        REL_INSIDE,
        REL_OUTSIDE,
        REL_ON
    };


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
        std::map<const CSGMesh*, std::vector<unsigned>>  TriangleTable;
        unsigned TriangleCount;

        OctreeNode():Child(0), Parent(0)
        , TriangleCount(0){}
    };

    typedef int CarvedInfo;
    typedef std::vector<std::hash_map<unsigned, CarvedInfo>> TriangleRecord; // may be better as a vector

    struct Octree
    {
        OctreeNode *Root;
        
        CSGMesh**   pMesh;
        unsigned    nMesh;
        int**       ppRelationTable;
        TriangleRecord CarvedTriangleInfo;
    };

    Octree* BuildOctree(CSGMesh** meshList, unsigned nMesh, int** reltab);
    Relation PolyhedralInclusionTest(GS::double3& point, Octree* pOctree);

}// namespace CSG
