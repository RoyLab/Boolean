#ifndef _FONT_H_
#define _FONT_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <fstream>
#include "typedefs.h"
#include "Texture.h"

namespace GS{
////////////////////////////////////////////////////////////////////////////////
// Class name: FontClass
////////////////////////////////////////////////////////////////////////////////
class Font
{
private:
	struct FontType
	{
		float left, right;
		int size;
	};

	struct VertexType
	{
		float3 position;
	    float2 texture;
	};

public:
	Font();
	Font(const Font&);
	~Font();

	bool Initialize(ID3D11Device*, char*, WCHAR*);
	void Relase();

	ID3D11ShaderResourceView* GetTexture() {return m_Texture->GetTexture();}

	void BuildVertexArray(void*, char*, float, float);

private:
	bool LoadFontData(char*);
	void ReleaseFontData();
	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();

private:
	FontType* m_Font;
	Texture*  m_Texture;
};

}
#endif