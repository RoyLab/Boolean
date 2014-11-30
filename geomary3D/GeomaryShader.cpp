#include "stdafx.h"
#include "Geomary3DShader.h"
#include "typedefs.h"
#include "transform.h"

 
CGeomary3DShader::CGeomary3DShader()
	:mpD3DMatrixCB(NULL)
	,mpLightCB(NULL)
	,mpCamPosCB(NULL)
{
}

CGeomary3DShader::~CGeomary3DShader()
{
	Release();
}

void CGeomary3DShader::Release()
{
	if (mpD3DMatrixCB)
	{
		delete mpD3DMatrixCB;
		mpD3DMatrixCB = NULL;
	}
	if(mpLightCB)
	{
		delete mpLightCB;
		mpLightCB = NULL;
	}
	if(mpCamPosCB)
	{
		delete mpCamPosCB;
		mpCamPosCB = NULL;
	}
	GS::Shader::Release();
}

bool CGeomary3DShader::Initialize(ID3D11Device* device, HWND hwnd)
{

	bool bResult = GS::Shader::Initialize(device, hwnd, L"./shaders/BasicMaterial.fx", "VS", 
		         L"./shaders/BasicMaterial.fx", "PS");
	if (!bResult)
		return false ;
	mpD3DMatrixCB = new GS::D3D11Buffer(GS::Dynamic, GS::Const);
    bResult = mpD3DMatrixCB->CreateDeviceBuffer(device, NULL,  sizeof(MatrixBufferType));
	if (!bResult)
		return false ;
    mpLightCB = new GS::D3D11Buffer(GS::Dynamic, GS::Const);
	bResult =   mpLightCB->CreateDeviceBuffer(device, NULL,  sizeof(MatrixBufferType));
	if (!bResult)
		return false ;

	mpCamPosCB = new GS::D3D11Buffer(GS::Dynamic, GS::Const);
	return  mpCamPosCB->CreateDeviceBuffer(device, NULL,  sizeof(CameraBufferType));

}


//void CGeomary3DShader::SetShaderParameters(ID3D11DeviceContext* dc,  D3DXMATRIX worldMtx,
//										     D3DXMATRIX viewMtx,  D3DXMATRIX projMtx )
//{
//	MatrixBufferType* pData =(MatrixBufferType*) mpD3DCB->MapDeviceBuffer(dc, GS::WriteDiscard, 0, sizeof(MatrixBufferType));
//	if (!pData )
//		return ;
//	
//	// Transpose the matrices to prepare them for the shader.
//	D3DXMatrixTranspose(&worldMtx, &worldMtx);
//	D3DXMatrixTranspose(&viewMtx, &viewMtx);
//	D3DXMatrixTranspose(&projMtx, &projMtx);
//	pData->world = worldMtx;
//	pData->view = viewMtx;
//	pData->projection = projMtx;
//	mpD3DCB->UnMapDeviceBuffer(dc);
//	ID3D11Buffer* pCB = mpD3DCB->GetDeviceBuffer();
//	dc->VSSetConstantBuffers(0, 1, &pCB);
//
//}

void CGeomary3DShader::SetShaderMatrix(ID3D11DeviceContext* dc, const GS::float4x4& worldMtx,
									   const GS::float4x4& viewMtx, const GS::float4x4& projMtx, const GS::float3& camPos)
{
	if (!mpD3DMatrixCB || !mpCamPosCB)
			return;
	MatrixBufferType* pData =(MatrixBufferType*) mpD3DMatrixCB->MapDeviceBuffer(dc, GS::WriteDiscard, 0, sizeof(MatrixBufferType));
	if (!pData )
		return ;
	memcpy_s(&pData->world, sizeof (D3DXMATRIX), &GS::transpose(worldMtx), sizeof (GS::float4x4));
	memcpy_s(&pData->view, sizeof (D3DXMATRIX), &GS::transpose(viewMtx), sizeof (GS::float4x4));
	memcpy_s(&pData->projection, sizeof (D3DXMATRIX), &GS::transpose(projMtx), sizeof (GS::float4x4));
	mpD3DMatrixCB->UnMapDeviceBuffer(dc);
	ID3D11Buffer* pCB = mpD3DMatrixCB->GetDeviceBuffer();
	dc->VSSetConstantBuffers(0, 1, &pCB);

	CameraBufferType* pCamData =(CameraBufferType*) mpCamPosCB->MapDeviceBuffer(dc, GS::WriteDiscard, 0, sizeof(CameraBufferType));
	if (!pCamData )
		return ;
	memcpy_s(&pCamData->cameraPosition, sizeof (D3DXVECTOR3), &camPos, sizeof (GS::float3));
	pCamData->padding = 0.0;
	mpCamPosCB->UnMapDeviceBuffer(dc);
	ID3D11Buffer* pCB1 = mpCamPosCB->GetDeviceBuffer();
	dc->VSSetConstantBuffers(1, 1, &pCB1);
	
}

void CGeomary3DShader::SetShaderLights(ID3D11DeviceContext* dc, const GS::float4& ambient, const GS::float4& diffuse1, 
									   const GS::float4& specular1, const GS::float3& lightDir1, const GS::float4& diffuse2,
										const GS::float3& lightDir2, float specularPower)
{
	if (!mpLightCB)
		return ;
	LightBufferType* pLightBuffer = (LightBufferType*)mpLightCB->MapDeviceBuffer(dc, GS::WriteDiscard, 0, sizeof(LightBufferType));
	if (!pLightBuffer)
		return;
	memset(pLightBuffer,0, sizeof(LightBufferType));
	memcpy_s(&pLightBuffer->ambientColor, sizeof(D3DXVECTOR4), &ambient, sizeof(GS::float4));
	pLightBuffer->specularPower = specularPower;
	memcpy_s(&pLightBuffer->diffuseColor[0], sizeof(D3DXVECTOR4), &diffuse1, sizeof(GS::float4));
	memcpy_s(&pLightBuffer->diffuseColor[1], sizeof(D3DXVECTOR4), &diffuse2, sizeof(GS::float4));
	memcpy_s(&pLightBuffer->lightDirection[0], sizeof(D3DXVECTOR4), &lightDir1, sizeof(GS::float3));
	memcpy_s(&pLightBuffer->lightDirection[1], sizeof(D3DXVECTOR4), &lightDir2, sizeof(GS::float3));
	memcpy_s(&pLightBuffer->specularColor[0], sizeof(D3DXVECTOR4), &specular1, sizeof(GS::float4));
	memcpy_s(&pLightBuffer->specularColor[1], sizeof(D3DXVECTOR4), &specular1, sizeof(GS::float4));

	mpLightCB->UnMapDeviceBuffer(dc);
	ID3D11Buffer* pCB = mpLightCB->GetDeviceBuffer();
	dc->PSSetConstantBuffers(0, 1, &pCB);
}


void CGeomary3DShader::GetInputLayerout(D3D11_INPUT_ELEMENT_DESC** pInputDesc, int &nElements)
{
	*pInputDesc = new  D3D11_INPUT_ELEMENT_DESC[3];
	(*pInputDesc)[0].SemanticName = "POSITION";
	(*pInputDesc)[0].SemanticIndex = 0;
	(*pInputDesc)[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	(*pInputDesc)[0].InputSlot = 0;
	(*pInputDesc)[0].AlignedByteOffset = 0;
	(*pInputDesc)[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	(*pInputDesc)[0].InstanceDataStepRate = 0;

	(*pInputDesc)[1].SemanticName = "COLOR";
	(*pInputDesc)[1].SemanticIndex = 0;
	(*pInputDesc)[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	(*pInputDesc)[1].InputSlot = 0;
	(*pInputDesc)[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	(*pInputDesc)[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	(*pInputDesc)[1].InstanceDataStepRate = 0;

	(*pInputDesc)[2].SemanticName = "NORMAL";
	(*pInputDesc)[2].SemanticIndex = 0;
	(*pInputDesc)[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	(*pInputDesc)[2].InputSlot = 0;
	(*pInputDesc)[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	(*pInputDesc)[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	(*pInputDesc)[2].InstanceDataStepRate = 0;
	 nElements =3;


}