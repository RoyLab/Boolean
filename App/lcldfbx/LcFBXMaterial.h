//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXMaterial.h#8 $
//   $Change: 107559 $
//   $Date: 2013/12/11 $
//
//   Description:
//      Fbx material - this class is used to create nwc material
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

#ifndef LCFBXMATERIAL_HDR
#define LCFBXMATERIAL_HDR

#pragma once

///
/// FBX material Factors class is used to store material factors when processing one FBX material
/// These factors may be used for processing corresponding texture
///
class LcFBXMaterialFactors
{
public:
   FbxDouble3 m_diffuse;
   bool m_got_diffuse;
   FbxDouble3 m_emissive;
   bool m_got_emissive;
   LtFloat m_emissive_factor;
   FbxDouble3 m_ambient;
   bool m_got_ambient;
   LtFloat m_transparency_factor;
   bool m_got_transparency_factor;
   FbxDouble3 m_reflection;
   bool m_got_reflection;
   LtFloat m_reflection_factor;
   FbxDouble3 m_specular;
   bool m_got_specular;
   LtFloat m_specular_factor;
   LtFloat m_shininess;
   bool m_got_shininess;

   /// 
   /// Constructor
   /// 
   LcFBXMaterialFactors()
      : m_got_ambient(false)
      , m_got_diffuse(false)
      , m_got_emissive(false)
      , m_got_reflection(false)
      , m_got_transparency_factor(false)
      , m_got_specular(false)
      , m_got_shininess(false)
      , m_transparency_factor(0)
      , m_shininess(0)
   {
   }
};


///
/// FBX material class is used to create nwc material
///
class LcFBXMaterial
{
public:
   /// 
   /// Get nwc material by specific FBX surface material
   /// 
   /// @param sur_mat FBX surface material
   /// @param material_factors material factors
   /// 
   static LcNwcMaterial GetNwcMaterial(FbxSurfaceMaterial* sur_mat,
      LcFBXMaterialFactors& material_factors);

   /// 
   /// Get nwc material by specific FBX node
   /// 
   /// @param node FBX node
   /// @param material_factors material factors
   /// 
   static LcNwcMaterial GetNwcMaterial(FbxNode* node , LcFBXMaterialFactors& material_factors);
};

#endif // LCFBXMATERIAL_HDR