#include <assert.h>
#include "stdafx.h"
#include "ModelManager.h"
#include "BoolOp.h"
#include "PlaneMesh.h"
#include "BSPTree.h"
#include "OctTree.h"


CModelManager::CModelManager()
	: mbInvalidate(false)
{
	 
}

CModelManager::~CModelManager()
{
	Clear();
}



void CModelManager::Add(GS::BaseMesh* pMesh)
{
	assert(pMesh != NULL);
	mpMeshList.push_back(pMesh);

}

void CModelManager::Clear()
{
	for (int i = 0; i < mpMeshList.size(); i ++)
	{
		delete mpMeshList[i];
	}
	mpMeshList.clear();
	ClearResults();
}

void CModelManager::Invalidate()
{
	mbInvalidate = true;
}



void CModelManager::Render(ID3D11Device* device,ID3D11DeviceContext* deviceContext)
{
	if(mpResultMeshList.size() )
	{
		for(int i = 0 ; i < mpResultMeshList.size(); i++)
			mpResultMeshList[i]->Render(device, deviceContext,false);
		return ; 
	}
	if (mpMeshList.size() ==0 )
		return ; 
	for (int i = 0 ; i < mpMeshList.size();i++)
		mpMeshList[i]->Render(device, deviceContext, mbInvalidate);
	mbInvalidate = false ;

}

void CModelManager::CreateOctree()
{
    if (mpMeshList.size() ==0 )
		return ; 
    ClearResults();
    mpResultMeshList.reserve(mpMeshList.size()+1);
    GS::OctTree* pTree = new GS::OctTree();
    pTree->BuildOctTree(mpMeshList);
    for (int i = 0; i < mpMeshList.size(); i ++)
        mpResultMeshList.push_back( mpMeshList[i]->Clone());
    GS::LineSet* pLineSet = new GS::LineSet();
    pTree->CreateTreeGeometry(*pLineSet, GS::float4(0, 1, 0, 0.8));
    if (pLineSet->PrimitiveCount() >0)
        mpResultMeshList.push_back(pLineSet);
    delete pTree;

    



}



int  CModelManager::GetPrimiaryCount() const 
{
	int count = 0;
	for (int i = 0 ; i< mpMeshList.size(); i++)
	{
		count += mpMeshList[i]->PrimitiveCount();
	}
	return count ; 
}




int  CModelManager::GetVertexCount() const 
{
	int count = 0;
	for (int i = 0 ; i< mpMeshList.size(); i++)
	{
		count += mpMeshList[i]->VertexCount();
	}
	return count ; 
}

void CModelManager::ClearResults()
{
	for (int i = 0; i< mpResultMeshList.size(); i++)
		delete mpResultMeshList[i];
	mpResultMeshList.clear();
}

void CModelManager::BoolOperation(BooleanOP op, GS::BoolAlgorithm ba)
{
	ClearResults();
	if (mpMeshList.size() == 0)
		return ;
	GS::BaseMesh* pMesh1 = mpMeshList[0];
	GS::BaseMesh* pMesh2 = NULL;
	if (mpMeshList.size() > 1)
		pMesh2 = mpMeshList[1];
    GS::BaseMesh* pResult = NULL;
    GS::BoolOp* pBoolOp = NULL;
    switch (ba)
    {
        case GS::eMeshBool:
            pBoolOp = GS::BoolOpFactory::GetBoolOpObj(GS::eMeshBool);
            break;
        case GS::eBSPBool:
             pBoolOp = GS::BoolOpFactory::GetBoolOpObj(GS::eBSPBool);
             break;
        case GS::eLBSPBool:
            pBoolOp = GS::BoolOpFactory::GetBoolOpObj(GS::eLBSPBool);
             break;
    }
    switch (op)
    {
        case eUnion:
            pResult = pBoolOp->Union(pMesh1, pMesh2);
            break;
        case eIntersect:
            pResult = pBoolOp->Intersect(pMesh1, pMesh2);
            break;
        case eDifference:
            pResult = pBoolOp->Diff(pMesh1, pMesh2);
            break;
        default :
            break;
    }
	if (pResult )
		mpResultMeshList.push_back(pResult);

}


void CModelManager::EvaluateBoolExpression(const std::string& expression)
{
    ClearResults();
    //if (mpMeshList.size() < 3)
    //    return ;
    GS::BoolOp* pBoolOp = GS::BoolOpFactory::GetBoolOpObj(GS::eMeshBool);
    GS::BaseMesh* pResult = pBoolOp->Execute(mpMeshList,expression);
	if (pResult )
        mpResultMeshList.push_back(pResult);
	
}




