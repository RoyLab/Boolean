#include "Light.h"
namespace GS{

Light::Light(ELightType eType)
	:meType(eType)
	, mbOn(true)
{

	mDiffuseColor= float4(0.38, 0.38, 0.38, 1.0);
	mSpecularColor = float4(0.38, 0.38, 0.38, 1.0);
	mIndensity = 1.0f;
	mSpecularFactor =10.0; 
}

Light::Light(ELightType eType, const float4& diffuse, const float4& specular)
  :meType(eType)
  ,mDiffuseColor(diffuse)
  ,mSpecularColor(specular)
  , mbOn(true)
{
	mIndensity = 1.0f;
	mSpecularFactor = 10.0;
}

DirectionLight::DirectionLight()
	:Light(Directional)
{

}
	
DirectionLight::DirectionLight(const float4& diffuse, const float4& specular, const float3 direct, float indensity )
	: Light(Directional, diffuse, specular)
	, mDirect(direct)
{

}







}

