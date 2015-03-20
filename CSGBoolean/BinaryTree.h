#pragma once
#include "COctree.h"
#include <map>

namespace GS
{
    class CSGExprNode;

} // namespace GS

namespace CSG
{
    struct Octree;
    struct MPMesh;

    enum BiNodeType
    {
        TYPE_UNKNOWN = 0,
        TYPE_UNION,
        TYPE_INTERSECT,
        TYPE_DIFF,
		TYPE_LEAF
    };

    struct CSGTreeNode
    {
        BiNodeType Type;
		Relation relation;
        CSGTreeNode *pLeft, *pRight, *Parent;

        MPMesh* pMesh;
		bool	 bInverse;

		CSGTreeNode();
		~CSGTreeNode();
    };

    struct CSGTree
    {
        CSGTreeNode* pRoot;
		std::map<unsigned, CSGTreeNode*> Leaves;

		CSGTree();
		~CSGTree();
    };

    CSGTree* ConvertCSGTree(GS::CSGExprNode* root, MPMesh*** arrMesh, int *nMes); // convert nodes.
	CSGTree* ConvertToPositiveTree(const CSGTree* tree);
	Relation CompressCSGTree(CSGTree* tree, unsigned Id, Relation rel);
	Relation ParsingCSGTree(MPMesh* pMesh, Relation* tab, unsigned nMesh, CSGTree*& curTree);

    inline bool IsLeaf(CSGTreeNode* node) {return !(node->pLeft && node->pRight);}
    CSGTree* copy(const CSGTree* thiz);

	/** 
	if it is a left child, return negative
	if it is a right child, return positive
	if it is a root, return 0
	*/
	inline int LeftOrRight(CSGTreeNode* node)
	{
		assert(node);

		if (!node->Parent) return 0;
		if (node->Parent->pLeft == node) return -1;
		if (node->Parent->pRight == node) return 1;

		assert(0);
		return 0;
	}

} // namespace CSG

