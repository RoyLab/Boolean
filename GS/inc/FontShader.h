#ifndef _FONT_SHADER_H
#define _FONT_SHADER_H


#include "D3D11Buffer.h"
#include "Shader.h"
#include "typedefs.h"
#include <d3dx10math.h>

namespace GS{

class FontShader:public Shader{
private :
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};
	struct PixelBufferType
	{
		D3DXVECTOR4 pixelColor;
	};
public : 
	 FontShader();
	  virtual ~FontShader(); 
	  bool Initialize(ID3D11Device* device, HWND hwnd);
	  void SetShaderParameters(ID3D11DeviceContext* dc, const GS::float4x4& worldMtx,
							   const GS::float4x4& viewMtx, const GS::float4x4& projMtx, 
							   ID3D11ShaderResourceView* texture, const float4& pixelColor);
	  virtual void   Render(ID3D11DeviceContext* deviceContext);
	  virtual void   Release();
protected:
	  void GetInputLayerout(D3D11_INPUT_ELEMENT_DESC**, int& );
private :
	ID3D11SamplerState* m_sampleState;
	GS::D3D11Buffer* mpD3DMatrixCB;
	GS::D3D11Buffer* mpD3DPixelBuffer;

};

}
#endif 