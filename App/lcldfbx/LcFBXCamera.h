//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXCamera.h#6 $
//   $Change: 85129 $
//   $Date: 2012/03/31 $
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
#ifndef LCFBXCAMERA_HDR
#define LCFBXCAMERA_HDR
#pragma once

class LcFBXScene;
///
/// FBX camera class is used to create nwc view point
///
class LcFBXCamera
{
public:
   LcFBXCamera(LcFBXScene* scene) {m_fbx_scene=scene;};
   /// 
   /// Set nwc viewpoints according to camera
   /// 
   /// @param nwc_scene the nwcreate scene
   /// @param pScene FBX scene
   /// @param pTime FBX animation time
   /// @param pCameraArray FBX camera array
   /// 
   void SetViewpointsFromCamera(LcNwcScene* nwc_scene, 
      FbxScene* pScene, 
      FbxTime* pTime, 
      FbxArray<FbxNode*>& pCameraArray); 

private:

   /// 
   /// get nwc view point according to camera
   /// 
   /// @param pScene FBX scene
   /// @param pTime FBX animation time
   /// @param pCamera FBX camera
   /// @param nwc_view nwc view
   /// 
   LtBoolean GetViewAccordingToCamera(FbxScene* pScene, 
      FbxTime* pTime, 
      FbxCamera* pCamera,
      LcNwcView& nwc_view);

   /// 
   /// Get current camera
   /// 
   /// @param pScene FBX Scene
   /// @param pTime FBX animation time
   /// @param pCameraArray FBX camera array
   /// 
   FbxCamera* GetCurrentCamera(FbxScene* pScene, 
      FbxTime* pTime, 
      FbxArray<FbxNode*>& pCameraArray);

   /// 
   /// Get camera animated parameters
   /// 
   /// @param pNode FBX node
   /// @param pTime FBX animation time
   /// 
   void GetCameraAnimatedParameters(FbxNode* pNode, 
      FbxTime* pTime);

   /// 
   /// Judge if this camera is producer camera
   /// 
   /// @param pScene FBX scene
   /// @param pCamera FBX camera
   /// 
   bool IsProducerCamera(FbxScene*  pScene, FbxCamera* pCamera);

   FbxString mCurrentCameraName;
   LcFBXScene* m_fbx_scene;

};

#endif //LCFBXCAMERA_HDR