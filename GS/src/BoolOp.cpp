#include <stack>
#include "tbb/tick_count.h"
#include "BoolOp.h"
#include "MeshBoolOp.h"
#include "BSPBoolOp.h"
#include "NewMeshBoolOp.h"
namespace GS{


BaseMesh* BoolOp::Union ( BaseMesh* mesh1,  BaseMesh* mesh2)
{
    tbb::tick_count t0 = tbb::tick_count::now();
  
	if (mesh1 == NULL && mesh2 == NULL)
		return NULL;
	if ( mesh1 == NULL)
		return mesh2->Clone();
	if (mesh2 == NULL)
		return mesh1->Clone();
	BaseMesh* result = NULL;
	if (1 ||  mesh1->AABB().Intersects(mesh2->AABB()))
	{
        result = ComputeBoolean(mesh1, mesh2, eUnion);
		if (result && result->PrimitiveCount() ==0 )
		{
			delete result;
			result = NULL;
		}
	}
	else {
		result = mesh1->Clone();
		for (int i = 0; i < mesh2->PrimitiveCount(); i++) 
		{
			const TriInfo&  info= mesh2->TriangleInfo(i);
			result->Add(mesh2->Vertex(info.VertexId[0]) , mesh2->Vertex(info.VertexId[1]), mesh2->Vertex(info.VertexId[2]));
			        
		}
	}

    if (result != NULL)
    {
        result->GenID();
    //    result->GenSurface();
     //   result->GenAABB(true);
    }
   /*  tbb::tick_count t1 = tbb::tick_count::now();
     WCHAR buffer [100];
    ::swprintf_s(buffer, 100, L"Time %f mec", (t1-t0).seconds());
    ::MessageBox(NULL, buffer, L"Warning", MB_OK); */
	return result ; 
	 
}

BaseMesh* BoolOp::Diff (BaseMesh* mesh1,  BaseMesh* mesh2)
{
	BaseMesh* result = NULL;
	if (mesh1 != NULL)
	{
		if (mesh2 == NULL || !(mesh1->AABB().Intersects(mesh2->AABB())))	
			result = mesh1->Clone();
		else{
            result = ComputeBoolean(mesh1, mesh2,eDiff);
			if (result && result->PrimitiveCount() ==0 )
			{
				delete result;
				result = NULL;
			}
		}
	}
	if (result != NULL)
	{
		result->GenID();
	//	result->GenSurface();
	//	result->GenAABB(true);
	}
	return result ; 

}
 
BaseMesh* BoolOp::Intersect (BaseMesh* mesh1,  BaseMesh* mesh2)
{
	BaseMesh* result = NULL;
	if (mesh1 != NULL && mesh2 != NULL && (mesh1->AABB().Intersects(mesh2->AABB())))
	{
        result = ComputeBoolean(mesh1, mesh2, eIntersect);
		if (result && result->PrimitiveCount() ==0 )
		{
			delete result;
			result = NULL;
		}
	}
	if (result != NULL)
	{
		result->GenID();
	}
	return result ; 
 


}

BaseMesh* BoolOp::Execute(std::vector<BaseMesh*>& meshList, const std::string& expression)
{
    if (meshList.size() == 0)
        return NULL;

    // Parser expression 
    std::string postfix = InfixToPostfix(expression);
    return Evalute(meshList, postfix);
}




std::string BoolOp::InfixToPostfix(const std::string& infix)
{
    std::string postfix ;
    std::stack<char> opstack;
    for (int i = 0; i< infix.size(); i++)
    {
        if (!IsOperator(infix[i]))
        {
            postfix +=infix[i];
            continue;
        }
        postfix +=' ';
        if (opstack.empty() || (opstack.top() == '(' && infix[i] !=')'))
            opstack.push(infix[i]);
        else {
            while (OperatorPriority(infix[i]) <= OperatorPriority(opstack.top()))
            {
               if(opstack.top() != '(')
               {
                    postfix +=opstack.top();
                    opstack.pop();
                    if (opstack.empty())
                        break ; 
               }else
                    break;
            }
            if (infix[i] != ')')
                opstack.push(infix[i]);
            else if (opstack.top() == '(')
                 opstack.pop();
        }
    }
    while (!opstack.empty())
    {
        postfix+= opstack.top();
        opstack.pop();
    }
    return postfix;
}

bool BoolOp::IsOperator(char c)
{
    if ((c == '(') || (c == ')') ||(c == '+')
          || (c == '*') || (c == '-'))
          return true;
    return false ; 
}

int BoolOp::OperatorPriority(int c)
{
    if( c== ')')
        return 0;
    if ((c == '+') || (c == '-'))
        return 1;
    if (c == '*')
        return 2;
    return 3;// '('
}


///////////////////////////////////////////////////////////

BoolOp* BoolOpFactory::GetBoolOpObj(BoolAlgorithm ba)
{
    switch (ba)
    {
        case eNewMeshBool:
            return NewMeshBoolOp::GetInstance();
        case eMeshBool:
            return MeshBoolOp::GetInstance();
        case eBSPBool:
            return BSPBoolOp::GetInstance();
        case eLBSPBool:
            return LBSPBoolOp::GetInstance();
        default: 
            return NULL;
    }
}
    
    


}




