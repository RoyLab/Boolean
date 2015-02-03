#pragma warning(disable: 4800 4996)
#include "COctree.h"
#include "BinaryTree.h"
#include "CSGExprNode.h"
#include "BaseMesh.h"
#include "CMesh.h"
#include "topology.h"
#include <list>
#include <set>
#include <ctime>

#ifdef _DEBUG
#include <vld.h>
#endif

#define CSG_EXPORTS
#include "Bool.h"

using GS::double3;

namespace CSG
{
	HANDLE _output;
	clock_t t0;
    typedef unsigned uint;

    void ParsingCSGTree(CSGTree* pCSGTree, uint num, int ***tab);
    void GetResultMesh(Octree* pOctree, GS::ListOfvertices &vertex);

	void StdOutput(char* str)
	{
		std::string ch(str);
		WriteConsole(_output, str, ch.size(), 0, 0);
		WriteConsole(_output, "\n", 1, 0, 0);
	}

	static GS::BaseMesh* BooleanOperation1(GS::CSGExprNode* input, HANDLE stdoutput)
	{
        char ch[32];
		_output= stdoutput;

        CSGMesh** arrMesh = NULL;
        int nMesh = -1;

		StdOutput("Start:");
        t0 = clock();

        CSGTree* pCSGTree = ConvertCSGTree(input, &arrMesh, &nMesh);
        if (!pCSGTree) return NULL;

        sprintf(ch, "Convert:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);
        
        int **relationTab = NULL;
        ParsingCSGTree(pCSGTree, nMesh, &relationTab);
        if (!relationTab) return NULL;

        sprintf(ch, "ParseTree:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

        Octree* pOctree = BuildOctree(arrMesh, nMesh);
        if (!pOctree) return NULL;

        delete pCSGTree;

        sprintf(ch, "BuildOctree:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

        GS::ListOfvertices vertices;
        GetResultMesh(pOctree, vertices);
        delete pOctree;

        sprintf(ch, "GenMesh:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);
        
        GS::BaseMesh* pRes = ConverteToBaseMesh(vertices);

        sprintf(ch, "Convert:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

        return pRes;
	}

	static void RelationTest(OctreeNode* pNode, Octree* pOctree)
	{
		for (auto &pair: pNode->DiffMeshIndex)
			pair.Rela = PolyhedralInclusionTest(pNode->BoundingBox.Center(),
			pOctree, pair.ID);

		if (pNode->Child)
		{
			for (uint i = 0; i < 8; i++)
				RelationTest(&pNode->Child[i], pOctree);
		}
	}

	static void RelationTest(Octree* pOctree)
	{
		RelationTest(pOctree->Root, pOctree);
	}

	static void TagNode(OctreeNode* root, const CSGTree* last)
	{
		const CSGTree* cur(nullptr);
		bool isMemAllocated = false;
		Relation curRel = REL_UNKNOWN;

		if (last)
		{
			if (root->DiffMeshIndex.size())
			{
				// compress with in/out info, create a image to cur
				isMemAllocated = true;
				CSGTree* tmpTree = copy(last);
				for (auto &pair: root->DiffMeshIndex)
				{
					curRel = CompressCSGTree(tmpTree, pair.ID, pair.Rela);
					if (curRel != REL_UNKNOWN)
						break;
				}
				cur = tmpTree;
			}
			else cur = last;
		}
		
		if (IsLeaf(root))
		{
			if (root->Type == NODE_SIMPLE)
			{
				root->pRelationData = new SimpleData(false);

				if (cur && cur->Leaves.size() == 1)
					*(SimpleData*)(root->pRelationData) = true;

				assert(!(cur && cur->Leaves.size() > 1));
			}
			else root->pRelationData = copy(cur);
		}
		else
		{
			if (curRel != REL_UNKNOWN)
			{
				for (uint i = 0; i < 8; i++)
					TagNode(&root->Child[i], 0);
			}
			else
			{
				for (uint i = 0; i < 8; i++)
					TagNode(&root->Child[i], cur);
			}
		}

		if (isMemAllocated) delete cur; // !! cur is a const pointer
	}

	static void TagLeaves(Octree* pOctree, CSGTree* pCSG)
	{
		TagNode(pOctree->Root, pCSG);
	}

	static GS::BaseMesh* BooleanOperation2(GS::CSGExprNode* input, HANDLE stdoutput)
	{
		char ch[32];
		_output= stdoutput;

        CSGMesh** arrMesh = NULL;
        int nMesh = -1;

        CSGTree* pCSGTree = ConvertCSGTree(input, &arrMesh, &nMesh);
        Octree* pOctree = BuildOctree(arrMesh, nMesh);

		CSGTree* pPosCSG = ConvertToPositiveTree(pCSGTree);
		delete pCSGTree;

		RelationTest(pOctree);
		TagLeaves(pOctree, pPosCSG);

		delete pOctree;
		delete pPosCSG;

		for (uint i = 0; i < nMesh; i++)
			delete arrMesh[i];
		delete [] arrMesh;

		return NULL;
	}


    extern "C" CSG_API GS::BaseMesh* BooleanOperation(GS::CSGExprNode* input, HANDLE stdoutput)
    {
		return BooleanOperation2(input, stdoutput);
    }

    extern "C" CSG_API GS::BaseMesh* BooleanOperation_MultiThread(GS::CSGExprNode* input)
    {
        return NULL;
    }

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



  /*  static void TriangleIntersectionTest(Octree* pOctree, uint meshId1, uint meshId2, OctreeNode* leaf)
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
            AddVertices(pOctree, meshId, triId, single);
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
                vertices.insert(vertices.end(), candidates->begin()+i,
                    candidates->begin()+i+3);
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

		char ch[32];
        sprintf(ch, "Intersect:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

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

        sprintf(ch, "CollectSimple:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

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

        sprintf(ch, "CollectOther:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

        for (uint i =0; i < pOctree->nMesh; i++)
            for (uint index: unTouchedTriangles[i])
                AddVertices(pOctree, i, index, vertex);
    }*/
}

