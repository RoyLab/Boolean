#pragma once
#include <vector>
#include "Light.h"
class CLightMgr
{
public:
	CLightMgr(void);
	~CLightMgr(void);
	void AddLight(GS::Light* pLight); 
	void SetAmbientColor(const GS::float4& color) { mAmbient.SetDiffuseColor( color);}
	const GS::float4& AmbientColor() const {return mAmbient.DiffuseColor();}
	const GS::DirectionLight& DefaultLight(int idx) const;
	int  DefaultLightSize() const { return mLights.size(); }
private : 
	GS::DirectionLight mAmbient;
	std::vector<GS::Light*> mLights;
};

