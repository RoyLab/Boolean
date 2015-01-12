#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_
#include "VirtualDeviceDx11.h"
#include "Camera.h"
#include "Geomary3DShader.h"
#include "ModelManager.h"
#include "LightMgr.h"

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


class CGraphics{
public : 
    CGraphics();
    ~CGraphics();
    bool Initialize(int, int, HWND);
    void ResizeWindow(int width, int height);
    bool LoadSceneFromStream(char* fbxFileName);
    const CModelManager&  GetModelMgr() const {return mModelMgr;}
    void Union();
    void Difference();
    void Intersect();
    void EvaluateBoolExpression();

    // BSP Boolean operation 
    void BSPUnion();
    void BSPIntersect();
    void BSPDifference();

    // Localized BSP Boolean operation 
    void LocalizedBSPUnion();
    void LocalizedBSPIntersect();
    void LocalizedBSPDifference();
    //                                       
    void ShowOctree(bool bShow);


    void Shutdown();
    bool Frame();
    void ZoomView(short zDelta);
    void StartOrbitView(int x, int y);
    void UpdateOrbitView(int x, int y); 

	void setExpression(char* str){mExpression = str;}

private : 
    void ComputeLightDirection(const GS::DirectionLight& light, const GS::float4x4& worldMat, GS::float3& Lightdir);
    bool Render();  
    int  FlipYAxis(int y);
private : 
    GS::VirtualDeviceDx11* m_pD3D;
    CGeomary3DShader	   mShader;
    GS::Camera             mCamera;
    CModelManager          mModelMgr; //manage geomary data 
    CLightMgr              mLightMgr;
    GS::Box3               mBoundingBox;
    GS::int2               mViewPortSize;
    GS::int2               mCursorPos;
    GS::float3             mCenterPos;
	
	std::string				mExpression;
};

#endif 