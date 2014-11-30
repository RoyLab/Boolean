//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXLight.h#5 $
//   $Change: 85129 $
//   $Date: 2012/03/31 $
//
//   Description:
//      Fbx Light - this class is used to create nwc light
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
#ifndef LCFBXLIGHT_HDR
#define LCFBXLIGHT_HDR

#pragma once

#include "LcFBXScene.h"

///
/// FBX Light class is used to create nwc light
///
class LcFBXLight
{
public:
   LcFBXLight(LcFBXScene* scene) {m_fbx_scene=scene;};

   /// 
   /// Add nwc light
   /// 
   /// @param node FBX node
   /// @param ptime FBX animation time
   /// @param fbx_scene FBX scene
   /// @param parent_global_position parent global position
   /// 
   bool AddNwcLight(FbxNode* node, FbxTime& ptime, LcFBXScene& fbx_scene, FbxAMatrix& parent_global_position);

private:
   LcFBXScene* m_fbx_scene;

};

#endif // LCFBXLIGHT_HDR