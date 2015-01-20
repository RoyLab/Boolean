#include "stdafx.h"
#include "Graphics.h"
#include "SceneParser.h"
#include "typedefs.h"
#include "Global.h"
#include "BoolOp.h"
#include <iostream>


CGraphics::CGraphics()
	:mViewPortSize(0, 0)
	, mCursorPos(0, 0)
{
	m_pD3D =0;
	mExpression="0-(1+2)";
}

CGraphics::~CGraphics()
{
	if (m_pD3D)
	{
		delete m_pD3D;
		m_pD3D = 0 ; 
	}
}

void CGraphics::Shutdown()
{
	mShader.Release();
	if (m_pD3D)
	{
		m_pD3D->Shutdown();
		delete m_pD3D;
		m_pD3D =0;
	}
	GS::Global::Shutdown();
}

bool CGraphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	GS::Global::Initialize();
	mViewPortSize = GS::int2(screenWidth,screenHeight );
	// Create the Direct3D object.
	m_pD3D = new GS::VirtualDeviceDx11();
	if(!m_pD3D)
	{
		return false;
	}
    if (screenWidth == 0)
		screenWidth =1;
	if (screenHeight == 0)
		screenHeight =1;
	// Initialize the Direct3D object.
	result = m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, false, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}
	result = mShader.Initialize(m_pD3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Shader.", L"Error", MB_OK);
		return false;
	}
    return true;
}

void  CGraphics::ResizeWindow(int width, int height)
{

	if (width == 0 || height ==0 )
		return ; 
	mViewPortSize = GS::int2(width, height);
	if (m_pD3D)
		m_pD3D->Resize(width, height);

    mCamera.SetProjectionMode(true);
	mCamera.SetExtends(width, height);
}

bool CGraphics::Frame()
{
	bool result;
	
	if (!m_pD3D)
		return false; 

	// Render the graphics scene.
	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}

bool CGraphics::Render()
{
	if(!m_pD3D)
		return false;
	GS::float4x4 worldMatrix (GS::id4x4());
	// Clear the buffers to begin the scene.
    GS::float4x4 worldMat = GS::inverse(mCamera.ViewMatrix());

	const GS::float4& diffuse1 = mLightMgr.DefaultLight(0).DiffuseColor();
	const GS::float4& specular1 = mLightMgr.DefaultLight(0).SpecularColor();
	GS::float3 lightDir1 = normalize(mLightMgr.DefaultLight(0).Direction());

	const GS::float4& diffuse2 = mLightMgr.DefaultLight(1).DiffuseColor();
	GS::float3 lightDir2 = normalize(mLightMgr.DefaultLight(1).Direction());
	ComputeLightDirection(mLightMgr.DefaultLight(1), worldMat, lightDir2);
	float SpecularFactor = mLightMgr.DefaultLight(0).SpecularFactor();
	m_pD3D->BeginScene(1.0f, 1.0f, 1.0f, 1.0f);
	mShader.SetShaderMatrix(m_pD3D->GetDeviceContext(), worldMatrix, mCamera.ViewMatrix(), mCamera.ProjectionMatrix(), mCamera.Eye());
	mShader.SetShaderLights(m_pD3D->GetDeviceContext(),mLightMgr.AmbientColor(), diffuse1, specular1, lightDir1, diffuse2, lightDir2, SpecularFactor);
	mShader.Render( m_pD3D->GetDeviceContext());
	mModelMgr.Render(m_pD3D->GetDevice(), m_pD3D->GetDeviceContext());
	// Present the rendered scene to the screen.  
    m_pD3D->EndScene();  
  
    return true; 
}

inline
void CGraphics::ComputeLightDirection(const GS::DirectionLight& light, const GS::float4x4& worldMat, GS::float3& Lightdir)
{

	Lightdir = GS::mul(worldMat,GS::point(light.Direction())).xyz;
   // Lightdir = light.Direction();
	Lightdir = GS::normalize(Lightdir);
	
	
}

void CGraphics::ShowOctree(bool bShow)
{
    if (!bShow)
        mModelMgr.ClearResults();
    else 
        mModelMgr.CreateOctree();


}


void CGraphics::Union()
{
    mModelMgr.BoolOperation(eUnion, GS::eMeshBool);
}

void CGraphics::Difference()
{
    mModelMgr.BoolOperation(eDifference, GS::eMeshBool);
}

void CGraphics::Intersect()
{
    mModelMgr.BoolOperation(eIntersect, GS::eMeshBool);
}

 void CGraphics::EvaluateBoolExpression()
 {
     bool flag = false;
     //std::string s ="";
     //char c[20];
     //const int end = 5;
     //for (int i = 0 ; i < end; i++)
     //{
     //    
     //    sprintf_s(c, 20, "%d", i);
     //    s+=c;
     //    if (i <end-1)
     //        if (flag) s+="+";
     //        else s+="-";
     //   flag = !flag;
     //}

     //mModelMgr.EvaluateBoolExpression(s);
     mModelMgr.EvaluateBoolExpression(mExpression.c_str());
 }


void  CGraphics::BSPUnion()
{
    mModelMgr.BoolOperation(eUnion, GS::eBSPBool);
}

void  CGraphics::BSPIntersect()
{
    mModelMgr.BoolOperation(eIntersect, GS::eBSPBool);
}


void  CGraphics::BSPDifference()
{
    mModelMgr.BoolOperation(eDifference, GS::eBSPBool);
}


void CGraphics::LocalizedBSPUnion()
{
    mModelMgr.BoolOperation(eUnion, GS::eLBSPBool);
}
    
void CGraphics::LocalizedBSPIntersect()
{
    mModelMgr.BoolOperation(eIntersect, GS::eLBSPBool);
}
    
void CGraphics::LocalizedBSPDifference()
{
    mModelMgr.BoolOperation(eDifference, GS::eLBSPBool);
}


bool CGraphics::LoadSceneFromStream(char* fbxFileName)
{

	GS::float3 pos(0, 0, -10);
	GS::float3 target(0, 0, 1);
	GS::float3 up(0,1, 0);
	//GS::float3 pos(31.696, -85.373, 105.02);
	//GS::float3 target(20.960, 14.08, 3.013);
	//GS::float3 up(-0.07, 0.709, 0.700);
	mCamera.SetPosition(pos, target, up);
	SceneParser* parser = SceneParser::GetParser(fbxFileName);
	if (parser == NULL)
		return false ;
	if (!parser->Initialize())
		return false;
	parser->LoadScene(fbxFileName);
	std::vector<GS::BaseMesh*>  meshList; 
	std::vector<GS::Light*>     lights; 
	parser->ProcessScene(meshList, lights, mCamera);
	mModelMgr.Clear();
	mBoundingBox.Clear();
	for (int i = 0; i < meshList.size(); i++)
	{

		mModelMgr.Add(meshList[i]);
		mBoundingBox.IncludeBox(meshList[i]->AABB());
	}
    //mCamera.SetOrthoRange(mBoundingBox);
	mModelMgr.Invalidate();
	meshList.clear();
	lights.clear();
	delete parser;
	return true;
}

void CGraphics::ZoomView(short zDelta)
{
	 
		 float fPercent = 60/ 100.0;
        if (zDelta > 0) 
            fPercent = -fPercent / 2.0;
        fPercent += 1.0;  
		float fieldWidth = mCamera.HorizontalExtent();
		float fieldHeight = mCamera.VerticalExtent();
        fieldWidth  *= fPercent;
        fieldHeight *= fPercent;
		mCamera.SetExtends(fieldWidth, fieldHeight);
	
}

void CGraphics::StartOrbitView(int x, int y)
{
	mCursorPos.x  = x;
	mCursorPos.y = FlipYAxis(y);
	mCenterPos = mCamera.Target();

}

void CGraphics::UpdateOrbitView(int x, int y)
{
	int  dx = x - mCursorPos.x;
	int  dy = FlipYAxis(y)- mCursorPos.y;
	
	int sx = std::min(mViewPortSize.x,  mViewPortSize.y);
	if (sx < 1) {
		 sx = 1;
     }
    double pixelAngleScale = PI_2 / ((double)(sx) * 0.4) ;
    double ax = (double)(dx) * pixelAngleScale;
    double ay = -(double)(dy) * pixelAngleScale;
	mCamera.Orbit(mCenterPos, ax, ay);
	mCursorPos.x  = x;
    mCursorPos.y = FlipYAxis(y);
      
}

inline 
int CGraphics::FlipYAxis(int y)
{
	return mViewPortSize.y - y -1;
}
