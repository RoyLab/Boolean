#pragma once
#include "meshboolop.h"

namespace GS
{
class NewMeshBoolOp :
    public MeshBoolOp
{
public:
    NewMeshBoolOp(void);
    virtual ~NewMeshBoolOp(void);

    static BoolOp* GetInstance();
protected:
    BaseMesh* DoCompute(CSGExprNode* node);
};
}

