#ifndef _SHADER_H
#define _SHADER_H
#include <d3d11.h>
#include <d3dx11async.h>
namespace GS{

class Shader{

public : 
	Shader();
    virtual ~Shader();
	virtual void   Release();
	bool   Initialize(ID3D11Device* device, HWND hwnd, WCHAR* vsFileName, LPCSTR vsName, WCHAR* psFileName,  LPCSTR psName );
	virtual void   Render(ID3D11DeviceContext* deviceContext);
protected :
	virtual void  GetInputLayerout(D3D11_INPUT_ELEMENT_DESC**, int& ) =0;
    void HandleCompilingError(ID3D10Blob* ErrorMessage, WCHAR* FileName);
protected:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader*  m_pixelShader;
	ID3D11InputLayout*  m_layout;
};



}





#endif 