//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXCamera.cpp#13 $
//   $Change: 102685 $
//   $Date: 2013/08/12 $
//
//   Description:
//      Fbx Camera - this class is used to create nwc view point
//
//
//   Copyright Notice:
//
//     Copyright 2009 Autodesk, Inc.  All rights reserved.
//  
//     This computer source code and related instructions and 
//     comments are the unpublished confidential and proprietary
//     information of Autodesk, Inc. and are protected under 
//     United States and foreign intellectual property laws.
//     They may not be disclosed to, copied or used by any third
//     party without the prior written consent of Autodesk, Inc.
//
#include "precompiled.h"
#include "LcFBXCamera.h"
#include "LiFBXMisc.h"
#include "LcFBXScene.h"

void
LcFBXCamera::SetViewpointsFromCamera(LcNwcScene* nwc_scene,
                                     FbxScene* pScene, 
                                     FbxTime* pTime, 
                                     FbxArray<FbxNode*>& pCameraArray)
{
   LiUAssertAny(nwc_scene && pScene && pTime);
   if (!nwc_scene || !pScene || !pTime)
      return;

   FbxCamera* current_camera = GetCurrentCamera(pScene, pTime, pCameraArray);
   LcNwcView current_view;

   if (GetViewAccordingToCamera(pScene, pTime, current_camera, current_view))
   {
      current_view.SetRenderStyle(eFULL_RENDER);
      nwc_scene->SetCurrentView(current_view);
   }

   // Find the camera in the camera array.
   for (LtInt32 i = 0; i < pCameraArray.GetCount(); i++)
   {
      if (mCurrentCameraName.Compare(pCameraArray[i]->GetName()) != 0)
      {
         FbxNode* lNode = pCameraArray[i];
         if (lNode)
         {
            // Get the animated parameters of the camera.
            GetCameraAnimatedParameters(lNode, pTime);

            FbxCamera* camera = (FbxCamera*) lNode->GetNodeAttribute();
            LcNwcView saved_view;
            if (GetViewAccordingToCamera(pScene, pTime, camera, saved_view))
            {
               nwc_scene->AddSavedView(saved_view);
            }

         }
      }
   }
}

LtBoolean
LcFBXCamera::GetViewAccordingToCamera(FbxScene* pScene, 
                                      FbxTime* pTime, 
                                      FbxCamera* pCamera,
                                      LcNwcView& nwc_view)
{
   LiUAssertAny(pScene && pTime && pCamera);
   if (!pScene || !pTime || !pCamera)
      return FALSE;

   // Find the current camera at the given time.
   FbxCamera* lCamera = pCamera;
   FbxNode*   lCameraNode = lCamera ? lCamera->GetNode() : NULL;

   // Compute the camera position and direction.
   FbxVector4 lEye(0,0,1);
   FbxVector4 lCenter(0,0,0);
   FbxVector4 lUp(0,1,0);
   FbxVector4 lForward, lRight;

   if (lCamera)
   {
      lEye = lCamera->Position.Get();
      lUp = lCamera->UpVector.Get();
   }

   if (lCameraNode && lCameraNode->GetTarget())
   {
      lCenter = LiFBXGetGlobalPosition(lCameraNode->GetTarget(), NULL, pTime).GetT();
   }
   else
   {
      if (!lCameraNode || IsProducerCamera(pScene, lCamera))
      {
         LiUAssertAny(lCamera);
         if (lCamera)
            lCenter = lCamera->InterestPosition.Get();
      }
      else
      {
         // Get the direction
         FbxAMatrix lGlobalRotation;
         FbxVector4 lRotationVector(LiFBXGetGlobalPosition(lCameraNode, NULL, pTime).GetR());
         lGlobalRotation.SetR(lRotationVector);

         // Get the length
         FbxVector4 lInterestPosition(lCamera->InterestPosition.Get());
         FbxVector4 lCameraGlobalPosition(LiFBXGetGlobalPosition(lCameraNode, NULL, pTime).GetT());
         LtFloat      lLength = (FbxVector4(lInterestPosition - lCameraGlobalPosition).Length());

         // Set the center.
         // A camera with rotation = {0,0,0} points to the X direction. So create a
         // vector in the X direction, rotate that vector by the global rotation amount
         // and then position the center by scaling and translating the resulting vector
         lRotationVector = FbxVector4(1.0,0,0);
         lCenter = lGlobalRotation.MultT(lRotationVector);
         lCenter *= lLength;
         lCenter += lEye;

         // Update the default up vector with the camera rotation.
         lRotationVector = FbxVector4(0,1.0,0);
         lUp = lGlobalRotation.MultT(lRotationVector);
      }
   }

   // Align the up vector.
   lForward = lCenter - lEye;
   lForward.Normalize();
   lRight = lForward.CrossProduct(lUp);
   lRight.Normalize();
   lUp = lRight.CrossProduct(lForward);
   lUp.Normalize();

   // Rotate the up vector with the roll value.
   LtFloat lRadians = 0;

   if (lCamera)
      lRadians = 3.1416 * lCamera->Roll.Get() / 180.0;
   lUp *= cos(lRadians);
   lRight *= sin(lRadians);
   lUp = lUp + lRight;

   LtFloat lNearPlane = 0.01;
   if (lCamera)
      lNearPlane = lCamera->GetNearPlane();
   LtFloat lFarPlane = 1000.0;
   if (lCamera)
      lFarPlane = lCamera->GetFarPlane();

   // Config view with the current camera.
   if (lCameraNode)
   {
      const LtChar* name = lCameraNode->GetName();
      LcUWideStringBuffer wname;
      if (name)
      {
          LcUString::UTF8ToWideString(name, &wname);
      }
      if (!wname.IsEmpty())
      {
         nwc_view.SetName(wname.GetString()); 
      }
       

      LtNwcProjection project_mode = 
         lCamera->ProjectionType.Get() == FbxCamera::ePerspective ? LI_NWC_PERSPECTIVE: LI_NWC_ORTHOGRAPHIC;
      nwc_view.SetProjection(project_mode);
   }

   nwc_view.SetPosition(lEye[0], 
      lEye[1], 
      lEye[2]);
   nwc_view.PointAt(lCenter[0], 
      lCenter[1], 
      lCenter[2]);
   nwc_view.AlignUp(lUp[0], 
      lUp[1], 
      lUp[2]);

   return TRUE;
}

FbxCamera* 
LcFBXCamera::GetCurrentCamera(FbxScene* pScene, 
                             FbxTime* pTime, 
                             FbxArray<FbxNode*>& pCameraArray)
{
   FbxGlobalSettings& lGlobalSettings = pScene->GetGlobalSettings();
   FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
   mCurrentCameraName = lGlobalSettings.GetDefaultCamera();

   // check if we need to create the Producer cameras!
   if (lGlobalCameraSettings.GetCameraProducerPerspective() == NULL &&
      lGlobalCameraSettings.GetCameraProducerBottom() == NULL &&
      lGlobalCameraSettings.GetCameraProducerTop() == NULL &&
      lGlobalCameraSettings.GetCameraProducerFront() == NULL &&
      lGlobalCameraSettings.GetCameraProducerBack() == NULL &&
      lGlobalCameraSettings.GetCameraProducerRight() == NULL &&
      lGlobalCameraSettings.GetCameraProducerLeft() == NULL)
   {
      lGlobalCameraSettings.CreateProducerCameras();
   }

   if (mCurrentCameraName.Compare(FBXSDK_CAMERA_PERSPECTIVE) == 0)
   {
      return lGlobalCameraSettings.GetCameraProducerPerspective();
   }
   else if (mCurrentCameraName.Compare(FBXSDK_CAMERA_BOTTOM) == 0)
   {
      return lGlobalCameraSettings.GetCameraProducerBottom();
   }
   else if (mCurrentCameraName.Compare(FBXSDK_CAMERA_TOP) == 0)
   {
      return lGlobalCameraSettings.GetCameraProducerTop();
   }
   else if (mCurrentCameraName.Compare(FBXSDK_CAMERA_FRONT) == 0)
   {
      return lGlobalCameraSettings.GetCameraProducerFront();
   }
   else if (mCurrentCameraName.Compare(FBXSDK_CAMERA_BACK) == 0)
   {
      return lGlobalCameraSettings.GetCameraProducerBack();
   }
   else if (mCurrentCameraName.Compare(FBXSDK_CAMERA_RIGHT) == 0)
   {
      return lGlobalCameraSettings.GetCameraProducerRight();
   }
   else if (mCurrentCameraName.Compare(FBXSDK_CAMERA_LEFT) == 0)
   {
      return lGlobalCameraSettings.GetCameraProducerLeft();
   }
   else if (mCurrentCameraName.Compare(FBXSDK_CAMERA_SWITCHER) == 0)
   {
      FbxCameraSwitcher* lCameraSwitcher = pScene->GlobalCameraSettings().GetCameraSwitcher();
      FbxAnimCurve* lCurve = NULL;
      if (lCameraSwitcher)
      {
         lCurve = lCameraSwitcher->CameraIndex.GetCurve(m_fbx_scene->GetCurrentAnimationLayer());

         LtInt32 lCameraIndex = 0;
         if (lCurve)
            // Get the index of the camera in the camera array.
            lCameraIndex = (LtInt32) lCurve->Evaluate(*pTime) - 1;

         if (lCameraIndex >= 0 && lCameraIndex < pCameraArray.GetCount())
         {
            FbxNode* lNode = pCameraArray[lCameraIndex];

            // Get the animated parameters of the camera.
            GetCameraAnimatedParameters(lNode, pTime);

            return (FbxCamera*) lNode->GetNodeAttribute();
         }
      }
   }
   else
   {
      LtInt32 i;
      FbxNode* lNode = NULL;

      // Find the camera in the camera array.
      for (i = 0; i < pCameraArray.GetCount(); i++)
      {
         if (mCurrentCameraName.Compare(pCameraArray[i]->GetName()) == 0)
         {
            lNode = pCameraArray[i];
            break;
         }
      }

      if (lNode)
      {
         // Get the animated parameters of the camera.
         GetCameraAnimatedParameters(lNode, pTime);

         return (FbxCamera*) lNode->GetNodeAttribute();
      }
   }

   return lGlobalCameraSettings.GetCameraProducerPerspective();
}

void 
LcFBXCamera::GetCameraAnimatedParameters(FbxNode* pNode, 
                                 FbxTime* pTime)
{
   FbxCamera* lCamera = (FbxCamera*) pNode->GetNodeAttribute();
   lCamera->Position.Set(LiFBXGetGlobalPosition(pNode, NULL, pTime).GetT());

   FbxAnimCurve* fc;
   
   fc = lCamera->Roll.GetCurve(m_fbx_scene->GetCurrentAnimationLayer());
   if (fc)
      lCamera->Roll.Set(fc->Evaluate(*pTime));

   if (lCamera->GetApertureMode() == FbxCamera::eHorizontal || 
      lCamera->GetApertureMode() == FbxCamera::eVertical) 
   {
      fc = lCamera->FieldOfView.GetCurve(m_fbx_scene->GetCurrentAnimationLayer());
      if (fc)
         lCamera->FieldOfView.Set(fc->Evaluate(*pTime));
   }
   else if (lCamera->GetApertureMode() == FbxCamera::eHorizAndVert)
   {
      // In this aperture mode, the squeeze ratio is not relevant 
      // because the fields of view in X and Y are independent.
      // It's value is set to retrieve the proper aspect in function SetCamera().
      //
      // if:
      // 
      // aspect = field of view X / field of view Y = aperture width * squeeze ratio / aperture height
      //
      // then:
      // 
      // squeeze ratio = (field of view X * aperture height) / (field of view Y * aperture width)
      //
      LtFloat lFieldOfViewX = lCamera->FieldOfViewX.Get();
      LtFloat lFieldOfViewY = lCamera->FieldOfViewY.Get();
      fc = lCamera->FieldOfViewX.GetCurve(m_fbx_scene->GetCurrentAnimationLayer());
      if (fc)
         lFieldOfViewX = fc->Evaluate(*pTime);

      fc = lCamera->FieldOfViewY.GetCurve(m_fbx_scene->GetCurrentAnimationLayer());
      if (fc)
         lFieldOfViewY = fc->Evaluate(*pTime);

      LtFloat lSqueezeRatio = (lFieldOfViewX * lCamera->GetApertureHeight()) / (lFieldOfViewY * lCamera->GetApertureWidth());

      lCamera->FieldOfViewX.Set(lFieldOfViewX);
      lCamera->FieldOfViewY.Set(lFieldOfViewY);
      lCamera->SetSqueezeRatio(lSqueezeRatio);

      // Optical center offset is not yet supported in this example.
      //LtFloat lOpticalCenterX = pNode->GetCurrentTakeNode()->GetCameraOpticalCenterX()->Evaluate(*pTime);
      //LtFloat lOpticalCenterY = pNode->GetCurrentTakeNode()->GetCameraOpticalCenterY()->Evaluate(*pTime);
   }
   else if (lCamera->GetApertureMode() == FbxCamera::eFocalLength)
   {
      LtFloat lFocalLength = lCamera->FocalLength.Get();
      fc = lCamera->FocalLength.GetCurve(m_fbx_scene->GetCurrentAnimationLayer());
      if (fc && fc ->Evaluate(*pTime))
         lCamera->FocalLength.Set(lFocalLength);
   }
}

bool 
LcFBXCamera::IsProducerCamera(FbxScene*  pScene, FbxCamera* pCamera)
{
   FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
   if (pCamera == lGlobalCameraSettings.GetCameraProducerPerspective())
      return true;
   if (pCamera == lGlobalCameraSettings.GetCameraProducerTop())
      return true;
   if (pCamera == lGlobalCameraSettings.GetCameraProducerBottom())
      return true;
   if (pCamera == lGlobalCameraSettings.GetCameraProducerFront())
      return true;
   if (pCamera == lGlobalCameraSettings.GetCameraProducerBack())
      return true;
   if (pCamera == lGlobalCameraSettings.GetCameraProducerRight())
      return true;
   if (pCamera == lGlobalCameraSettings.GetCameraProducerLeft())
      return true;

   return false;
}