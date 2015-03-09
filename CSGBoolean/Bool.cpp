#include "precompile.h"
#include "MPMesh.h"
#define CSG_EXPORTS
#include "Bool.h"
#include <ctime>
#include <list>
#include "COctree.h"
#include "BinaryTree.h"
#include "isect.h"


#ifdef _DEBUG
#include <vld.h>
#endif

#pragma warning(disable: 4800 4996)


namespace CSG
{
	HANDLE _output;
	clock_t t0;

    static void GetLeafNodes(OctreeNode* pNode, std::list<OctreeNode*>& leaves, int NodeType);

	void StdOutput(char* str)
	{
		std::string ch(str);
		WriteConsole(_output, str, ch.size(), 0, 0);
		WriteConsole(_output, "\n", 1, 0, 0);
	}

	/*static void RelationTest(OctreeNode* pNode, Octree* pOctree)
	{
		for (auto &pair: pNode->DiffMeshIndex)
			pair.Rela = PolyhedralInclusionTest(pNode->BoundingBox.Center(),
			pOctree, pair.ID);

		if (pNode->Child)
		{
			for (unsigned i = 0; i < 8; i++)
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
				for (unsigned i = 0; i < 8; i++)
					TagNode(&root->Child[i], 0);
			}
			else
			{
				for (unsigned i = 0; i < 8; i++)
					TagNode(&root->Child[i], cur);
			}
		}

		if (isMemAllocated) delete cur; // !! cur is a const pointer
	}

	static void TagLeaves(Octree* pOctree, CSGTree* pCSG)
	{
		TagNode(pOctree->Root, pCSG);
	}*/


	/*static void TriangleIntersectionTest(Octree* pOctree, unsigned meshId1, unsigned meshId2, OctreeNode* leaf)
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
	*/

	void ISectTest(Octree* pOctree)
	{
		assert(pOctree);
		std::list<OctreeNode*> leaves;
		GetLeafNodes(pOctree->Root, leaves, NODE_COMPOUND);

		for (auto leaf: leaves)
		{
			auto itr = leaf->TriangleTable.begin();
			auto iEnd = leaf->TriangleTable.end();
			decltype(iEnd) itr2;
			unsigned i, j, ni, nj;
			MPMesh *meshi, *meshj;
			MPMesh::FaceHandle tri1, tri2;
			Vec3d v0,v1,v2,nv,u0,u1,u2,nu,start,end;
			MPMesh::FVIter fvItr;
			bool isISect;
			int startT(0), endT(0);

			for (; itr != iEnd; ++itr)
			{
				itr2 = itr;
				++itr2;
				for (; itr2 != iEnd; ++itr2)
				{
					ni = itr->second.size();
					nj = itr2->second.size();

					meshi = pOctree->pMesh[itr->first];
					meshj = pOctree->pMesh[itr2->first];

					for (i = 0; i < ni; i++)
					{
						for (j = 0; j < nj; j++)
						{
							tri1 = itr->second[i];
							tri2 = itr2->second[j];

							// intersection test main body
							fvItr = meshi->fv_iter(tri1);
							v0 = meshi->point(*fvItr++);
							v1 = meshi->point(*fvItr++);
							v2 = meshi->point(*fvItr);

							fvItr = meshj->fv_iter(tri2);
							u0 = meshj->point(*fvItr++);
							u1 = meshj->point(*fvItr++);
							u2 = meshj->point(*fvItr);

							nv = meshi->normal(tri1);
							nu = meshj->normal(tri2);
							
							isISect = TriTriIntersectTest(v0, v1, v2, nv,
								u0, u1, u2, nu, startT, endT, start, end);

							if (!isISect) continue;


						}
					}
				}
			}

		}

	}

	static GS::BaseMesh* BooleanOperation2(GS::CSGExprNode* input, HANDLE stdoutput)
	{
		char ch[32];
		_output= stdoutput;

        MPMesh** arrMesh = NULL;
        int nMesh = -1;

		StdOutput("Start:");
        t0 = clock();

        CSGTree* pCSGTree = ConvertCSGTree(input, &arrMesh, &nMesh);
		CSGTree* pPosCSG = ConvertToPositiveTree(pCSGTree);
		delete pCSGTree;

        Octree* pOctree = BuildOctree(arrMesh, nMesh);

        sprintf(ch, "RelationTest:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

		ISectTest(pOctree);
		//RelationTest(pOctree);
        sprintf(ch, "/RelationTest:%d\0", clock()-t0);
		t0 = clock();
		StdOutput(ch);

		//TagLeaves(pOctree, pPosCSG);

		delete pOctree;
		delete pPosCSG;

		for (int i = 0; i < nMesh; i++)
			delete arrMesh[i];
		delete [] arrMesh;

		sprintf(ch, "End:%d\0", clock()-t0);
		StdOutput(ch);

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

		if (IsLeaf(pNode) || pNode->Type == NODE_SIMPLE)
        {
            if (!pNode->TriangleCount) return; 
            if (NodeType == pNode->Type)
                leaves.push_back(pNode);
            return;
        }

        for ( int i = 0; i < 8 ; i++)
            GetLeafNodes(&pNode->Child[i], leaves, NodeType);
    }

}

