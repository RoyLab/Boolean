#include "BSPBoolOp.h"
#include "PlaneMesh.h"
#include "BSPTree.h"
#include "OctTree.h"
#include "FixedPlaneMesh.h"
#include "FixedBSPTree.h"
#include <iostream>
#include <ctime>
#include "BSPOctTree.h"

namespace GS{

static void GSOutputTimeLog(const wchar_t* ch)
{
#ifdef _DEBUG
	std::wstring debug;
	debug += ch;
	debug += std::to_wstring(clock());
	debug += L"\n";
	OutputDebugString(debug.c_str());
#endif 
}


BSPBoolOp:: ~BSPBoolOp()
{

}


BoolOp* BSPBoolOp::GetInstance()
{
    static BSPBoolOp boolOp;
    return &boolOp;
}
//#define SWITCH_WR
#ifdef SWITCH_WR
BaseMesh* BSPBoolOp::ComputeBoolean(BaseMesh* mesh1, BaseMesh* mesh2, BOOL_OP op)
{
    PlaneMesh* pPlaneMesh1 = mesh1->ToPlaneMesh();
    PlaneMesh* pPlaneMesh2 = mesh2->ToPlaneMesh();
    BaseMesh* pMesh = ComputeBoolean(pPlaneMesh1, pPlaneMesh2, op);
    delete pPlaneMesh1;
    delete pPlaneMesh2;
    return pMesh;
}
#else
BaseMesh* BSPBoolOp::ComputeBoolean(BaseMesh* mesh1, BaseMesh* mesh2, BOOL_OP op)
{
	Box3 bbox(mesh1->AABB());
	bbox.IncludeBox(mesh2->AABB());
    
    mesh1->NormalizeCoord(&bbox);
    mesh2->NormalizeCoord(&bbox);

    FixedPlaneMesh* pPlaneMesh1 = ToFixedPlaneMesh(mesh1);
    FixedPlaneMesh* pPlaneMesh2 = ToFixedPlaneMesh(mesh2);
    BaseMesh* pMesh = ComputeBoolean(pPlaneMesh1, pPlaneMesh2, op);
    delete pPlaneMesh1;
    delete pPlaneMesh2;

    mesh1->DenormalizeCoord();
    mesh2->DenormalizeCoord();
    return pMesh;
}
#endif
BaseMesh* BSPBoolOp::ComputeBoolean(FixedPlaneMesh* mesh1, FixedPlaneMesh* mesh2, BOOL_OP op)
{
	GSOutputTimeLog(L"start: ");
    FixedBSPTree::SET_OP BSPOp;
    switch (op)
    {
        case eUnion:
            BSPOp = FixedBSPTree::OP_UNION;
            break;
        case eIntersect:
            BSPOp = FixedBSPTree::OP_INTERSECT;
            break;
        case eDiff:
            BSPOp = FixedBSPTree::OP_DIFFERENCE;
            break;
        default :
            break;
    }

	Box3 bbox(-1.5, -1.5, -1.5, 1.5, 1.5, 1.5);
    FixedBSPTree* pTree1 =  mesh1->ToBSPTree();
	GSOutputTimeLog(L"tree1, build: ");

    pTree1->FormSubHyperPlane(bbox);
	//pTree1->OutputDebugInfo("D:\\x.txt");
     
	GSOutputTimeLog(L"tree1, sp: ");

    FixedBSPTree* pTree2 = mesh2->ToBSPTree();
	GSOutputTimeLog(L"tree2, build: ");
    if (BSPOp == FixedBSPTree::OP_DIFFERENCE)
    {
        pTree2->Negate();
        BSPOp = FixedBSPTree::OP_INTERSECT;
    }
    pTree2->FormSubHyperPlane(bbox);
	GSOutputTimeLog(L"tree2, sp: ");
	//pTree2->OutputDebugInfo("D:\\y.txt");

    FixedBSPTree* pResultTree = pTree1->Merge(pTree2, BSPOp);
	GSOutputTimeLog(L"merged: ");
	//pResultTree->OutputDebugInfo("D:\\z.txt");
    delete pTree2;
    delete pTree1;
    //FixedPlaneMesh* pPlaneMesh1 = new FixedPlaneMesh(pTree1, mesh1->Color());//debug2

    // delete pTree1;//debug2
    FixedPlaneMesh* pPlaneMesh1 = new FixedPlaneMesh(pResultTree, mesh1->Color());
    delete pResultTree;
	GSOutputTimeLog(L"toPlaneMesh: ");

	pPlaneMesh1->SetAABB(mesh1->AABB());
    BaseMesh* pMesh = pPlaneMesh1->ToBaseMesh();
	GSOutputTimeLog(L"toBaseMesh: ");

    pMesh->DenormalizeCoord();
    pMesh->GenAABB(true);
    //BaseMesh* pMesh = mesh1->ToBaseMesh();//debug1
    if (pMesh && pMesh->PrimitiveCount() ==0 )
	{
        delete pMesh;
        pMesh = NULL;
    }
    delete pPlaneMesh1;
    return pMesh;
    return nullptr;
}

BaseMesh* BSPBoolOp::ComputeBoolean(PlaneMesh* mesh1, PlaneMesh* mesh2, BOOL_OP op)
{
    BSPTree::SET_OP BSPOp;
    switch (op)
    {
        case eUnion:
            BSPOp = BSPTree::OP_UNION;
            break;
        case eIntersect:
            BSPOp = BSPTree::OP_INTERSECT;
            break;
        case eDiff:
            BSPOp = BSPTree::OP_DIFFERENCE;
            break;
        default :
            break;
    }
	Box3 bbox(mesh1->AABB());
	bbox.IncludeBox(mesh2->AABB());

    BSPTree* pTree1 =  mesh1->ToBSPTree();
    pTree1->FormSubHyperPlane(bbox);
	pTree1->OutputDebugInfo("D:\\x1.txt");

    BSPTree* pTree2 = mesh2->ToBSPTree();
    if (BSPOp == BSPTree::OP_DIFFERENCE)
    {
        pTree2->Negate();
        BSPOp = BSPTree::OP_INTERSECT;
    }
    pTree2->FormSubHyperPlane(bbox);
	pTree2->OutputDebugInfo("D:\\y1.txt");

    BSPTree* pResultTree = pTree1->Merge(pTree2, BSPOp);
	pResultTree->OutputDebugInfo("D:\\z1.txt");
    delete pTree2;
    delete pTree1;

   PlaneMesh* pPlaneMesh1 = new PlaneMesh(pResultTree);
   //PlaneMesh* pPlaneMesh1 = new PlaneMesh(pTree1);
    //delete pTree1;
    BaseMesh* pMesh = pPlaneMesh1->ToBaseMesh();
    if (pMesh && pMesh->PrimitiveCount() ==0 )
	{
        delete pMesh;
        pMesh = NULL;
    }
    delete pPlaneMesh1;

    return pMesh;
}

BaseMesh*  BSPBoolOp::Evalute(std::vector<BaseMesh*>& meshList, std::string& postfix) 
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////

LBSPBoolOp:: ~LBSPBoolOp()
{
}


BoolOp* LBSPBoolOp::GetInstance()
{
    static LBSPBoolOp boolOp;
    return &boolOp;
}

BaseMesh* LBSPBoolOp::ComputeBoolean(BaseMesh* mesh1,  BaseMesh* mesh2, BOOL_OP op)
{
    
    std::vector<BaseMesh*> meshes(2);
    meshes[0] =mesh1;
    meshes[1]= mesh2;
    OctTree * pOctTree = new OctTree();
    pOctTree->BuildOctTree(meshes);
//    pOctTree->CarveTree();
   // std::vector<FixedPlaneMesh*> criticalMeshes;
    std::vector<BaseMesh*> criticalMeshes;
    std::vector<BaseMesh*> nonecriticalMeshes;
  //  pOctTree->GenMeshesFromCells(criticalMeshes, true);
    pOctTree->GenMeshesFromCells(nonecriticalMeshes, eCritical);
    delete pOctTree;
 //   delete criticalMeshes[1];
    delete nonecriticalMeshes[1];
   /* for (int i = 0; i < nonecriticalMeshes[0]->PrimitiveCount(); i++) 
    {
        const TriInfo&  info= nonecriticalMeshes[0]->TriangleInfo(i);
        criticalMeshes[0]->Add(nonecriticalMeshes[0]->Vertex(info.VertexId[0]) , nonecriticalMeshes[0]->Vertex(info.VertexId[1]), nonecriticalMeshes[0]->Vertex(info.VertexId[2]));
    }*/
    //delete nonecriticalMeshes[0];
    return nonecriticalMeshes[0];


    FixedBSPTree::SET_OP BSPOp;
    switch (op)
    {
        case eUnion:
            BSPOp = FixedBSPTree::OP_UNION;
            break;
        case eIntersect:
            BSPOp = FixedBSPTree::OP_INTERSECT;
            break;
        case eDiff:
            BSPOp = FixedBSPTree::OP_DIFFERENCE;
            break;
        default :
            break;
    }
	BaseMesh *result = new BaseMesh;
	BSPOctree *tree = new BSPOctree(BSPOp);
    tree->BSPOperation(mesh1, mesh2, &result);
	return result;
}

BaseMesh*  LBSPBoolOp::Evalute(std::vector<BaseMesh*>& meshList, std::string& postfix) 
{
    return NULL;
}


}

