#include "precompile.h"
#include "BinaryTree.h"
#include "CSGExprNode.h"
#include <cassert>
#include <climits>

// TO-DO: we need method called findLeaf(uint Id)
// to instead GetLeafList;


#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif


#include "MPMesh.h"

namespace CSG
{
	CSGTreeNode::CSGTreeNode():
		Type(TYPE_UNKNOWN), Parent(0),
		pLeft(0), pRight(0), pMesh(0), 
		bInverse(false)
	{
	}

	CSGTreeNode::~CSGTreeNode()
	{
		SAFE_RELEASE(pLeft);
		SAFE_RELEASE(pRight);
	}

	CSGTree::CSGTree():
		pRoot(nullptr)
	{
	}

	CSGTree::~CSGTree()
	{
		SAFE_RELEASE(pRoot);
	}


    static void ConvertCSGTreeNode(GS::CSGExprNode* input, CSGTreeNode** pRoot, std::vector<MPMesh*>& meshList)
    {
        if (!input) return;

        CSGTreeNode*& root = *pRoot;
        root = new CSGTreeNode;

        GS::BaseMesh* pBaseMesh = input->GetMesh();
        if (pBaseMesh)
        {
            root->pMesh = ConvertToMPMesh(pBaseMesh);
            root->pMesh->ID = meshList.size();
			root->Type = TYPE_LEAF;
            meshList.emplace_back(root->pMesh);
			return;
        }
        
        switch (input->BoolOperation())
        {
        case GS::CSGUnion:
            root->Type = TYPE_UNION;
            break;
        case GS::CSGIntersect:
            root->Type = TYPE_INTERSECT;
            break;
        case GS::CSGDiff:
            root->Type = TYPE_DIFF;
            break;
        default:
            root->Type = TYPE_UNKNOWN;
            break;
        }

        ConvertCSGTreeNode(input->LeftNode(), &root->pLeft, meshList);
        ConvertCSGTreeNode(input->RightNode(), &root->pRight, meshList);

		if (root->pLeft) root->pLeft->Parent = root;
		if (root->pRight) root->pRight->Parent = root;
    }


    CSGTree* ConvertCSGTree(GS::CSGExprNode* root, MPMesh*** arrMesh, int *nMesh) // convert nodes.
    {
        if (!root) return NULL;

        CSGTree* pRes = new CSGTree;
        pRes->pRoot = NULL;
        std::vector<MPMesh*> meshList;

        ConvertCSGTreeNode(root, &pRes->pRoot, meshList);

        MPMesh**& pMesh = *arrMesh;
        *nMesh = meshList.size();
        pMesh = new MPMesh*[*nMesh];
        memcpy(pMesh, meshList.data(), sizeof(CSGMesh*)*(*nMesh));
        return pRes;
    }

	static CSGTreeNode* ConvertToPositiveTree(const CSGTreeNode* root, bool inverse, uint level, uint& maxLvl)
	{
		CSGTreeNode* res = new CSGTreeNode;
		if (root->Type == TYPE_LEAF) // it is a leaf node
		{
			res->bInverse = inverse;
			res->Type = TYPE_LEAF;
			res->pMesh = root->pMesh;
			res->pMesh->bInverse = res->bInverse;
			maxLvl = (maxLvl < level)? level:maxLvl;
		}
		else
		{
			uint Ldepth(0), Rdepth(0);
			if (root->Type == TYPE_DIFF)
			{
				res->Type = TYPE_INTERSECT;
				res->pLeft = ConvertToPositiveTree(root->pLeft, inverse, level+1, Ldepth);
				res->pRight = ConvertToPositiveTree(root->pRight, !inverse, level+1, Rdepth);
			}
			else
			{
				res->Type = root->Type;
				res->pLeft = ConvertToPositiveTree(root->pLeft, inverse, level+1, Ldepth);
				res->pRight = ConvertToPositiveTree(root->pRight, !inverse, level+1, Rdepth);
			}
			maxLvl = (Ldepth > Rdepth)? Ldepth:Rdepth;
			if (Ldepth < Rdepth)
			{
				auto tmpNode = res->pLeft;
				res->pLeft = res->pRight;
				res->pRight = tmpNode;
			}

			if (res->pLeft) res->pLeft->Parent = res;
			if (res->pRight) res->pRight->Parent = res;
		}

		return res;
	}

	static void GetLeafList(CSGTreeNode* root, decltype(CSGTree::Leaves)& leaves)
	{
		if (root->Type == TYPE_LEAF)
		{
			leaves[root->pMesh->ID] = root;
		}
		else
		{
			GetLeafList(root->pLeft, leaves);
			GetLeafList(root->pRight, leaves);
		}
	}


	static void GetLeafList(CSGTree* tree)
	{
		tree->Leaves.clear();
		if (!tree->pRoot) return;
		GetLeafList(tree->pRoot, tree->Leaves);
	}

	CSGTree* ConvertToPositiveTree(const CSGTree* myTree)
	{
		CSGTree* result = new CSGTree;
		uint maxLvl = 0;
		result->pRoot = ConvertToPositiveTree(myTree->pRoot, false, 0, maxLvl);
		GetLeafList(result);
		return result;
	}

	static CSGTreeNode* copy(const CSGTreeNode* thiz)
	{
		if (!thiz) return nullptr;

		CSGTreeNode* pRes = new CSGTreeNode(*thiz);
		pRes->pLeft = copy(thiz->pLeft);
		pRes->pRight = copy(thiz->pRight);

		if (pRes->pLeft) pRes->pLeft->Parent = pRes;
		if (pRes->pRight) pRes->pRight->Parent = pRes;

		return pRes;
	}

    CSGTree* copy(const CSGTree* thiz)
	{
		if (!thiz) return nullptr;

		CSGTree* pCopy = new CSGTree;
		pCopy->pRoot = copy(thiz->pRoot);
		GetLeafList(pCopy);
		return pCopy;
	}

	static Relation CompressCSGTreeWithInside(CSGTree* tree, uint Id)
	{
		auto leaf = tree->Leaves[Id];

		CSGTreeNode *curPtr = leaf, *parent = leaf->Parent;
		while (parent && parent->Type == TYPE_UNION)
		{
			curPtr = parent;
			parent = parent->Parent;
		}

		if (!parent) 
		{
			delete tree->pRoot;
			tree->Leaves.clear();
			tree->pRoot = nullptr;
			return REL_INSIDE;
		}

		assert(parent->Type == TYPE_INTERSECT);
		int resCur = LeftOrRight(curPtr);
		int resPar = LeftOrRight(parent);

		if (resPar == 0) // root
		{
			if (resCur < 0)
			{
				tree->pRoot = parent->pRight;
				parent->pRight = nullptr;
				delete parent;
			}
			else 
			{
				assert(resCur);
				tree->pRoot = parent->pLeft;
				parent->pLeft = nullptr;
				delete parent;
			}
			tree->pRoot->Parent = nullptr;
		}
		else if (resPar < 0)
		{
			if (resCur < 0)
			{
				parent->pRight->Parent = parent->Parent;
				parent->Parent->pLeft = parent->pRight;
				parent->pRight = nullptr;
				delete parent;
			}
			else 
			{
				assert(resCur);
				parent->pLeft->Parent = parent->Parent;
				parent->Parent->pLeft = parent->pLeft;
				parent->pLeft = nullptr;
				delete parent;
			}
		}
		else
		{
			if (resCur < 0)
			{
				parent->pRight->Parent = parent->Parent;
				parent->Parent->pRight = parent->pRight;
				parent->pRight = nullptr;
				delete parent;
			}
			else 
			{
				assert(resCur);
				parent->pLeft->Parent = parent->Parent;
				parent->Parent->pRight = parent->pLeft;
				parent->pLeft = nullptr;
				delete parent;
			}
		}
		GetLeafList(tree);
		return REL_UNKNOWN;
	}

	static Relation CompressCSGTreeWithOutside(CSGTree* tree, uint Id)
	{
		auto leaf = tree->Leaves[Id];
		CSGTreeNode *curPtr = leaf, *parent = leaf->Parent;
		while (parent && parent->Type == TYPE_INTERSECT)
		{
			curPtr = parent;
			parent = parent->Parent;
		}

		if (!parent) 
		{
			delete tree->pRoot;
			tree->pRoot = nullptr;
			tree->Leaves.clear();
			return REL_OUTSIDE;
		}

		assert(parent->Type == TYPE_UNION);
		int resCur = LeftOrRight(curPtr);
		int resPar = LeftOrRight(parent);

		if (resPar == 0) // root
		{
			if (resCur < 0)
			{
				tree->pRoot = parent->pRight;
				parent->pRight = nullptr;
				delete parent;
			}
			else 
			{
				assert(resCur);
				tree->pRoot = parent->pLeft;
				parent->pLeft = nullptr;
				delete parent;
			}
			tree->pRoot->Parent = nullptr;
		}
		else if (resPar < 0)
		{
			if (resCur < 0)
			{
				parent->pRight->Parent = parent->Parent;
				parent->Parent->pLeft = parent->pRight;
				parent->pRight = nullptr;
				delete parent;
			}
			else 
			{
				assert(resCur);
				parent->pLeft->Parent = parent->Parent;
				parent->Parent->pLeft = parent->pLeft;
				parent->pLeft = nullptr;
				delete parent;
			}
		}
		else
		{
			if (resCur < 0)
			{
				parent->pRight->Parent = parent->Parent;
				parent->Parent->pRight = parent->pRight;
				parent->pRight = nullptr;
				delete parent;
			}
			else 
			{
				assert(resCur);
				parent->pLeft->Parent = parent->Parent;
				parent->Parent->pRight = parent->pLeft;
				parent->pLeft = nullptr;
				delete parent;
			}
		}
		GetLeafList(tree);
		return REL_UNKNOWN;
	}
	
	Relation CompressCSGTree(CSGTree* tree, uint Id, Relation rel)
	{
		// T∩N⇒N,  F∪N⇒N, F∩N⇒F, and T∪N⇒T
		if (tree->Leaves.find(Id) == tree->Leaves.end())
			return REL_UNKNOWN;

		switch (rel)
		{
		case REL_INSIDE:
			return CompressCSGTreeWithInside(tree, Id);
			break;
		case REL_OUTSIDE:
			return CompressCSGTreeWithOutside(tree, Id);
			break;
		default:
			assert(0);
			break;
		}

		return REL_UNKNOWN;
	}

}  // namespace CSG

