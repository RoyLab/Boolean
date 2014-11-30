#ifndef _BSP_BOOL_OP_H
#define _BSP_BOOL_OP_H
#include "BoolOp.h"
namespace GS{


class BSPBoolOp : public BoolOp{
 
public : 
    static BoolOp* GetInstance();
    virtual ~BSPBoolOp();

protected :
    virtual BaseMesh* ComputeBoolean(BaseMesh* mesh1, BaseMesh* mesh2, BOOL_OP op); 
    virtual BaseMesh* Evalute(std::vector<BaseMesh*>& meshList, std::string& postfix) ;
    static  BaseMesh* ComputeBoolean(FixedPlaneMesh* mesh1, FixedPlaneMesh* mesh2, BOOL_OP op);
    static  BaseMesh* ComputeBoolean(PlaneMesh* mesh1, PlaneMesh* mesh2, BOOL_OP op);
protected :
    BSPBoolOp() {};

};


class LBSPBoolOp: public BSPBoolOp{
public : 
    static BoolOp* GetInstance();
    virtual ~LBSPBoolOp();
protected:
    virtual BaseMesh* ComputeBoolean(BaseMesh* mesh1,  BaseMesh* mesh2, BOOL_OP op);
    virtual BaseMesh*  Evalute(std::vector<BaseMesh*>& meshList, std::string& postfix) ;
    
private :
    LBSPBoolOp() {};

};

}

#endif 

