#ifndef _CSG_EXPR_NODE_H
#define _CSG_EXPR_NODE_H

#include "BaseMesh.h"
namespace GS{

enum CSGBoolOp{
	CSGUnion,
	CSGDiff,
	CSGIntersect,
	CSGTmpDiff
};

class CSGExprNode {


public:
	CSGExprNode(BaseMesh* pMesh) 
		:mpMesh(pMesh)
		,mpLeftNode(NULL)
		,mpRightNode(NULL)
	{
	}
	~CSGExprNode()
	{
		if (mpLeftNode)
			delete mpLeftNode;
		if (mpRightNode)
			delete mpRightNode;
    }
    CSGBoolOp BoolOperation() const { return meBoolOp;}
    BaseMesh* GetMesh() const {return mpMesh;}
    CSGExprNode* LeftNode () const {return mpLeftNode;}
    CSGExprNode* RightNode() const {return mpRightNode;}
    static CSGExprNode* Union(BaseMesh* left, BaseMesh* right)
    {
        CSGExprNode* leftNode = new CSGExprNode(left);
        CSGExprNode*  rightNode = new CSGExprNode(right);
        return new CSGExprNode(leftNode, rightNode, CSGUnion);
    }

    static CSGExprNode* Union(CSGExprNode* left, CSGExprNode* right)
    {
        return new CSGExprNode(left, right, CSGUnion);
    }

    static CSGExprNode* Diff(BaseMesh* left, BaseMesh* right)
    {
        CSGExprNode* leftNode = new CSGExprNode(left);
        CSGExprNode*  rightNode = new CSGExprNode(right);
        return new CSGExprNode(leftNode, rightNode,CSGDiff);
    }
     static CSGExprNode* Diff(CSGExprNode* left, CSGExprNode* right)
    {
        return new CSGExprNode(left, right,CSGDiff);
    }
	static CSGExprNode* Intersect(BaseMesh* left, BaseMesh* right)
	{
		CSGExprNode* leftNode = new CSGExprNode(left);
		CSGExprNode*  rightNode = new CSGExprNode(right);
		return new CSGExprNode(leftNode, rightNode,CSGIntersect);

	}
    static CSGExprNode* Intersect(CSGExprNode* left, CSGExprNode* right)
    {
        return new CSGExprNode(left, right,CSGIntersect);
    }
private:
	CSGExprNode(CSGExprNode* left, CSGExprNode* right, CSGBoolOp boolOp)
		:mpLeftNode(left)
		,mpRightNode(right)
		,meBoolOp(boolOp)
		,mpMesh(NULL)
	{

	}
private:
	BaseMesh* mpMesh;
	CSGBoolOp meBoolOp;
	CSGExprNode* mpLeftNode;
	CSGExprNode* mpRightNode;
};


}



#endif