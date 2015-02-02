#pragma once

namespace GS
{
    class CSGExprNode;

} // namespace GS

namespace CSG
{
    struct Octree;
    struct CSGMesh;

    enum NodeOp
    {
        OP_UNKNOWN = 0,
        OP_UNION,
        OP_INTERSECT,
        OP_DIFF
    };

    struct CSGTreeNode
    {
        NodeOp Operation;
        CSGTreeNode *pLeft, *pRight;

        CSGMesh* pMesh;
		bool	 bInverse;

        CSGTreeNode():
			pLeft(0), pRight(0), pMesh(0)
			, bInverse(false){}
    };

    struct CSGTree
    {
        CSGTreeNode* pRoot;
    };

    CSGTree* ConvertCSGTree(GS::CSGExprNode* root, CSGMesh*** arrMesh, int *nMes); // convert nodes.
	CSGTree* ConvertToPositiveTree(const CSGTree* myTree);
    
} // namespace CSG

