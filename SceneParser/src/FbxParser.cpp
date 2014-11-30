#include "FbxParser.h"
#include "BaseMesh.h"
#include "typedefs.h"
#include "quaternion.h"

FbxParser::FbxParser()
	:mpFbxManager(NULL)
	,mpFbxScene(NULL)
{

}

FbxParser::~FbxParser()
{	
	Release();
}

void FbxParser::Release()
{
//if (mpFbxScene)
	//	mpFbxScene->Destroy();
	if (mpFbxManager)
	{
		mpFbxManager->Destroy();
		mpFbxManager = 0; 
	}
}

bool FbxParser::Initialize()
{
	if(mpFbxManager)
		return true; 
	 mpFbxManager =  FbxManager::Create();
    if( !mpFbxManager )
    {
        return false ;
    } 
	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(mpFbxManager, IOSROOT);
	mpFbxManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	mpFbxManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    mpFbxScene = FbxScene::Create(mpFbxManager, "My Scene");
	if( !mpFbxScene )
    {
		mpFbxManager->Destroy();
		mpFbxManager = NULL;
        return false ;
    }
	return true;
}


bool FbxParser::LoadScene(const char* pFilename)
{
	if(!mpFbxManager)
	if (mpFbxScene)
		mpFbxScene->Clear();
	int lSDKMajor,  lSDKMinor,  lSDKRevision;
    int lFileMajor, lFileMinor, lFileRevision;
	 // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(mpFbxManager,"");
	    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, mpFbxManager->GetIOSettings());
	  lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
	if (!lImportStatus)
	{
		lImporter->Destroy();
		return false ;
	}
	if (lImporter->IsFBX())
	{
		// Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
		mpFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL,        true);
        mpFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE,         true);
        mpFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK,            true);
        mpFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE,           true);
        mpFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO,            true);
        mpFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION,       true);
        mpFbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}
	// Import the scene.
	bool lStatus = lImporter->Import(mpFbxScene);
	lImporter->Destroy();
	return lStatus;
}

void FbxParser::ProcessScene(std::vector<GS::BaseMesh*>& meshs, std::vector<GS::Light*>& lights,GS::Camera& camera)
{
	int i;
    FbxNode* lNode = mpFbxScene->GetRootNode();
    if(lNode)
    {
        for(i = 0; i < lNode->GetChildCount(); i++)
        {
            ProcessNode(lNode->GetChild(i),meshs,lights, camera);
        }
    }
}


void FbxParser::ProcessNode(FbxNode* pNode,std::vector<GS::BaseMesh*>& meshs, 
							std::vector<GS::Light*>& lights, GS::Camera& camera)
{
    FbxNodeAttribute::EType lAttributeType;
    int i;

    if(pNode->GetNodeAttribute() == NULL)
    {
         return ;
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
	    default:
	        break;
        //case FbxNodeAttribute::eMarker:  
        //    DisplayMarker(pNode);
        //    break;

        //case FbxNodeAttribute::eSkeleton:  
        //    DisplaySkeleton(pNode);
        //    break;

        case FbxNodeAttribute::eMesh:      
            ProcessMesh(pNode, meshs);
            break;

        //case FbxNodeAttribute::eNurbs:      
        //    DisplayNurb(pNode);
        //    break;

        //case FbxNodeAttribute::ePatch:     
        //    DisplayPatch(pNode);
        //    break;

        case FbxNodeAttribute::eCamera:    
            ProcessCamera(pNode, camera);
            break;

        case FbxNodeAttribute::eLight:     
            ProcessLight(pNode, lights);
            break;

        //case FbxNodeAttribute::eLODGroup:
        //    DisplayLodGroup(pNode);
        //    break;
        }   
    }

   /* DisplayUserProperties(pNode);
    DisplayTarget(pNode);
    DisplayPivotsAndLimits(pNode);
    DisplayTransformPropagation(pNode);
    DisplayGeometricTransform(pNode);*/

    for(i = 0; i < pNode->GetChildCount(); i++)
    {
        ProcessNode(pNode->GetChild(i), meshs, lights, camera);
    }
}

void FbxParser::ProcessMesh(FbxNode* pNode,std::vector<GS::BaseMesh*>& meshs)
{
	FbxMesh* lMesh = (FbxMesh*) pNode->GetNodeAttribute ();
	if (lMesh == NULL)
		return ; 
	int triangleCount = lMesh->GetPolygonCount();  
    int vertexCounter = 0;  
	if (triangleCount ==0)
		return ; 
	
	GS::BaseMesh* pMesh = new GS::BaseMesh();
	GS::double3 p0, p1, p2;
	int vertexId = 0;
	GS::VertexInfo v1, v2, v3;
	
	for(int i = 0 ; i < triangleCount ; ++i)  
    {
        int ctrlPointIndex = lMesh->GetPolygonVertex(i , 0);
		
		ReadVertex(lMesh, ctrlPointIndex, v1.pos);
		ReadColor(lMesh, ctrlPointIndex, vertexId, v1.color);
		ReadNormal(lMesh, ctrlPointIndex, vertexId++, v1.normal);
		
		// read the second vertex
		ctrlPointIndex = lMesh->GetPolygonVertex(i , 1);
	    ReadVertex(lMesh, ctrlPointIndex, v2.pos);
		ReadColor(lMesh, ctrlPointIndex, vertexId, v2.color);
		ReadNormal(lMesh, ctrlPointIndex, vertexId++, v2.normal);
		// read the third vertex
		ctrlPointIndex = lMesh->GetPolygonVertex(i , 2);
		ReadVertex(lMesh, ctrlPointIndex, v3.pos);
		ReadColor(lMesh, ctrlPointIndex, vertexId, v3.color);
		ReadNormal(lMesh, ctrlPointIndex, vertexId++, v3.normal);
		pMesh->Add(v1, v2, v3);
	}
	pMesh->GenID();
	//pMesh->GenSurface();
	pMesh->GenAABB(true);
	meshs.push_back(pMesh);
}

inline
void FbxParser::ReadVertex(FbxMesh* pMesh , int ctrlPointIndex , GS::double3& p)  
{  
   FbxVector4* pCtrlPoint = pMesh->GetControlPoints();  
   p.x = pCtrlPoint[ctrlPointIndex][0];  
   p.y = pCtrlPoint[ctrlPointIndex][1];  
   p.z = pCtrlPoint[ctrlPointIndex][2];  
}  

void FbxParser::ReadColor(FbxMesh* pMesh, int ctrlPointIndex,int vertexCounter , GS::float4& color)
{
	if (pMesh->GetElementVertexColorCount() < 1)
		return ; 
	FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor( 0);
	FbxColor fbxColor;		
	switch (leVtxc->GetMappingMode())
	{
	default:
		break;
	case FbxGeometryElement::eByControlPoint:
		switch (leVtxc->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			 fbxColor = leVtxc->GetDirectArray().GetAt(ctrlPointIndex);
			  color= GS::float4(fbxColor.mRed, fbxColor.mGreen, fbxColor.mBlue, fbxColor.mAlpha);
			break;
		case FbxGeometryElement::eIndexToDirect:
			{
				int id = leVtxc->GetIndexArray().GetAt(ctrlPointIndex);
				fbxColor = leVtxc->GetDirectArray().GetAt(id);
			     color= GS::float4(fbxColor.mRed, fbxColor.mGreen, fbxColor.mBlue, fbxColor.mAlpha);
			}
			break;
		default:
			break; // other reference modes not shown here!
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		{
			switch (leVtxc->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				 fbxColor = leVtxc->GetDirectArray().GetAt(vertexCounter);
				 color= GS::float4(fbxColor.mRed, fbxColor.mGreen, fbxColor.mBlue, fbxColor.mAlpha);
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = leVtxc->GetIndexArray().GetAt(vertexCounter);
					fbxColor = leVtxc->GetDirectArray().GetAt(id);
					color= GS::float4(fbxColor.mRed, fbxColor.mGreen, fbxColor.mBlue, fbxColor.mAlpha);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
		}
		break;

	case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
	case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
	case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
		break;
	}

}



void FbxParser::ReadNormal(FbxMesh* pMesh, int ctrlPointIndex , int vertexCounter , GS::double3& normal)  
{  
    if(pMesh->GetElementNormalCount() < 1)  
    {  
        return;  
    }  

    FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal( 0);
    switch(leNormal->GetMappingMode())  
    {  
    case FbxGeometryElement::eByControlPoint:  
        {  
            switch(leNormal->GetReferenceMode())  
            {  
            case  FbxGeometryElement::eDirect:  
                {  
                    normal.x = leNormal->GetDirectArray().GetAt(ctrlPointIndex)[0];  
                    normal.y = leNormal->GetDirectArray().GetAt(ctrlPointIndex)[1];  
                    normal.z = leNormal->GetDirectArray().GetAt(ctrlPointIndex)[2];  
                }  
                break;  
  
            case FbxGeometryElement::eIndexToDirect:  
                {  
                    int id = leNormal->GetIndexArray().GetAt(ctrlPointIndex);  
                    normal.x = leNormal->GetDirectArray().GetAt(id)[0];  
                    normal.y = leNormal->GetDirectArray().GetAt(id)[1];  
                    normal.z = leNormal->GetDirectArray().GetAt(id)[2];  
                }  
                break;  
  
            default:  
                break;  
            }  
        }  
        break;  
  
    case FbxGeometryElement::eByPolygonVertex:  
        {  
            switch(leNormal->GetReferenceMode())  
            {  
            case FbxGeometryElement::eDirect:   
                {  
                    normal.x = leNormal->GetDirectArray().GetAt(vertexCounter)[0];  
                    normal.y = leNormal->GetDirectArray().GetAt(vertexCounter)[1];  
                    normal.z = leNormal->GetDirectArray().GetAt(vertexCounter)[2];  
                }  
                break;  
  
            case FbxGeometryElement::eIndexToDirect:  
                {  
                    int id = leNormal->GetIndexArray().GetAt(vertexCounter);  
                    normal.x = leNormal->GetDirectArray().GetAt(id)[0];  
                    normal.y = leNormal->GetDirectArray().GetAt(id)[1];  
                    normal.z = leNormal->GetDirectArray().GetAt(id)[2];  
                }  
                break;  
  
            default:  
                break;  
            }  
        }  
        break;  
    }  
}  

void FbxParser::ProcessCamera(FbxNode* pNode, GS::Camera& camera)
{
	FbxCamera* lCamera = (FbxCamera*) pNode->GetNodeAttribute();
	if (lCamera == NULL)
		return ;
	FbxNode* pTargetNode = pNode->GetTarget();
	FbxNode* pTargetUpNode = pNode->GetTargetUp();
	FbxVector4  lEye = lCamera->Position.Get();
    FbxVector4  lUp; 
	FbxVector4 lCenter;
	if (!pTargetNode)
	   lCenter = lCamera->InterestPosition.Get();
	if (!pTargetUpNode)
		lUp = lCamera->UpVector.Get();
	  
   double lRadians = PI * lCamera->Roll.Get() / 180.0;
    // Align the up vector.
   FbxVector4 lForward = lCenter - lEye;
   lForward.Normalize();
   FbxVector4 lRight = lForward.CrossProduct(lUp);
   lRight.Normalize();
   lUp = lRight.CrossProduct(lForward);
   lUp.Normalize();
   lUp *= cos(lRadians);
   lRight *= sin(lRadians);
   lUp = lUp + lRight;
	GS::float3 pos(lEye[0], lEye[1], lEye[2]);
	GS::float3 up(lUp[0], lUp[1], lUp[2]);
	GS::float3 target(lCenter[0], lCenter[1], lCenter[2]);
	GS::Camera cam(pos, target, up);
	float lNearPlane = 0.01;
    if (lCamera)
		lNearPlane = lCamera->GetNearPlane();
    float lFarPlane = 1000.0;
    if (lCamera)
		lFarPlane = lCamera->GetFarPlane();
    bool bPerspectice =  lCamera->ProjectionType.Get() == FbxCamera::ePerspective ;
	cam.SetProjectionMode(bPerspectice);
	cam.SetProjectionPlanes(lNearPlane, lFarPlane);
	camera = cam;

}
  

void FbxParser::ProcessLight(FbxNode* pNode, std::vector<GS::Light*>& lights)
{
   FbxLight* llight = (FbxLight*) pNode->GetNodeAttribute();
   if (!llight)
      return  ;

    // Get the light color.
   FbxDouble3 c = llight->Color.Get();
   GS::float4 lcolor( c[0], c[1], c[2], 1.0 );
   float intensity = llight->Intensity.Get();
   if (intensity)
     lcolor= lcolor*(intensity/100);
  
    // to do so far, we only process directional light
    if (llight->LightType.Get() == FbxLight::eDirectional)
	{
		//FbxDouble3 dir = pNode->LclRotation.Get(); 
		FbxAnimEvaluator* lEvaluator = mpFbxScene->GetAnimationEvaluator();

        FbxAMatrix lGlobal;
        lGlobal= lEvaluator->GetNodeGlobalTransform( pNode);

		FbxVector4 rotation = lGlobal.GetR();

		 FbxVector4 tran = lGlobal.GetT();
		 FbxQuaternion quaternion = lGlobal.GetQ();
		 GS::float4 q(quaternion[0], quaternion[1], quaternion[2],quaternion[3]);
		 GS::float4x4 rotMat = GS::quat_rotation_matrix(q);
		 GS::float4 dir(mul(rotMat, GS::float4(0, 0, -1, 1)));
    /* dir(0,0,-1);
   FbxQuaternion quaternion = lGlobal.GetQ();
   quaternion.
   LcLRotation3f quaternion3f(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
   LcLTransform3f rot3f(quaternion3f);
   LcLVec3f rot_dir = dir * rot3f;*/
	}
}

