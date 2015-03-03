#include "precompile.h"
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

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "MPMesh.h"

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


	void StdOutput(char* str)
	{
		std::string ch(str);
		WriteConsole(_output, str, ch.size(), 0, 0);
		WriteConsole(_output, "\n", 1, 0, 0);
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

        MPMesh** arrMesh = NULL;
        int nMesh = -1;

		StdOutput("Start:");
        t0 = clock();
        CSGTree* pCSGTree = ConvertCSGTree(input, &arrMesh, &nMesh);
        Octree* pOctree = BuildOctree(arrMesh, nMesh);

		//CSGTree* pPosCSG = ConvertToPositiveTree(pCSGTree);
		//delete pCSGTree;

  //      sprintf(ch, "RelationTest:%d\0", clock()-t0);
		//t0 = clock();
		//StdOutput(ch);

		//RelationTest(pOctree);

  //      sprintf(ch, "/RelationTest:%d\0", clock()-t0);
		//t0 = clock();
		//StdOutput(ch);

		//TagLeaves(pOctree, pPosCSG);

		//delete pOctree;
		//delete pPosCSG;

		//for (uint i = 0; i < nMesh; i++)
		//	delete arrMesh[i];
		//delete [] arrMesh;

		//sprintf(ch, "End:%d\0", clock()-t0);
		//StdOutput(ch);

		return NULL;
	}


    extern "C" CSG_API GS::BaseMesh* BooleanOperation(GS::CSGExprNode* input, HANDLE stdoutput)
    {
		return BooleanOperation2(input, stdoutput);
		return NULL;
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

}

