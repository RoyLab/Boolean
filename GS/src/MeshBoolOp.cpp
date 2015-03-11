#include <stack>
#include "MeshBoolOp.h"
#include "CSGTree.h"
#include "Bool.h"
#pragma comment(lib, "CSGBoolean")

namespace GS{
    

MeshBoolOp:: ~MeshBoolOp()
{

}


BoolOp* MeshBoolOp::GetInstance()
{
    static MeshBoolOp boolOp;
    return &boolOp;
}


BaseMesh*  MeshBoolOp::ComputeBoolean(BaseMesh* mesh1,  BaseMesh* mesh2, BOOL_OP op) 
{
	CSGExprNode* pNode(nullptr);
    switch (op)
    {
        case eUnion:
            pNode = CSGExprNode::Union(mesh1, mesh2);
			break;
        case eIntersect:
            pNode = CSGExprNode::Intersect(mesh1, mesh2);
			break;
        case eDiff:
            pNode = CSGExprNode::Diff(mesh1, mesh2);
			break;
        default : 
            assert(0);
            return NULL;
    }

	BaseMesh* res = DoCompute(pNode);
	return res;
}


BaseMesh* MeshBoolOp::DoCompute(CSGExprNode* node)
{
	CSGTree* tree =  new CSGTree();
	tree->AddExpr(node);
	tree->Evaluate1();
	BaseMesh* result = tree->GetResultCopy(node);
	delete tree;
	return result;
}



BaseMesh*  MeshBoolOp::Evalute(std::vector<BaseMesh*>& meshList, std::string& postfix)
{
    //construct CSGExproNode 
    CSGExprNode* pNode = EvalutePostfix(meshList, postfix);
    BaseMesh* res = DoCompute(pNode);
	return res;
}


CSGExprNode* MeshBoolOp::EvalutePostfix( std::vector<BaseMesh*>& meshList, std::string& postfix)
{
    std::stack<CSGExprNode*> operandstack;
    int i = 0 ;
   
    while (i < postfix.size())
    {
        int j = 0 ;
        char number[16];
        while (i< postfix.size())
        {
            if (postfix[i] == ' ' || IsOperator(postfix[i]) )
            {   
                if (j) 
                {  
                    number[j] = 0;
                    int idx = atoi(number);
                    assert(idx < meshList.size());
                    operandstack.push(new CSGExprNode(meshList[idx]));
                    j = 0;
                }
                if (IsOperator(postfix[i]))
                    break;
                i++;
            }else number[j++] = postfix[i++];    
         }
        if ( i < postfix.size())
        {
            CSGExprNode* right = operandstack.top();
            operandstack.pop();
            CSGExprNode* left  = operandstack.top();
            operandstack.pop();
            switch(postfix[i])
            {
                case '+':
                    operandstack.push(CSGExprNode::Union(left, right));
                    break;
                case '*':
                    operandstack.push(CSGExprNode::Intersect(left, right)); 
                    break;
                case '-':
                    operandstack.push(CSGExprNode::Diff(left, right)); 
                    break;
                default: 
                    assert(0);
            }
            i++;
       }
      
            
    }
    CSGExprNode* pResult = operandstack.top();
    operandstack.pop();
    return pResult;
}



}
 