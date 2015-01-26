#ifndef _MESH_BOOL_OP_H_
#define _MESH_BOOL_OP_H_

#include "BoolOp.h"
#include "CSGExprNode.h"

namespace GS{

class MeshBoolOp : public BoolOp{

public : 
    virtual ~MeshBoolOp();
    static BoolOp* GetInstance() ; 
protected:
    virtual  BaseMesh*  ComputeBoolean(BaseMesh* mesh1,  BaseMesh* mesh2, BOOL_OP op) ;
    virtual  BaseMesh*  Evalute(std::vector<BaseMesh*>& meshList, std::string& postfix);
    virtual  BaseMesh*  DoCompute(CSGExprNode* node);
             CSGExprNode* EvalutePostfix( std::vector<BaseMesh*>& meshList, std::string& postfix);

    MeshBoolOp() {};
};


}

#endif 