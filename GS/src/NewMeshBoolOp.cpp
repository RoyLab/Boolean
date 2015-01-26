#include "NewMeshBoolOp.h"
#include "Bool.h"

namespace GS
{
NewMeshBoolOp::NewMeshBoolOp(void)
{
}


NewMeshBoolOp::~NewMeshBoolOp(void)
{
}

BoolOp* NewMeshBoolOp::GetInstance()
{
    static NewMeshBoolOp boolOp;
    return &boolOp;
}

BaseMesh* NewMeshBoolOp::DoCompute(CSGExprNode* node)
{
    return CSG::BooleanOperation(node);
}

}
