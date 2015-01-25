#pragma warning(disable: 4800)
#include "NewOctree.h"
#include "NewCSGTree.h"
#include "CSGExprNode.h"
#include "BaseMesh.h"
#include "NewCSGMesh.h"
#include "topology.h"
#include <list>
#include <set>

#define CSG_EXPORTS
#include "Bool.h"

using GS::double3;

namespace CSG
{
    typedef unsigned uint;

    void ParsingCSGTree(CSGTree* pCSGTree, uint num, int ***tab);
    void GetResultMesh(Octree* pOctree, GS::ListOfvertices &vertex);

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

        GS::ListOfvertices vertices;
        GetResultMesh(pOctree, vertices);
        delete pOctree;
        
        GS::BaseMesh* pRes = ConverteToBaseMesh(vertices);

        return pRes;
    }

    extern "C" CSG_API GS::BaseMesh* BooleanOperation_MultiThread(GS::CSGExprNode* input)
    {
        return NULL;
    }

    static inline bool IsLeaf(CSGTreeNode* node) {return !(node->pLeft && node->pRight);}

    static void ParseNode(CSGTreeNode* node, int ***tab, uint num, std::vector<uint> *meshList)
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
                        (*tab)[leftMeshList[i]][rightMeshList[j]] ^= (REL_SAME ^ REL_OUTSIDE);
                        (*tab)[rightMeshList[j]][leftMeshList[i]] ^= REL_OUTSIDE;
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
                        (*tab)[leftMeshList[i]][rightMeshList[j]] ^= REL_INSIDE;
                        (*tab)[rightMeshList[j]][leftMeshList[i]] ^= REL_INSIDE;
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
                        (*tab)[leftMeshList[i]][rightMeshList[j]] ^= (REL_OPPOSITE ^ REL_OUTSIDE);
                        (*tab)[rightMeshList[j]][leftMeshList[i]] ^= REL_INSIDE;
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


    static void ParsingCSGTree(CSGTree* pCSGTree, uint num, int ***tab)
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

    static inline bool IsLeaf(OctreeNode* node) {return !node->Child;}

    static void GetLeafNodes(OctreeNode* pNode, std::list<OctreeNode*>& leaves, int NodeType)
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

    static void TriangleIntersectionTest(Octree* pOctree, uint meshId1, uint meshId2, OctreeNode* leaf)
    {
        auto &triangles1 = leaf->TriangleTable.find(meshId1);
        auto &triangles2 = leaf->TriangleTable.find(meshId2);

        if (triangles1 == leaf->TriangleTable.end() || 
            triangles2 == leaf->TriangleTable.end()) return;

        auto mesh1 = pOctree->pMesh[meshId1];
        auto mesh2 = pOctree->pMesh[meshId2];
        
        for (auto i: triangles1->second)
        {
            auto &carveInfo = pOctree->CarvedTriangleInfo[meshId1][i];
            carveInfo.ToBeTest.insert(leaf->ValidTable.begin(), leaf->ValidTable.end());
            carveInfo.ToBeTest.erase(meshId1);

            if (!carveInfo.Surface)
            {
                std::vector<int> triIds(1);
                triIds[0] = (int)i;
                carveInfo.Surface = new GS::Surface<double>(mesh1->pOrigin, triIds);
                carveInfo.Surface->GenerateOuterLineByTri();
                carveInfo.Surface->GenerateAABB();
            }
        }

        for (auto j: triangles2->second)
        {
            auto &carveInfo = pOctree->CarvedTriangleInfo[meshId2][j];
            carveInfo.ToBeTest.insert(leaf->ValidTable.begin(), leaf->ValidTable.end());
            carveInfo.ToBeTest.erase(meshId2);

            if (!carveInfo.Surface)
            {
                std::vector<int> triIds(1);
                triIds[0] = (int)j;
                carveInfo.Surface = new GS::Surface<double>(mesh2->pOrigin, triIds);
                carveInfo.Surface->GenerateOuterLineByTri();
                carveInfo.Surface->GenerateAABB();
            }
        }


        for (auto i: triangles1->second)
        {
            auto &triInfo1 = pOctree->CarvedTriangleInfo[meshId1][i];
            for (auto j: triangles2->second)
            {
                auto &tri1 = mesh1->mTriangle[i];
                auto &tri2 = mesh2->mTriangle[j];

                double3 &p0 = mesh1->mVertex[tri1.VertexIndex[0]];
                double3 &p1 = mesh1->mVertex[tri1.VertexIndex[1]];
                double3 &p2 = mesh1->mVertex[tri1.VertexIndex[2]];

                double3 &v0 = mesh2->mVertex[tri2.VertexIndex[0]];
                double3 &v1 = mesh2->mVertex[tri2.VertexIndex[1]];
                double3 &v2 = mesh2->mVertex[tri2.VertexIndex[2]];

                std::vector<GS::Seg3D<double>> intersects;
                bool IsIntersected = 
                    GS::TriangleInterTriangle(p0, p1, p2, tri1.Normal, 
                    v0, v1, v2, tri2.Normal, intersects);

                if (IsIntersected)
                {
                    auto &triInfo2 = pOctree->CarvedTriangleInfo[meshId2][j];
                    for (const auto &seg: intersects)
                    {
                        triInfo1.Surface->AddConstraint(seg.start, seg.end, mesh1->pOrigin->GetID());
                        triInfo2.Surface->AddConstraint(seg.start, seg.end, mesh2->pOrigin->GetID());
                    }
                }
            }
        }

    }

    static bool CheckMeshNodePairValid(Octree* pOctree, OctreeNode* node, uint meshId)
    {
        bool IsMeshValid = true;
        int *requestedTab = pOctree->ppRelationTable[meshId];
        do
        {
            for (auto &diffMesh: node->DiffMeshIndex)
            {
                if (diffMesh.Rela == REL_UNKNOWN)
                    diffMesh.Rela = PolyhedralInclusionTest(node->BoundingBox.Center(), pOctree, diffMesh.ID);
                int requestedRelation = requestedTab[diffMesh.ID];
                if (!(diffMesh.Rela & requestedRelation))
                {
                    IsMeshValid = false;
                    break;
                }
            }
            node = node->Parent;
        } while (IsMeshValid && node);
        return IsMeshValid;
    }

    static void Triangulate(CarvedInfo* record)
    {
        if (!record->Surface) return;
        if (record->Surface->Triangulated()) return;

        record->Triangulated.clear();
        record->Surface->Triangulate(record->Triangulated);
    }

    static inline double3 CenterOfGravity(double3& a, double3& b, double3& c)
    {
        return (a+b+c)/3.0;
    }

    static void AddVertices(Octree* pOctree, uint meshId, uint triId, GS::ListOfvertices& vertices)
    {
        auto &triInfo = pOctree->pMesh[meshId]->mTriangle[triId];
        GS::VertexInfo vInfo;
        vInfo.color = GS::float4(1,1,1,1);
        vInfo.normal = GS::double3(1,0,0);

        vInfo.pos = pOctree->pMesh[meshId]->mVertex[triInfo.VertexIndex[0]];
        vertices.push_back(vInfo);
        vInfo.pos = pOctree->pMesh[meshId]->mVertex[triInfo.VertexIndex[1]];
        vertices.push_back(vInfo);
        vInfo.pos = pOctree->pMesh[meshId]->mVertex[triInfo.VertexIndex[2]];
        vertices.push_back(vInfo);
    }


    static void MeshClassification(uint meshId, uint triId, CarvedInfo* record, Octree* pOctree, GS::ListOfvertices& vertices)
    {
        GS::ListOfvertices *candidates = NULL;
        GS::ListOfvertices single;
        if (record->Surface->HasConstraint())
        {
            candidates = &(record->Triangulated);
        }
        else
        {
            candidates = &single;
            // normals ? colors ?
            AddVertices(pOctree, meshId, triId, vertices);
        }

        const uint n = candidates->size();
        for (uint i = 0; i < n; i+=3)
        {
            bool IsValid = true;
            for (auto testMeshId: record->ToBeTest)
            {
                auto relation = 
                    PolyhedralInclusionTest(
                    CSG::CenterOfGravity((*candidates)[i].pos, 
                    (*candidates)[i+1].pos, 
                    (*candidates)[i+2].pos),
                    pOctree, testMeshId);

                if (!(relation & pOctree->ppRelationTable[meshId][testMeshId]))
                {
                    IsValid = false;
                    break;
                }
            }
            if (IsValid)
                vertices.insert(vertices.end(), record->Triangulated.begin()+i,
                    record->Triangulated.begin()+i+3);
        }
    }

    static void GetResultMesh(Octree* pOctree, GS::ListOfvertices& vertex)
    {
        assert(pOctree && pOctree->Root);

        // intersect test
        std::list<OctreeNode*> leaves;
        GetLeafNodes(pOctree->Root, leaves, NODE_COMPOUND);

        pOctree->CarvedTriangleInfo.resize(pOctree->nMesh);
        for (auto leaf: leaves) // for each leaf node
        {
            for (auto &triIndex:leaf->TriangleTable) // for every mesh in a leaf node
            {
                uint meshId = triIndex.first;
                if (CheckMeshNodePairValid(pOctree, leaf, meshId))
                    leaf->ValidTable.push_back(meshId);
            }

            uint n = leaf->ValidTable.size();
            for (uint i = 0; i < n; i++)
                for (uint j = i+1; j < n; j++)
                    TriangleIntersectionTest(pOctree, i, j, leaf);
        }

        // collect simple nodes
        leaves.clear();
        GetLeafNodes(pOctree->Root, leaves, NODE_SIMPLE);
        std::vector<std::set<uint>> unTouchedTriangles(pOctree->nMesh);

        for (auto leaf: leaves) // for each leaf node
        {
            for (auto &triIndex:leaf->TriangleTable) // for every mesh in a leaf node
            {
                uint meshId = triIndex.first;
                if (CheckMeshNodePairValid(pOctree, leaf, meshId)) 
                {
                    auto &tri = triIndex.second;
                    auto &CarvedtriInfo = pOctree->CarvedTriangleInfo[meshId];
                    for (auto index: tri)
                    {
                        if (CarvedtriInfo.find(index) == CarvedtriInfo.end())
                            unTouchedTriangles[meshId].insert(index);
                    }
                }
            }
        }

        // triangulate carved meshes
        vertex.clear();
        for (uint i = 0; i < pOctree->nMesh; i++)
        {
            auto &carve = pOctree->CarvedTriangleInfo[i];
            for (auto &carveTri: carve)
            {
                Triangulate(&carveTri.second);
                MeshClassification(i, carveTri.first, &carveTri.second, pOctree, vertex);
            }
        }

        for (uint i =0; i < pOctree->nMesh; i++)
            for (uint index: unTouchedTriangles[i])
                AddVertices(pOctree, i, index, vertex);
    }
}

