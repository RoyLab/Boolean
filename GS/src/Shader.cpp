#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Shader.h"

namespace GS{

Shader::Shader()
	: m_pixelShader(NULL)
	, m_vertexShader(NULL)
	, m_layout(NULL)
{
}


 Shader::~Shader()
 {
	 Release();
 }




void Shader::Release()
{
	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}

bool   Shader::Initialize(ID3D11Device* device, HWND hwnd, WCHAR* vsFileName, LPCSTR vsName, WCHAR* psFileName,  LPCSTR psName )
{

	HRESULT result;
	ID3D10Blob* errorMessage = 0;
	ID3D10Blob* vertexShaderBuffer = 0;
	ID3D10Blob* pixelShaderBuffer = 0 ;
	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFileName, NULL, NULL, vsName, "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS|D3D10_SHADER_DEBUG|D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		HandleCompilingError(errorMessage, vsFileName);
		return false;
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFileName, NULL, NULL, psName, "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS|D3D10_SHADER_DEBUG|D3D10_SHADER_SKIP_OPTIMIZATION, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		HandleCompilingError(errorMessage, psFileName);
		return false;
	}
	    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		vertexShaderBuffer->Release();
		return false;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		vertexShaderBuffer->Release();
		pixelShaderBuffer->Release();
		return false;
	}
	int nElements =0;
	D3D11_INPUT_ELEMENT_DESC* pInputDesc = NULL;
	GetInputLayerout(&pInputDesc, nElements);
	// Create the vertex input layout.
	if (nElements)
	{
		result = device->CreateInputLayout(pInputDesc, nElements, vertexShaderBuffer->GetBufferPointer(), 
									        vertexShaderBuffer->GetBufferSize(), &m_layout);
		delete pInputDesc;
		if(FAILED(result))
		{
			vertexShaderBuffer->Release();
			pixelShaderBuffer->Release();
			return false;
		}
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	pixelShaderBuffer->Release();
	return true;
   
}

void Shader::Render(ID3D11DeviceContext* deviceContext)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	
	return;
}


void Shader::HandleCompilingError(ID3D10Blob* ErrorMessage, WCHAR* FileName)
{
	char* pErrMsg ;
	int size = 0; 
	if(!ErrorMessage)
	{
		char fileName[64];
		wcstombs(fileName,FileName, wcslen(FileName)+1);
		char errMsg[128];
		sprintf_s(errMsg, _countof(errMsg), "Missing Shader File %s\n", fileName);
		pErrMsg = (char*)errMsg; 
		size = strlen(pErrMsg); 
	}
	else 
	{
		pErrMsg = (char*)ErrorMessage->GetBufferPointer();
		size = ErrorMessage->GetBufferSize();
	}
	// Open a file to write the error message to.
	std::ofstream fout;
	fout.open("shader-error.txt");

	// Write out the error message.
	for(int i=0; i<size; i++)
	{
		fout << (pErrMsg)[i];
	}
	// Close the file.
	fout.close();
}


};