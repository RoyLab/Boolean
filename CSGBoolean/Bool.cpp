#include "NewOctree.h"
#include "NewCSGTree.h"
#include "CSGExprNode.h"
#include "BaseMesh.h"
#include "NewCSGMesh.h"
#include <list>

#define CSG_EXPORTS
#include "Bool.h"

using GS::double3;

namespace CSG
{
    typedef unsigned uint;

    void ParsingCSGTree(CSGTree* pCSGTree, uint num, int ***tab);
    int GetResultMesh(Octree* pOctree, double3 **vertex);

    extern "C" CSG_API GS::BaseMesh* BooleanOperation(GS::CSGExprNode* input)
    {
        CSGMesh** arrMesh = NULL;
        int nMesh = -1;

        CSGTree* pCSGTree = ConvertCSGTree(input, &arrMesh, &nMesh);
        if (!pCSGTree) return NULL;
        
        int **relationTab = NULL;
        ParsingCSGTree(pCSGTree, nMesh, &relationTab);
        if (!relationTab) return NULL;

        Octree* pOctree = BuildOctree(arrMesh, nMesh, relationTab);
        if (!pOctree) return NULL;

        delete pCSGTree;

        double3* pRes1 = NULL;
        int triangleCount = GetResultMesh(pOctree, &pRes1);
        delete pOctree;
        if (!pRes1) return NULL;
        
        GS::BaseMesh* pRes = ConverteToBaseMesh(pRes1, triangleCount);
        delete pRes1;

        return pRes;
    }

    extern "C" CSG_API GS::BaseMesh* BooleanOperation_MultiThread(GS::CSGExprNode* input)
    {
        return NULL;
    }

    bool IsLeaf(CSGTreeNode* node) {return !(node->pLeft && node->pRight);}

    void ParseNode(CSGTreeNode* node, int ***tab, uint num, std::vector<uint> *meshList)
    {
        if (!node) return;
        if (IsLeaf(node))
        {
            meshList->push_back(node->pMesh->ID);
            return;
        }

        std::vector<uint> leftMeshList, rightMeshList;
        ParseNode(node->pLeft, tab, num, &leftMeshList);
        ParseNode(node->pRight, tab, num, &rightMeshList);

        switch (node->Operation)
        {
        case OP_UNION:
            {
                uint n = leftMeshList.size();
                uint k = rightMeshList.size();

                for (uint i = 0; i < n; i++)
                {
                    for (uint j = 0; j < k; j++)
                    {
                        (*tab)[leftMeshList[i]][rightMeshList[j]] ^= (CPN_SAME ^ CPN_OUTSIDE);
                        (*tab)[rightMeshList[j]][leftMeshList[i]] ^= CPN_OUTSIDE;
                    }
                }
                break;
            }
        case OP_INTERSECT:
            {
                uint n = leftMeshList.size();
                uint k = rightMeshList.size();

                for (uint i = 0; i < n; i++)
                {
                    for (uint j = 0; j < k; j++)
                    {
                        (*tab)[leftMeshList[i]][rightMeshList[j]] ^= CPN_INSIDE;
                        (*tab)[rightMeshList[j]][leftMeshList[i]] ^= CPN_INSIDE;
                    }
                }
                break;
            }
        case OP_DIFF:
            {
                uint n = leftMeshList.size();
                uint k = rightMeshList.size();

                for (uint i = 0; i < n; i++)
                {
                    for (uint j = 0; j < k; j++)
                    {
                        (*tab)[leftMeshList[i]][rightMeshList[j]] ^= (CPN_OPPOSITE ^ CPN_OUTSIDE);
                        (*tab)[rightMeshList[j]][leftMeshList[i]] ^= CPN_INSIDE;
                        (*tab)[rightMeshList[j]][num]++;
                    }
                }
                break;
            }
        default:
            assert(0);
            break;
        }

        // mergeMeshList
        for (uint i: leftMeshList) meshList->push_back(i);
        for (uint i: rightMeshList) meshList->push_back(i);
    }


    void ParsingCSGTree(CSGTree* pCSGTree, uint num, int ***tab)
    {
        assert(pCSGTree);

        int **&table = *tab;
        table = new int*[num];
        for (uint i = 0; i < num; i++)
        {
            // num rows record the relation
            // the last one row record information of INVERSE
            table[i] = new int[num+1];
            memset(table[i], 0, sizeof(int)*(1+num));
        }

        std::vector<uint> meshList;
        ParseNode(pCSGTree->pRoot, tab, num, &meshList);
    }

    bool IsLeaf(OctreeNode* node) {return !node->Child;}

    void GetLeafNodes(OctreeNode* pNode, std::list<OctreeNode*>& leaves, int NodeType)
    {
        if (pNode == NULL) return;

        if (IsLeaf(pNode))
        {
            if (!pNode->TriangleCount) return; 
            if (NodeType == pNode->Type)
                leaves.push_back(pNode);
            return ;
        }

        for ( int i = 0; i < 8 ; i++)
            GetLeafNodes(&pNode->Child[i], leaves, NodeType);
    }

    void TriangleIntersectionTest(Octree* pOctree, uint i, uint j, OctreeNode* leaf)
    {

    }

    bool CheckMeshNodePairValid(Octree* pOctree, OctreeNode* node, uint meshId)
    {
        bool IsMeshValid = true;
        int *requestedTab = pOctree->ppRelationTable[meshId];
        do
        {
            for (auto &diffMesh: node->DiffMeshIndex)
            {
                if (diffMesh.Rela == REL_UNKNOWN)
                    diffMesh.Rela = PolyhedralInclusionTest(node->BoundingBox.Center(), pOctree);
                assert(diffMesh.Rela != REL_ON);
                int requestedRelation = requestedTab[diffMesh.ID];
                if ((diffMesh.Rela == REL_INSIDE && !(requestedRelation & CPN_INSIDE)) ||
                    (diffMesh.Rela == REL_OUTSIDE && !(requestedRelation & CPN_OUTSIDE)))
                {
                    IsMeshValid = false;
                    break;
                }
            }
            node = node->Parent;
        } while (IsMeshValid && node);
        return IsMeshValid;
    }


    int GetResultMesh(Octree* pOctree, double3 **vertex)
    {
        assert(pOctree && pOctree->Root);

        // intersect test
        std::list<OctreeNode*> leaves;
        GetLeafNodes(pOctree->Root, leaves, NODE_COMPOUND);

        for (auto leaf: leaves) // for each leaf node
        {
            std::vector<uint> validTable;
            for (auto &triIndex:leaf->TriangleTable) // for every mesh in a leaf node
            {
                uint meshId = triIndex.first->ID;
                if (CheckMeshNodePairValid(pOctree, leaf, meshId))
                    validTable.push_back(meshId);
            }

            uint n = validTable.size();
            for (uint i = 0; i < n; i++)
                for (uint j = i+1; j < n; j++)
                    TriangleIntersectionTest(pOctree, i, j, leaf);
        }

        // collect simple nodes
        leaves.clear();
        GetLeafNodes(pOctree->Root, leaves, NODE_SIMPLE);
        std::vector<std::list<uint>> unTouchedTriangles(pOctree->nMesh);

        for (auto leaf: leaves) // for each leaf node
        {
            for (auto &triIndex:leaf->TriangleTable) // for every mesh in a leaf node
            {
                uint meshId = triIndex.first->ID;
                if (CheckMeshNodePairValid(pOctree, leaf, meshId))
                {
                    auto &tri = triIndex.second;
                    auto &CarvedtriInfo = pOctree->CarvedTriangleInfo[meshId];
                    for (auto index: tri)
                    {
                        if (CarvedtriInfo.find(index) == CarvedtriInfo.end())
                            unTouchedTriangles[meshId].emplace_back(index);
                    }
                }
            }
        }

        // triangulate carved meshes
        for (uint i = 0; i < pOctree->nMesh; i++)
        {

        }

        return 0;
    }
}

