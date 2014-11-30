//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXLight.cpp#12 $
//   $Change: 94812 $
//   $Date: 2012/12/05 $
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

#include "precompiled.h"
#include "LcFBXLight.h"
#include "LiFBXMisc.h"
#include <lclinear\LcLRotation.h>
#include <lclinear\LcLTransform.h>
#include <stlport\vector>

bool 
LcFBXLight::AddNwcLight(FbxNode* node, FbxTime& ptime, LcFBXScene& fbx_scene, FbxAMatrix& parent_global_position)
{
   LiUAssertAny(node);
   if (!node)
   {
      return false;
   }

   FbxLight* llight = (FbxLight*) node->GetNodeAttribute();
   if (!llight)
   {
      return false;
   }

   FbxAMatrix llight_rotation, llight_global_position;

   // Must rotate the light's global position because 
   // FBX lights point towards the Y negative axis.
   FbxVector4 ly_negative_axis(-90.0, 0.0, 0.0);
   llight_rotation.SetR(ly_negative_axis);
   llight_global_position = parent_global_position * llight_rotation;

   // Get the light color.
   FbxDouble3 c = llight->Color.Get();
   FbxColor lcolor( c[0], c[1], c[2] );
   FbxAnimCurve* fc;
   fc = llight->Color.GetCurve(fbx_scene.GetCurrentAnimationLayer(), FBXSDK_CURVENODE_COLOR_RED);
   if (fc) 
   {
      lcolor.mRed = fc->Evaluate(ptime);
   }

   fc = llight->Color.GetCurve(fbx_scene.GetCurrentAnimationLayer(), FBXSDK_CURVENODE_COLOR_GREEN);
   if (fc) 
   {
      lcolor.mGreen = fc->Evaluate(ptime);
   }

   fc = llight->Color.GetCurve(fbx_scene.GetCurrentAnimationLayer(), FBXSDK_CURVENODE_COLOR_BLUE);
   if (fc) 
   {
      lcolor.mBlue = fc->Evaluate(ptime);
   }

   LtFloat lcone_angle = llight->OuterAngle.Get();
   LtFloat lhot_spot = llight->InnerAngle.Get();
   // The cone angle is only relevant if the light is a spot.
   if (llight->LightType.Get() == FbxLight::eSpot)
   {
      fc = llight->OuterAngle.GetCurve(fbx_scene.GetCurrentAnimationLayer());
      if (fc) 
      {
         lcone_angle = fc->Evaluate(ptime);
      }

      fc = llight->InnerAngle.GetCurve(fbx_scene.GetCurrentAnimationLayer());
      if (fc)
      {
         lhot_spot = fc->Evaluate(ptime);
      }
   }

   bool active_status = llight->CastLight.Get();
   LtFloat intensity = llight->Intensity.Get();
   if (intensity)
   {
      lcolor.Set( (lcolor.mRed*intensity)/100.0, (lcolor.mGreen*intensity)/100.0, (lcolor.mBlue*intensity)/100.0);
   }   

   FbxVector4 tran = llight_global_position.GetT();
   LcLVec3f dir(0,0,-1);
   FbxQuaternion quaternion = llight_global_position.GetQ();
   LcLRotation3f quaternion3f(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
   LcLTransform3f rot3f(quaternion3f);
   LcLVec3f rot_dir = dir * rot3f;

   if (llight->LightType.Get() == FbxLight::eSpot)
   {
      LcNwcSpotLight nw_spot_light;
      nw_spot_light.SetOn(active_status);

      nw_spot_light.SetPosition(tran[0],tran[1],tran[2]);
      nw_spot_light.SetDirection(rot_dir[0], rot_dir[1], rot_dir[2]);  
     
      nw_spot_light.SetDiffuseColor(lcolor.mRed,lcolor.mGreen,lcolor.mBlue);
      nw_spot_light.SetSpecularColor(lcolor.mRed,lcolor.mGreen,lcolor.mBlue);
      //other
      //note : todo ?nw_spot_light.SetAttenuation();??
      // NOTE: our spot light use cone specified as half angle,
      // but FBX light use a full cone angle.
      nw_spot_light.SetCutOffAngle((lcone_angle/180.0)*LI_PI/2); 
      nw_spot_light.SetDropOffRate(lcone_angle?(1.0 - lhot_spot/lcone_angle):0);
      nw_spot_light.SetEye(FALSE); 

      //add light to scene
      fbx_scene.GetNwcScene()->AddLight(nw_spot_light);
   }
   else if(llight->LightType.Get() == FbxLight::ePoint)
   {
      LcNwcPointLight nw_point_light;
      nw_point_light.SetOn(active_status);

      nw_point_light.SetPosition(tran[0],tran[1],tran[2]);       
         
      //colour
      //note : todo ??nw_point_light.SetAmbientColor(rgb.x,rgb.y,rgb.z);??
      nw_point_light.SetDiffuseColor(lcolor.mRed,lcolor.mGreen,lcolor.mBlue);  
      nw_point_light.SetSpecularColor(lcolor.mRed,lcolor.mGreen,lcolor.mBlue); 
      //other
      //note : todo ?nw_spot_light.SetAttenuation();?? 
      nw_point_light.SetEye(FALSE); 

      //add light to scene
      fbx_scene.GetNwcScene()->AddLight(nw_point_light);
   }
   else if (llight->LightType.Get() == FbxLight::eDirectional)
   {
      LcNwcDistantLight nw_distance_light;
      nw_distance_light.SetOn(active_status);

      nw_distance_light.SetDirection(rot_dir[0], rot_dir[1], rot_dir[2]);

      //colour
      //note : todo ??nw_point_light.SetAmbientColor(rgb.x,rgb.y,rgb.z);??
      nw_distance_light.SetDiffuseColor(lcolor.mRed,lcolor.mGreen,lcolor.mBlue);  
      nw_distance_light.SetSpecularColor(lcolor.mRed,lcolor.mGreen,lcolor.mBlue); 
      //other
      //note : todo ?nw_spot_light.SetAttenuation();?? 
      nw_distance_light.SetEye(FALSE);

      //add light to scene
      fbx_scene.GetNwcScene()->AddLight(nw_distance_light);
   }
   else
   {
      return false;
   }

   return true;
}
