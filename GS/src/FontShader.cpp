#include "FontShader.h"
#include "transform.h"

namespace GS{

FontShader::FontShader()
	:mpD3DMatrixCB(NULL)
	,mpD3DPixelBuffer(NULL)
	,m_sampleState(NULL)
{
}

FontShader::~FontShader()
{
	Release();
}

void FontShader::Release()
{
	if (mpD3DMatrixCB)
	{
		delete mpD3DMatrixCB;
		mpD3DMatrixCB = NULL;
	}
	if (mpD3DPixelBuffer)
	{
		delete mpD3DPixelBuffer;
		mpD3DPixelBuffer = NULL;
	}
		// Release the sampler state.
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}
	
	GS::Shader::Release();
}

bool FontShader::Initialize(ID3D11Device* device, HWND hwnd)
{

	bool bResult = GS::Shader::Initialize(device, hwnd, L"../GS/shaders/font.vs", "FontVertexShader", 
		         L"../GS/shaders/font.ps", "FontPixelShader");
	if (!bResult)
		return false ;
	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
    HRESULT result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
	{
		return false;
	}
	mpD3DMatrixCB = new GS::D3D11Buffer(GS::Dynamic, GS::Const);
    bResult = mpD3DMatrixCB->CreateDeviceBuffer(device, NULL,  sizeof(MatrixBufferType));
	if (!bResult)
		return false ;
	mpD3DPixelBuffer = new GS::D3D11Buffer(GS::Dynamic, GS::Const);
    bResult = mpD3DPixelBuffer->CreateDeviceBuffer(device, NULL,  sizeof(PixelBufferType));
	if (!bResult)
		return false ;
	return true;
}

void FontShader::GetInputLayerout(D3D11_INPUT_ELEMENT_DESC** pInputDesc, int &nElements)
{
	*pInputDesc = new  D3D11_INPUT_ELEMENT_DESC[2];
	(*pInputDesc)[0].SemanticName = "POSITION";
	(*pInputDesc)[0].SemanticIndex = 0;
	(*pInputDesc)[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	(*pInputDesc)[0].InputSlot = 0;
	(*pInputDesc)[0].AlignedByteOffset = 0;
	(*pInputDesc)[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	(*pInputDesc)[0].InstanceDataStepRate = 0;

	(*pInputDesc)[1].SemanticName = "TEXCOORD";
	(*pInputDesc)[1].SemanticIndex = 0;
	(*pInputDesc)[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	(*pInputDesc)[1].InputSlot = 0;
	(*pInputDesc)[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	(*pInputDesc)[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	(*pInputDesc)[1].InstanceDataStepRate = 0;

	 nElements =2;


}

 void FontShader::SetShaderParameters(ID3D11DeviceContext* dc, const GS::float4x4& worldMtx,
							   const GS::float4x4& viewMtx, const GS::float4x4& projMtx, 
							   ID3D11ShaderResourceView* texture, const float4& pixelColor)
 {

	 if (!mpD3DMatrixCB ||!mpD3DPixelBuffer)
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

	// Set shader texture resource in the pixel shader.
	dc->PSSetShaderResources(0, 1, &texture);

	PixelBufferType* pPixelData =(PixelBufferType*) mpD3DPixelBuffer->MapDeviceBuffer(dc, GS::WriteDiscard, 0, sizeof(PixelBufferType));
	if (!pPixelData )
		return ;
	memcpy_s(&pPixelData->pixelColor, sizeof (D3DXVECTOR3), &pixelColor, sizeof (GS::float3));
	mpD3DPixelBuffer->UnMapDeviceBuffer(dc);
	ID3D11Buffer* pPixelCB = mpD3DPixelBuffer->GetDeviceBuffer();
	dc->PSSetConstantBuffers(0, 1, &pPixelCB);

 }

 void FontShader::Render(ID3D11DeviceContext* deviceContext)
 {
     Shader::Render(deviceContext);
     // Set the sampler state in the pixel shader.
     deviceContext->PSSetSamplers(0, 1, &m_sampleState);
 }

}