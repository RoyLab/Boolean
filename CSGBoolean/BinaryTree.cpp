#include "BinaryTree.h"
#include "CSGExprNode.h"
#include "CMesh.h"
#include <vector>
#include <assert.h>
#include "COctree.h"

namespace CSG
{
    static void ConvertCSGTreeNode(GS::CSGExprNode* input, CSGTreeNode** pRoot, std::vector<CSGMesh*>& meshList)
    {
        if (!input) return;

        CSGTreeNode*& root = *pRoot;
        root = new CSGTreeNode;

        GS::BaseMesh* pBaseMesh = input->GetMesh();
        if (pBaseMesh)
        {
            root->pMesh = new CSGMesh(pBaseMesh);
            root->pMesh->ID = meshList.size();
            meshList.emplace_back(root->pMesh);
        }
        
        switch (input->BoolOperation())
        {
        case GS::CSGUnion:
            root->Operation = OP_UNION;
            break;
        case GS::CSGIntersect:
            root->Operation = OP_INTERSECT;
            break;
        case GS::CSGDiff:
            root->Operation = OP_DIFF;
            break;
        default:
            root->Operation = OP_UNKNOWN;
            break;
        }

        ConvertCSGTreeNode(input->LeftNode(), &root->pLeft, meshList);
        ConvertCSGTreeNode(input->RightNode(), &root->pRight, meshList);
    }


    CSGTree* ConvertCSGTree(GS::CSGExprNode* root, CSGMesh*** arrMesh, int *nMesh) // convert nodes.
    {
        if (!root) return NULL;

        CSGTree* pRes = new CSGTree;
        pRes->pRoot = NULL;
        std::vector<CSGMesh*> meshList;

        ConvertCSGTreeNode(root, &pRes->pRoot, meshList);

        CSGMesh**& pMesh = *arrMesh;
        *nMesh = meshList.size();
        pMesh = new CSGMesh*[*nMesh];
        memcpy(pMesh, meshList.data(), sizeof(CSGMesh*)*(*nMesh));
        return pRes;
    }

	static CSGTreeNode* ConvertToPositiveTree(const CSGTreeNode* root, bool inverse, uint level, uint& maxLvl, Octree* pOctree)
	{
		CSGTreeNode* res = new CSGTreeNode;
		if (!root->pLeft && !root->pRight) // it is a leaf node
		{
			res->bInverse = inverse;
			res->pMesh = root->pMesh;
			res->pMesh->bInverse = res->bInverse;
			maxLvl = (maxLvl < level)? level:maxLvl;
		}
		else
		{
			uint Ldepth(0), Rdepth(0);
			if (root->Operation == OP_DIFF)
			{
				res->Operation = OP_INTERSECT;
				res->pLeft = ConvertToPositiveTree(root->pLeft, inverse, level+1, Ldepth, pOctree);
				res->pRight = ConvertToPositiveTree(root->pRight, !inverse, level+1, Rdepth, pOctree);
			}
			else
			{
				res->Operation = root->Operation;
				res->pLeft = ConvertToPositiveTree(root->pLeft, inverse, level+1, Ldepth, pOctree);
				res->pRight = ConvertToPositiveTree(root->pRight, !inverse, level+1, Rdepth, pOctree);
			}
			maxLvl = (Ldepth > Rdepth)? Ldepth:Rdepth;
			if (Ldepth < Rdepth)
			{
				auto tmpNode = res->pLeft;
				res->pLeft = res->pRight;
				res->pRight = tmpNode;
			}
		}

		return res;
	}


	CSGTree* ConvertToPositiveTree(const CSGTree* myTree, Octree* pOctree)
	{
		CSGTree* result = new CSGTree;
		uint maxLvl = 0;
		result->pRoot = ConvertToPositiveTree(myTree->pRoot, false, 0, maxLvl, pOctree);
		return result;
	}

	static void GetLeafList(CSGTreeNode* root, CSGTreeNode** arr)
	{
		if (root->pMesh)
		{
			assert(!arr[root->pMesh->ID]);
			arr[root->pMesh->ID] = root;
		}
		else
		{
			GetLeafList(root->pLeft, arr);
			GetLeafList(root->pRight, arr);
		}
	}


	CSGTreeNode** GetLeafList(CSGTree* tree, uint num)
	{
		CSGTreeNode **arr = new CSGTreeNode*[num];
		memset(arr, 0, sizeof(CSGTreeNode*)*num);
		GetLeafList(tree->pRoot, arr);
		return arr;
	}


}  // namespace CSG

