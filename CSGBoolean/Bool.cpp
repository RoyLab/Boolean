#include "NewOctree.h"
#include "NewCSGTree.h"
#include "CSGExprNode.h"
#include "BaseMesh.h"
#include "NewCSGMesh.h"

#define CSG_EXPORTS
#include "Bool.h"

namespace CSG
{
    typedef unsigned uint;

    void ParsingCSGTree(CSGTree* pCSGTree, uint num, int ***tab);
    void IntersectTest(Octree* pOctree);
    CSGMesh* MeshClassification(Octree* pOctree);

    extern "C" CSG_API GS::BaseMesh* BooleanOperation(GS::CSGExprNode* input)
    {
        CSGMesh** arrMesh = NULL;
        int nMesh = -1;

        CSGTree* pCSGTree = ConvertCSGTree(input, &arrMesh, &nMesh);
        if (!pCSGTree) return NULL;
        
        Octree* pOctree = BuildOctree(arrMesh, nMesh);
        if (!pOctree) return NULL;

        ParsingCSGTree(pCSGTree, nMesh, &pOctree->ppRelationTable);
        IntersectTest(pOctree);

        CSGMesh* pRes1 = MeshClassification(pOctree);
        if (!pRes1) return NULL;
        
        GS::BaseMesh* pRes = ConverteToBaseMesh(pRes1);
        delete pRes1;
        return pRes;
    }

    extern "C" CSG_API GS::BaseMesh* BooleanOperation_MultiThread(GS::CSGExprNode* input)
    {
        return NULL;
    }

    void ParsingCSGTree(CSGTree* pCSGTree, uint num, int ***tab)
    {
        int **&table = *tab;
        table = new int*[num];
        for (uint i = 0; i < num; i++)
        {
            table[i] = new int[num];
            memset(table[i], 0, sizeof(int)*num);
        }


    }

    void IntersectTest(Octree* pOctree)
    {

    }

    CSGMesh* MeshClassification(Octree* pOctree)
    {
        return NULL;
    }


}

