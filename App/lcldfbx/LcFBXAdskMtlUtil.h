//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXAdskMtlUtil.h#2 $
//   $Change: 107559 $
//   $Date: 2013/12/11 $
//
//   Description:
//      Fbx Adsk Material Creator - 
//      this class is used to create adsk material from non-adsk material in FBX file.
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
#ifndef LCFBXADSKMTLUTIL_HDR
#define LCFBXADSKMTLUTIL_HDR
#pragma once

class LcFBXMaterialFactors;
class LcFBXScene;

class LcFBXAdskMtlUtil
{
public:
   // Create protein materials from non-protein mtl
   static LcNwcAutodeskMaterial CreateAdskMtl(FbxSurfaceMaterial* material,
      LcFBXScene& fbx_scene,
      LcFBXMaterialFactors& material_factors,
      bool& uv_swap,
      FbxTexture::EUnifiedMappingType& mapping_type);

   // Create protein materials from non-protein mtl
   // and attach it to the geometry node
   static bool  AttachAdskMtl(FbxNode* node,
      FbxGeometry* pgeometry,
      LcFBXScene& fbx_scene,
      LcNwcGeometry& geo,
      bool& uv_swap,
      FbxTexture::EUnifiedMappingType& mapping_type,
      bool& adjust_uv);

   // Get protein material from fbx node and material id
   // this method will return the protein mtl created in LcFBXSceneProcessor,
   // the original material in FBX should be also of protein type.
   static LcNwcAutodeskMaterial GetAdskMtlFromBlob(LcFBXScene& fbx_scene, 
      FbxNode* node,
      LtNat32 mtl_id);

private:
   LcFBXAdskMtlUtil();
   ~LcFBXAdskMtlUtil();

};

#endif //LCFBXADSKMTLUTIL_HDR
