#ifndef _LIGHT_H_
#define _LIGHT_H_
#include "typedefs.h"
namespace GS{

enum ELightType {
	Directional, 
	Point,
	Spot
};

class Light{

public : 
	Light(ELightType eType);
	Light(ELightType eType, const float4& diffuse, const float4& specular);
	

	void ActiveLight(bool bOn) { mbOn = bOn;}
	void  SetDiffuseColor(const float4& color) { mDiffuseColor = color;}
	void  SetSpecularColor(const float4& color){ mSpecularColor = color;}
	void SetSpecularFactor(float factor)      {mSpecularFactor = factor;}
	const float4& DiffuseColor() const {return mDiffuseColor; } 
	const float4& SpecularColor() const {return mSpecularColor;} 
	float         SpecularFactor() const { return mSpecularFactor;}
	ELightType    LightType() const  {return meType;}
	bool          LightOn() const {return mbOn;}
protected : 
	float4 mSpecularColor;
	float4 mDiffuseColor;
	float  mIndensity;
	float  mSpecularFactor;
	ELightType meType;
	bool   mbOn;

};

class DirectionLight : public Light{
public :
	DirectionLight();
	DirectionLight(const float4& diffuse, const float4& specular, const float3 direct, float indensity = 1.0);
	const float3 & Direction() const {return mDirect;}
private : 
	float3 mDirect;
};

}




#endif 