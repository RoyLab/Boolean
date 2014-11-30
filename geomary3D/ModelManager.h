#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_
#include <vector>
#include "BaseMesh.h"
#include "D3D11Buffer.h"
#include "BoolOp.h"

enum BooleanOP{
    eUnion,
    eIntersect,
    eDifference
};

class CModelManager{
public:
	CModelManager();
	~CModelManager();
	void Add   (GS::BaseMesh* pMesh);
    int  Count () const {return mpMeshList.size();}
    
    void BoolOperation(BooleanOP op, GS::BoolAlgorithm ba);
    void EvaluateBoolExpression(const std::string& expression);

    void CreateOctree();
	void Clear();
	void ClearResults();
	void Invalidate();
	void Render(ID3D11Device* device,ID3D11DeviceContext* deviceContext);
	int  GetPrimiaryCount() const ;
	int  GetVertexCount() const ;

private : 
	std::vector<GS::BaseMesh*> mpMeshList;
	std::vector<GS::Geometry*> mpResultMeshList;
    bool mbInvalidate; 
};


#endif 
