#include "BinaryTree.h"
#include "CSGExprNode.h"
#include "CMesh.h"
#include <vector>
#include <assert.h>

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

	static CSGTreeNode* ConvertToPositiveTree(const CSGTreeNode* root, bool inverse)
	{
		CSGTreeNode* res = new CSGTreeNode;
		if (!root->pLeft && !root->pRight)
		{
			res->bInverse = inverse;
			res->pMesh = root->pMesh;
		}
		else
		{
			if (root->Operation == OP_DIFF)
			{
				res->Operation = OP_INTERSECT;
				res->pLeft = ConvertToPositiveTree(root->pLeft, inverse);
				res->pRight = ConvertToPositiveTree(root->pRight, !inverse);
			}
			else
			{
				res->Operation = root->Operation;
				res->pLeft = ConvertToPositiveTree(root->pLeft, inverse);
				res->pRight = ConvertToPositiveTree(root->pRight, !inverse);
			}
		}
		
		return res;
	}


	CSGTree* ConvertToPositiveTree(const CSGTree* myTree)
	{
		CSGTree* result = new CSGTree;
		result->pRoot = ConvertToPositiveTree(myTree->pRoot, false);
		return result;
	}

	

}  // namespace CSG

