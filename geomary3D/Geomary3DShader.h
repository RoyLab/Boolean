#pragma once

#include <d3dx10math.h>
#include "Shader.h"
#include "D3D11Buffer.h"
#include "typedefs.h"
#include "Light.h"

class CGeomary3DShader: public GS::Shader{
private :
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct CameraBufferType
	{
		D3DXVECTOR3 cameraPosition;
		float padding;
	};

	struct LightBufferType{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor[2];
        D3DXVECTOR4 specularColor[2];
		D3DXVECTOR4 lightDirection[2];
		bool        lightActive[2];
		float specularPower;
		D3DXVECTOR3 padding;
	};
 

public :
	  CGeomary3DShader();
	  virtual ~CGeomary3DShader(); 
	  bool Initialize(ID3D11Device* device, HWND hwnd);
	  void SetShaderMatrix(ID3D11DeviceContext* dc, const GS::float4x4& worldMtx,
							   const GS::float4x4& viewMtx, const GS::float4x4& projMtx, const GS::float3& camPos);
	  void SetShaderLights(ID3D11DeviceContext* dc, const GS::float4& ambient, const GS::float4& diffuse1,
					  	   const GS::float4& specular1,const GS::float3& lightDir1, const GS::float4& diffuse2,
						   const GS::float3& lightDir2, float specularPower );
	  virtual void   Release();
protected:
	  void GetInputLayerout(D3D11_INPUT_ELEMENT_DESC**, int& );
private :
	GS::D3D11Buffer* mpD3DMatrixCB;
	GS::D3D11Buffer* mpLightCB;
	GS::D3D11Buffer* mpCamPosCB;

};