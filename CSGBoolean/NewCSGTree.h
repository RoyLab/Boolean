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

    enum Component
    {
        CPN_NONE  =  0,
        CPN_INSIDE   =  1,
        CPN_OUTSIDE  =  2,
        CPN_SAME     =  4,
        CPN_OPPOSITE =  8
    };

    struct CSGTreeNode
    {
        NodeOp Operation;
        CSGTreeNode *pLeft, *pRight;

        CSGMesh* pMesh;

        CSGTreeNode():pLeft(0), pRight(0), pMesh(0){}
    };

    struct CSGTree
    {
        CSGTreeNode* pRoot;
    };

    CSGTree* ConvertCSGTree(GS::CSGExprNode* root, CSGMesh*** arrMesh, int *nMes); // convert nodes.
    
} // namespace CSG

