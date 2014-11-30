////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURE_H_
#define _TEXTURE_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx11tex.h>


////////////////////////////////////////////////////////////////////////////////
// Class name: Texture
////////////////////////////////////////////////////////////////////////////////
class Texture
{
public:
	Texture();
	Texture(const Texture&);
	~Texture() {Release();}

	bool Initialize(ID3D11Device*, WCHAR*);
	void Release();

	ID3D11ShaderResourceView* GetTexture() {return m_texture;}

private:
	ID3D11ShaderResourceView* m_texture;
};

inline 
bool Texture::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;

	// Load the texture in.
	result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &m_texture, NULL);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}

inline 
void Texture::Release()
{
	// Release the texture resource.
	if(m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}
}

#endif