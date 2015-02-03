#pragma once
#include "precompile.h"
#include "AABB.h"
#include "Box3.h"
#include <vector>
#include "Intersect.h"
#include "Surface.h"
#include <map>

namespace CSG
{
	struct CSGTree;

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

    struct CSGMesh;

    struct DiffMeshInfo
    {
        uint ID;
        Relation Rela;

        DiffMeshInfo(uint i, Relation rel = REL_UNKNOWN):
			ID(i), Rela(rel){}
    };

	typedef bool SimpleData;
	typedef CSGTree ComplexData;

    struct OctreeNode
    {
        AABB BoundingBox;
        NodeType Type;

        OctreeNode *Child, *Parent;

        std::vector<DiffMeshInfo> DiffMeshIndex;
        std::map<uint, std::vector<uint>>  TriangleTable;
        uint TriangleCount;

		void *pRelationData;

        OctreeNode();
		~OctreeNode();
    };



    struct CarvedInfo
    {
        GS::Surface<double>* Surface;
        GS::ListOfvertices Triangles;

        CarvedInfo():Surface(0){}
    };

    typedef std::vector<std::map<uint, CarvedInfo>> TriangleRecord; // may be better as a vector

    struct Octree
    {
        OctreeNode *Root;
        
        CSGMesh**   pMesh;
        uint			nMesh;

		Octree();
		~Octree();
    };

    Octree* BuildOctree(CSGMesh** meshList, uint nMesh);
    Relation PolyhedralInclusionTest(GS::double3& point, Octree* pOctree, uint meshId, bool = false);

    inline bool IsLeaf(OctreeNode* node) {return !node->Child;}

}// namespace CSG

