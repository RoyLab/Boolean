//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXMaterial.cpp#14 $
//   $Change: 107559 $
//   $Date: 2013/12/11 $
//
//   Description:
//      Fbx material - this class is used to create nwc material
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
#include "LcFBXMaterial.h"
#include "LiFBXMisc.h"


LcNwcMaterial
LcFBXMaterial::GetNwcMaterial(FbxNode* node , LcFBXMaterialFactors& material_factors)
{
   LiUAssertAny(node);
   if (NULL == node)
      return LcNwcMaterial(NULL);

   LtInt32 material_count = node->GetMaterialCount();
   FbxGeometry* geometry = node->GetGeometry();

   for (LtInt32 l = 0; l < geometry->GetLayerCount(); l++)
   {
      FbxLayerElementMaterial* ele_mat = geometry->GetLayer(l)->GetMaterials();
      if (ele_mat)
      {
         LcNwcMaterial nwc_mat(NULL);

         if (ele_mat->GetReferenceMode() == FbxLayerElement::eIndex)
            // Materials are in an undefined external table
            continue;

         if (material_count > 0)
         {
            FbxPropertyT<FbxDouble3> lFbxDouble3;
            FbxPropertyT<FbxDouble> lFbxDouble1;
            FbxColor theColor;

            for (LtInt32 count = 0; count < material_count; count ++)
            {
               FbxSurfaceMaterial *sur_mat = node->GetMaterial(count);
               LcNwcMaterial temp_nwc_mat = GetNwcMaterial(sur_mat,material_factors);
               if (temp_nwc_mat.GetHandle())
               {
                  nwc_mat = temp_nwc_mat;
               }
            }
         } // material_count > 0

         return nwc_mat;
      }  // ele_mat != NULL
   }

   return LcNwcMaterial(NULL);
}



LcNwcMaterial
LcFBXMaterial::GetNwcMaterial( FbxSurfaceMaterial* sur_mat, LcFBXMaterialFactors& material_factors)
{
   if (NULL==sur_mat)
   {
      return LcNwcMaterial(NULL);
   }

   FbxPropertyT<FbxDouble3> lFbxDouble3;
   FbxPropertyT<FbxDouble> lFbxDouble1;

   LcNwcMaterial nwc_mat;

   LtChar const* sur_mat_cname = sur_mat->GetName();
   LcUWideStringBuffer sur_mat_wname;
   if (sur_mat_cname)
   {
      LcUString::UTF8ToWideString(sur_mat_cname, &sur_mat_wname);
   }

   if (!sur_mat_wname.IsEmpty())
   {
      nwc_mat.SetName(sur_mat_wname.GetString());
   }

   //Get the implementation to see if it's a hardware shader.
   const FbxImplementation* limplementation = GetImplementation(sur_mat, FBXSDK_IMPLEMENTATION_HLSL);
   
   if(!limplementation)
   {
      limplementation = GetImplementation(sur_mat, FBXSDK_IMPLEMENTATION_CGFX);
   }

   if (limplementation)
   {
      //Now we have a hardware shader, but Navisworks doesn't support them.
      return LcNwcMaterial(NULL);
   }
   else if (sur_mat->GetClassId().Is(FbxSurfacePhong::ClassId))
   {
      // We found a Phong material.  Display its properties.
      
      FbxSurfacePhong* phong_mat = FbxCast<FbxSurfacePhong>(sur_mat);
      if (!phong_mat)
      {
         return LcNwcMaterial(NULL);

      }

      // Display the Ambient Color
      lFbxDouble3 =phong_mat->Ambient;
      nwc_mat.SetAmbientColor(lFbxDouble3.Get()[0], 
         lFbxDouble3.Get()[1],
         lFbxDouble3.Get()[2]);

      FbxDouble3 fb3 = lFbxDouble3.Get();

      material_factors.m_ambient = lFbxDouble3.Get();
      material_factors.m_got_ambient = true;


      // Display the Diffuse Color
      lFbxDouble3 =phong_mat->Diffuse;
      nwc_mat.SetDiffuseColor(lFbxDouble3.Get()[0], 
         lFbxDouble3.Get()[1],
         lFbxDouble3.Get()[2]);

      fb3 = lFbxDouble3.Get();

      material_factors.m_diffuse = lFbxDouble3.Get();
      material_factors.m_got_diffuse = true;

      // Display the Specular Color (unique to Phong materials)
      lFbxDouble3 =phong_mat->Specular;
      nwc_mat.SetSpecularColor(lFbxDouble3.Get()[0], 
         lFbxDouble3.Get()[1],
         lFbxDouble3.Get()[2]);

      fb3 = lFbxDouble3.Get();

      material_factors.m_specular = lFbxDouble3.Get();
      material_factors.m_specular_factor = (phong_mat->SpecularFactor).Get();
      material_factors.m_got_specular = true;


      // Display the Emissive Color
      lFbxDouble3 =phong_mat->Emissive;
      nwc_mat.SetEmissiveColor(lFbxDouble3.Get()[0], 
         lFbxDouble3.Get()[1],
         lFbxDouble3.Get()[2]);

      fb3 = lFbxDouble3.Get();

      material_factors.m_emissive = lFbxDouble3.Get();
      material_factors.m_got_emissive = true;
      material_factors.m_emissive_factor = (phong_mat->EmissiveFactor).Get();

      //Opacity is Transparency _Factor now

      //Set material transparent factor 1 when the value is 1 geting from fbxsdk. 
      //The transparent factor is always 0 in the fbx files from motionbuilder, 
      //but it is always 1 in the fbx files exported from maya. if not do so, nothing display for fbx files from maya
      LtFloat transparency_factor = 0;

      lFbxDouble1 = phong_mat->TransparencyFactor;
      if ( LiFBXFloatEqual(lFbxDouble1.Get(), 1) )
      {
         transparency_factor = 0;
      }
      else if(lFbxDouble1.Get()<0 || lFbxDouble1.Get()>1)
      {
         transparency_factor = 0;
      }
      else
      {
         transparency_factor = lFbxDouble1.Get();
      }

      nwc_mat.SetTransparency(transparency_factor);

      material_factors.m_transparency_factor = transparency_factor;
      material_factors.m_got_transparency_factor = true;


      // Display the Shininess
      lFbxDouble1 =phong_mat->Shininess;
      LtFloat shininess = 0;
      if (lFbxDouble1.Get()<0)
      {
         shininess = 0;
      }
      else
      {
         shininess =lFbxDouble1.Get()/100.0;
      }

      nwc_mat.SetShininess(shininess);


      material_factors.m_shininess = shininess;
      material_factors.m_got_shininess = true;

      // Display the Reflectivity
      // nwc has no reflection color
      lFbxDouble3 =phong_mat->Reflection;

      fb3 = lFbxDouble3.Get();

      material_factors.m_reflection = lFbxDouble3.Get();
      material_factors.m_reflection_factor = (phong_mat->ReflectionFactor).Get();
      material_factors.m_got_reflection = true;

      return nwc_mat;
   }
   else if(sur_mat->GetClassId().Is(FbxSurfaceLambert::ClassId) )
   {
      // We found a Lambert material. Display its properties.
      // Display the Ambient Color

      FbxSurfaceLambert* lambert_mat = FbxCast<FbxSurfaceLambert>(sur_mat);
      if (!lambert_mat)
      {
         return LcNwcMaterial(NULL);
      }

      lFbxDouble3=lambert_mat->Ambient;
      nwc_mat.SetAmbientColor(lFbxDouble3.Get()[0], 
         lFbxDouble3.Get()[1],
         lFbxDouble3.Get()[2]);

      material_factors.m_ambient = lFbxDouble3.Get();
      material_factors.m_got_ambient = true;

      // Display the Diffuse Color
      lFbxDouble3 =lambert_mat->Diffuse;
      nwc_mat.SetDiffuseColor(lFbxDouble3.Get()[0], 
         lFbxDouble3.Get()[1],
         lFbxDouble3.Get()[2]);

      material_factors.m_diffuse = lFbxDouble3.Get();
      material_factors.m_got_diffuse = true;


      // Display the Emissive
      lFbxDouble3 =lambert_mat->Emissive;
      nwc_mat.SetEmissiveColor(lFbxDouble3.Get()[0], 
         lFbxDouble3.Get()[1],
         lFbxDouble3.Get()[2]);

      material_factors.m_emissive = lFbxDouble3.Get();
      material_factors.m_got_emissive = true;
      material_factors.m_emissive_factor = (lambert_mat->EmissiveFactor).Get();

      //Set material transparent factor 1 when the value is 1 geting from fbxsdk. 
      //The transparent factor is always 0 in the fbx files from motionbuilder, 
      //but it is always 1 in the fbx files exported from maya. if not do so, nothing display for fbx files from maya
      LtFloat transparency_factor = 0;
      // Display the Opacity
      lFbxDouble1 = lambert_mat->TransparencyFactor;
      if ( LiFBXFloatEqual(lFbxDouble1.Get(), 1) )
      {
         transparency_factor = 0;
      }
      else if(lFbxDouble1.Get()<0 || lFbxDouble1.Get()>1)
      {
         transparency_factor = 0;
      }
      else
      {
         transparency_factor = lFbxDouble1.Get();
      }

      nwc_mat.SetTransparency(transparency_factor);

      material_factors.m_transparency_factor = transparency_factor;
      material_factors.m_got_transparency_factor = true;

      return nwc_mat;

   }
   else
   {
      return LcNwcMaterial(NULL);
   }
}
