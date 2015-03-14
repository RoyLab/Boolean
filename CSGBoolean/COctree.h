#pragma once
#include <vector>
#include <map>
#include "MPMesh.h"


namespace CSG
{
	struct CSGTree;
    struct MPMesh;

	//extern const char POINT_INOUT_TEST_STRING[16];

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
        REL_OPPOSITE = 8,
		REL_NA = -1
    };

    struct DiffMeshInfo
    {
        unsigned ID;
        Relation Rela;

        DiffMeshInfo(
			unsigned i, 
			Relation rel = REL_UNKNOWN
			): ID(i), Rela(rel)
		{}
    };

	//typedef bool SimpleData;
	//typedef CSGTree ComplexData;

	typedef std::map<unsigned, std::vector<MPMesh::FaceHandle>> TriTableT;

    struct OctreeNode
    {
        AABBmp BoundingBox;
        NodeType Type;

        OctreeNode *Child, *Parent;

        std::vector<DiffMeshInfo> DiffMeshIndex;
        TriTableT	TriangleTable;
        unsigned		TriangleCount;

		void *pRelationData;

        OctreeNode();
		~OctreeNode();
    };



    //struct CarvedInfo
    //{
    //    GS::Surface<double>* Surface;
    //    GS::ListOfvertices Triangles;

    //    CarvedInfo():Surface(0){}
    //};

    //typedef std::vector<std::map<unsigned, CarvedInfo>> TriangleRecord; // may be better as a vector

    struct Octree
    {
        OctreeNode *Root;
        
        MPMesh**		pMesh;
        unsigned		nMesh;

		Octree();
		~Octree();
    };

    Octree* BuildOctree(MPMesh** meshList, unsigned nMesh);
    Relation PolyhedralInclusionTest(Vec3d& point, Octree* pOctree, unsigned meshId, bool = false);

    inline bool IsLeaf(OctreeNode* node) {return !node->Child;}

}// namespace CSG

