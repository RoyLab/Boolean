#pragma once
#include "AABB.h"
#include <map>
#include <vector>

namespace CSG
{
    enum NodeType
    {
        NODE_UNKNOWN = 0,
        NODE_MIDSIDE,
        NODE_SIMPLE,
        NODE_COMPOUND
    };

    struct CSGMesh;

    struct OctreeNode
    {
        AABB BoundingBox;
        NodeType Type;

        OctreeNode *Child, *Parent;
        std::map<const CSGMesh*, std::vector<unsigned>>  TriangleTable;
        unsigned TriangleCount;

        OctreeNode():Child(0), Parent(0)
        , TriangleCount(0){}
    };

    struct Octree
    {
        OctreeNode *Root;
        
        CSGMesh**   pMesh;
        unsigned    nMesh;
        int**       ppRelationTable;
    };

    Octree* BuildOctree(CSGMesh** meshList, unsigned nMesh);

}// namespace CSG
