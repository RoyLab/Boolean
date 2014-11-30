#include "stdafx.h"
#include "LightMgr.h"


CLightMgr::CLightMgr(void)
{
	mLights.reserve(3);
	GS::DirectionLight* pDefaultLight= new  GS::DirectionLight(GS::float4(0.8, 0.8, 0.8, 1.0),GS::float4(0.0, 0.0, 0.0, 1.0),
	GS::float3( -0.7, 0.6, -0.8));
	mLights.push_back(pDefaultLight);
	pDefaultLight = new  GS::DirectionLight(GS::float4(0.45, 0.45, 0.45, 1.0), GS::float4(1.0, 1.0, 1.0, 1.0),
			GS::float3(1.0, -0.4, 0.4));
	mLights.push_back(pDefaultLight);
	mAmbient.SetDiffuseColor(GS::float4(0.2, 0.2, 0.2, 1.0));
}


CLightMgr::~CLightMgr(void)
{
	for(int i = 0 ; i< mLights.size(); i++)
	{
		delete mLights[i];
	}
	mLights.clear();
}

void CLightMgr::AddLight(GS::Light* pLight)
{
	mLights.push_back(pLight);
}

const GS::DirectionLight& CLightMgr::DefaultLight(int idx) const
{
	return *static_cast<GS::DirectionLight*> (mLights[idx]);
}