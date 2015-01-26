#ifndef _BOOL_OP_H_
#define _BOOL_OP_H_
#include <string>
#include "BaseMesh.h"

namespace GS{

enum BOOL_OP{
    eUnion,
    eIntersect,
    eDiff
};

enum BoolAlgorithm{
    eNewMeshBool,
    eMeshBool,
    eBSPBool,
    eLBSPBool
};

class BoolOp{

public :
    BaseMesh* Union (BaseMesh* mesh1,  BaseMesh* mesh2);
    BaseMesh* Diff (BaseMesh* mesh1,  BaseMesh* mesh2);
    BaseMesh* Intersect (BaseMesh* mesh1,  BaseMesh* mesh2);
    virtual BaseMesh* Execute(std::vector<BaseMesh*>&, const std::string&);
protected : 
    virtual BaseMesh*  ComputeBoolean(BaseMesh* mesh1,  BaseMesh* mesh2, BOOL_OP op) = 0 ;
    virtual BaseMesh*  Evalute(std::vector<BaseMesh*>& meshList, std::string& postfix) =0;
    std::string InfixToPostfix(const std::string& infix);
    bool IsOperator(char c);
    int BoolOp::OperatorPriority(int c);
};


class BoolOpFactory{
public : 
    static BoolOp* GetBoolOpObj(BoolAlgorithm ba);
};


}


#endif 