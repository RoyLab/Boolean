//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXAdskMtlUtil.cpp#6 $
//   $Change: 110684 $
//   $Date: 2014/02/20 $
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
#include "precompiled.h"
#include "LcFBXScene.h"
#include "LcFBXAdskMtlUtil.h"
#include "LcFBXMaterial.h"
#include "LiFBXMisc.h"
#include "LiFBXNETApi.h"

#include <lcutil/LcUSysFile.h>
#include <lcutil/LcUTempFile.h>
#include <lcogl/LcOglProteinURI.h>
#include <lcogl/LcOglProteinProperty.h>
#include <lcogl/LcOglProteinAsset.h>

// *** Protein Interop ***
// String definitions, these strings are required by locating the Protein resource correctly.
//
#pragma warning(push)
#pragma warning(disable: 4819 4189)

#include "adskassetapi_new/assetlibmgrutils.h"
#include "adskassetapi_new/ipropertytable.h"
#include "adskassetapi_new/iassetinstance.h"
#include "adskassetapi_new/ipathresolver.h"
#include "adskassetapi_new/iassetlibrary.h"
#include "adskassetapi_new/iassetlibrarymanager.h"
#include "adskassetapi_new/iassetserializer.h"

#include "core/io/FileSystem.h"
#include "core/io/FileDataStream.h"

using namespace autodesk::platform;
using namespace autodesk::platform::core::io;
using namespace autodesk::platform::assets;
using namespace autodesk::platform::core::string;

#pragma warning(pop)

struct FbxUVMappingData
{
   bool uv_swap;
   FbxTexture::EUnifiedMappingType mapping_type;

   FbxUVMappingData():uv_swap(false), mapping_type(FbxTexture::eUMT_UV)
   {}
};

struct ProteinMtlData
{
   IAssetInstance* asset_inst;
   LcOglProteinAsset* ogl_asset;
   IAssetLibraryManager* lib_manager;
   LtFloat color[3];
   LtNwcLinearUnits scene_units;
   LtFloat transparency;
   bool got_refl; // reflection
   LtFloat refl_factor;
   bool got_shininess;
   LtFloat shininess;
   LcFBXScene* scene;

   struct DiffuseMapData
   {
      LcUWideString file_path;
      bool has_alpha;

      DiffuseMapData():has_alpha(false)
      {}
   };
   DiffuseMapData diffuse_map;

   ProteinMtlData():asset_inst(NULL), ogl_asset(NULL), lib_manager(NULL), transparency(0), got_refl(false), got_shininess(false),scene(NULL)
   {
      color[0]=color[1]=color[2]=80.0f/256; // the default diffuse color of generic protein material
   }
};

static bool IsProteinMapReadable(const LtWChar* filename)
{
   if (LcUSysFile::HasFileExtensionW(filename, L"jpg"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"jpeg"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"png"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"tif"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"tiff"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"bmp"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"exr"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"hdr"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"dib"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"pcx"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"gif"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"tga"))
      return true;

   if (LcUSysFile::HasFileExtensionW(filename, L"rle"))
      return true;

   return false;
}

static adp::IDesignPropertyUnits::teType GetProteinUnits(LtNwcLinearUnits nw_units)
{
   switch(nw_units)
   {
   case LI_NWC_UNITS_METERS: return adp::IDesignPropertyUnits::eUMeter;
   case LI_NWC_UNITS_CENTIMETERS: return adp::IDesignPropertyUnits::eUCentiMeter;
   case LI_NWC_UNITS_MILLIMETERS: return adp::IDesignPropertyUnits::eUMilliMeter;
   case LI_NWC_UNITS_FEET: return adp::IDesignPropertyUnits::eUFoot;
   case LI_NWC_UNITS_INCHES: return adp::IDesignPropertyUnits::eUInch;
   case LI_NWC_UNITS_YARDS: return adp::IDesignPropertyUnits::eUYard;
   case LI_NWC_UNITS_KILOMETERS: return adp::IDesignPropertyUnits::eUKiloMeter;
   case LI_NWC_UNITS_MILES: return adp::IDesignPropertyUnits::eUMile;
   case LI_NWC_UNITS_MICROMETERS: return adp::IDesignPropertyUnits::eUMicroMeter;
   case LI_NWC_UNITS_MILS: return adp::IDesignPropertyUnits::eUMil;
   case LI_NWC_UNITS_MICROINCHES: return adp::IDesignPropertyUnits::eUMicroInch;
   default:
      LiUAssertAny(0);
      return adp::IDesignPropertyUnits::eUInch;
   }
}

// if b_temp_file is true, the input file is a temp file,
// the created LcUTempFile will reference to it directly
static LcUTempFile* GetTempFile(const LtWChar* file, bool b_temp_file=false)
{
   if(!LcUSysFile::ExistsW(file))
      return NULL;

   LcUTempFile* tfile = NULL;
   if(b_temp_file)
   {
      tfile = new LcUTempFile(file);
   }
   else
   {
      LcUWideStringBuffer dir;

      LcUWideStringBuffer extension;
      LcUSysFile::GetFileExtensionW(file, &extension);
      LcUSysFile::MakeTempFileW(NULL, L"Navisworks", extension.GetString(), 0, &dir);
      if(dir.IsEmpty())
         return NULL;

      LcUSysFile::CopyFileW(file, dir.GetString());

      if(!LcUSysFile::ExistsW(dir.GetString()))
         return NULL;

      tfile = new LcUTempFile(dir.GetString());
   }
   return tfile;
}

// if b_temp_map is true, the file map_dir references to will be deleted
// when the LcUTempFile object destroyed.
static LcOglProteinAsset* CreateUnifiedBitmap(FbxTexture* ltexture,
                       LtNwcLinearUnits scene_units,
                       FbxUVMappingData fbx_uv,
                       const LtWChar* map_dir, 
                       IAssetLibraryManager* lib_manager,
                       bool b_invert,
                       bool b_temp_map=false)
{
   LcOglProteinAsset* ogl_map = NULL;
   IAssetInstance* map_inst = NULL; 
   LtWChar* ubmp_schema_id = L"UnifiedBitmapSchema";
   IAssetSchema* ubmp_schema = lib_manager->FindAssetSchema(WideString(ubmp_schema_id));
   if (ltexture && !LcUWideString::IsEmpty(map_dir) && ubmp_schema)
   {
      LtFloat u_scale=ltexture->GetScaleU();
      LtFloat v_scale=ltexture->GetScaleV();
      LtFloat soffset = ltexture->GetTranslationU();
      LtFloat toffset = ltexture->GetTranslationV();

      if (fbx_uv.uv_swap)
      {
         toffset = ltexture->GetTranslationU();
         soffset = ltexture->GetTranslationV();
         v_scale=ltexture->GetScaleU();
         u_scale=ltexture->GetScaleV();
      }

      if (FbxTexture::eUMT_XY==fbx_uv.mapping_type||
         FbxTexture::eUMT_XZ==fbx_uv.mapping_type||
         FbxTexture::eUMT_YZ==fbx_uv.mapping_type)
      {
         soffset = 0;
         toffset = 1;
         u_scale =1;
         v_scale =1;
      }

      LtFloat rotationU=ltexture->GetRotationU();
      LtFloat rotationV=ltexture->GetRotationV();
      LtFloat rotationW=ltexture->GetRotationW();

      ///
      /// In FBXSDK, the texture rotation vector is constantly (0,0,0), no matter the real value of the texture rotation.
      /// For example, if the texture rotation vector is (0,0,90), (we can see these value from ascii file) but the vector gotten from FBXSDK is still (0,0,0);
      ///
      FbxDouble3 rv3 = ltexture->Rotation.Get();

      LtFloat rotation =0;
      if (!LiFBXFloatEqual(0,rotationU))
      {
         rotation = rotationU;
      }
      else if (!LiFBXFloatEqual(0,rotationV))
      {
         rotation = rotationV;
      }
      else if (!LiFBXFloatEqual(0,rotationW))
      {
         rotation = rotationW;
      }
      else
      {
         rotation = 0;
      }

      LtFloat rotation2=0;
      if(rotation)
      {
         // the rotation2 have to fall into [0,360]
         rotation2=rotation; 
         rotation2 *= -1;
         if(rotation2>360)
            rotation2-=360*(int)(rotation2/360);
         else if(rotation2<0)
            rotation2-=360*((int)(rotation2/360)-1);
      }

      bool u_repeat=(ltexture->GetWrapModeU()==FbxTexture::eRepeat);
      bool v_repeat=(ltexture->GetWrapModeV()==FbxTexture::eRepeat);

      // unifided bitmap instance
      map_inst = lib_manager->InstantiateAsset(ubmp_schema, WideString(IAssetLibraryManager::kSystemLibraryIdentifier), WideString(ubmp_schema_id));

      if (map_inst)
      {
         LcOglProteinAssetEncoder* ogl_encoder = LcOglProteinAssetFactory::CreateEncoder(NULL, NULL);
         if (ogl_encoder)
         {
            ogl_map = ogl_encoder->Encode(map_inst);
            LcOglProteinAssetFactory::DestroyEncoder(ogl_encoder);
         }
         if(ogl_map)
         {
            LcUTempFile* temp_file = GetTempFile(map_dir, b_temp_map);
            if(temp_file)
            {
               LcOglProteinProperty* ogl_property=NULL;
               ogl_property = ogl_map->FindProperty(L"unifiedbitmap_Bitmap");
               if(ogl_property)
                  ogl_property->SetEmbeddedURI(temp_file);

               LcOglProteinProperty* u_scale_prop = NULL;
               u_scale_prop = ogl_map->FindProperty(L"texture_UScale");
               if(u_scale_prop)
                  u_scale_prop->SetScalar(u_scale);

               LcOglProteinProperty* u_real_scale_prop = NULL;
               u_real_scale_prop = ogl_map->FindProperty(L"texture_RealWorldScaleX");
               if(u_real_scale_prop)
               {
                  // unitless UV coordinate, which will be in [0,1].
                  // Setting the protein real world scale to 0 turns off real world sizing
                  u_real_scale_prop->SetScalar(0);
               }

               v_scale *= -1; // Because when set TexCoord of object, the y was multiplied by -1 for presenter material.
               LcOglProteinProperty* v_scale_prop = NULL;
               v_scale_prop = ogl_map->FindProperty(L"texture_VScale");
               if(v_scale_prop)
                  v_scale_prop->SetScalar(v_scale);

               LcOglProteinProperty* v_real_scale_prop = NULL;
               v_real_scale_prop = ogl_map->FindProperty(L"texture_RealWorldScaleY");
               if(v_real_scale_prop)
               {
                  // Same as texture_RealWorldScaleX
                  v_real_scale_prop->SetScalar(0);
               }

               LcOglProteinProperty* u_offset_prop=NULL;
               u_offset_prop = ogl_map->FindProperty(L"texture_UOffset");
               if(u_offset_prop)
                  u_offset_prop->SetScalar(soffset);

               toffset = 1+toffset;// plus 1, because of the - on the v_scale
               LcOglProteinProperty* v_offset_prop=NULL;
               v_offset_prop = ogl_map->FindProperty(L"texture_VOffset");
               if(v_offset_prop)
                  v_offset_prop->SetScalar(toffset);
               u_offset_prop = ogl_map->FindProperty(L"texture_RealWorldOffsetX");
               if(u_offset_prop)
               {
                  // No real world offset
                  u_offset_prop->SetScalar(0);
               }

               v_offset_prop = ogl_map->FindProperty(L"texture_RealWorldOffsetY");
               if(v_offset_prop)
               {
                  // No real world offset
                  v_offset_prop->SetScalar(0);
               }

               LcOglProteinProperty* rot_prop = NULL;
               rot_prop = ogl_map->FindProperty(L"texture_WAngle");
               if(rot_prop)
                  rot_prop->SetScalar(rotation2);

               LcOglProteinProperty* utiling_prop = NULL;
               utiling_prop = ogl_map->FindProperty(L"texture_URepeat");
               if(utiling_prop)
                  utiling_prop->SetBoolean(u_repeat);

               LcOglProteinProperty* vtiling_prop = NULL;
               vtiling_prop=ogl_map->FindProperty(L"texture_VRepeat");
               if(vtiling_prop)
                  vtiling_prop->SetBoolean(v_repeat);

               if(b_invert) // For transparency map, the image may need to be inverted
               {
                  LcOglProteinProperty* invert_prop=NULL;
                  invert_prop = ogl_map->FindProperty(L"unifiedbitmap_Invert");
                  if(invert_prop)
                     invert_prop->SetBoolean(b_invert);
               }

            } //if(temp_file)

#ifdef _DEBUG
            ogl_map->Dump();     
#endif
         }//if(ogl_map)
         map_inst->Destroy();
      } //if (map_inst)
   }//if (uv_gen && !LcUWideString::IsEmpty(map_dir) && ubmp_schema)

   return ogl_map;
}

static bool ConvertDiffuseMap(FbxTexture* fbx_tex,
                       ProteinMtlData& ptn_mtl_data,
                       const LtWChar* map_dir,
                       FbxUVMappingData fbx_uv)
{
   LcOglProteinAsset* ogl_asset(ptn_mtl_data.ogl_asset);
   IAssetLibraryManager* lib_manager(ptn_mtl_data.lib_manager);
   if(!ogl_asset || !fbx_tex || !lib_manager)
      return false;

   LcOglProteinProperty* new_di_prop = NULL;

   //diffuse colour
   LcOglProteinProperty* di_color_prop = NULL;
   di_color_prop = ogl_asset->FindProperty(L"generic_diffuse");
   if (!di_color_prop)
   {
      new_di_prop = new LcOglProteinProperty;
      new_di_prop->SetIdentifier(L"generic_diffuse");
      di_color_prop = new_di_prop;
   }

   // diffuse map
   if (di_color_prop)
   {
      di_color_prop->SetColorRGB(1);
      di_color_prop->SetColorRGBAtIndex(0,ptn_mtl_data.color);

      LcOglProteinAsset* ogl_map_di = CreateUnifiedBitmap(fbx_tex, ptn_mtl_data.scene_units, fbx_uv, map_dir, lib_manager, false);
      if(ogl_map_di)
      {
         di_color_prop->RemoveAllConnectedAssets();
         di_color_prop->AddConnectedAsset(ogl_map_di);
         di_color_prop->SetConnectedAssetEnabled(true);
      }

      if(ptn_mtl_data.got_shininess)
      {
         LcOglProteinProperty* glos_prop = NULL;
         glos_prop = ogl_asset->FindProperty(L"generic_glossiness");
         if (glos_prop)
         {
            glos_prop->SetScalar(ptn_mtl_data.shininess);
         }
      }
   }

   if(new_di_prop)
   {
      ogl_asset->AddProperty(new_di_prop);
      delete new_di_prop;
   }

   return true;
}

static bool ConvertBumpMap(   FbxTexture* fbx_tex,
                       ProteinMtlData& ptn_mtl_data,
                       const LtWChar* map_dir,
                       FbxUVMappingData fbx_uv)
{
   LcOglProteinAsset* ogl_asset(ptn_mtl_data.ogl_asset);
   IAssetLibraryManager* lib_manager(ptn_mtl_data.lib_manager);
   if(!ogl_asset || !fbx_tex || !lib_manager)
      return false;

   if(LcUWideString::IsEmpty(map_dir))
      return false;

   LcOglProteinProperty* new_bump_prop = NULL;
   // bump property
   LcOglProteinProperty* bump_prop = NULL;
   bump_prop = ogl_asset->FindProperty(L"generic_bump_map");
   if (!bump_prop) // if there is no bump property, just new one.
   {
      new_bump_prop = new LcOglProteinProperty;
      new_bump_prop->SetIdentifier(L"generic_bump_map");
      bump_prop = new_bump_prop;
   }

   bool added = false;
   if (bump_prop)
   {
      LcOglProteinAsset* ogl_map_bump = CreateUnifiedBitmap(fbx_tex, ptn_mtl_data.scene_units, fbx_uv, map_dir, lib_manager, false);
      if(ogl_map_bump)
      {
         bump_prop->RemoveAllConnectedAssets();
         bump_prop->AddConnectedAsset(ogl_map_bump);
         bump_prop->SetConnectedAssetEnabled(true);
         added = true;
      }
   }

   if(new_bump_prop)
   {
      if(added)
         ogl_asset->AddProperty(new_bump_prop);
      delete new_bump_prop;
   }

   return true;
}

static bool ConvertTransparencyMap(FbxTexture* fbx_tex,
                       ProteinMtlData& ptn_mtl_data,
                       const LtWChar* map_dir,
                       FbxUVMappingData fbx_uv)
{
   LcOglProteinAsset* ogl_asset(ptn_mtl_data.ogl_asset);
   IAssetLibraryManager* lib_manager(ptn_mtl_data.lib_manager);
   if(!ogl_asset || !fbx_tex || !lib_manager)
      return false;

   LcOglProteinProperty* new_trans_prop = NULL;
   // transparency property
   LcOglProteinProperty* trans_prop = NULL;
   trans_prop = ogl_asset->FindProperty(L"generic_transparency");
   if (!trans_prop) // if there is no transparency property, just new one.
   {
      new_trans_prop = new LcOglProteinProperty;
      new_trans_prop->SetIdentifier(L"generic_transparency");
      trans_prop = new_trans_prop;
   }

   if (trans_prop)
   {
      trans_prop->SetScalar(ptn_mtl_data.transparency);

      if(!LcUWideString::IsEmpty(map_dir))
      {
         LcOglProteinAsset* ogl_map_trans = CreateUnifiedBitmap(fbx_tex, ptn_mtl_data.scene_units, fbx_uv, map_dir, lib_manager, true);
         if(ogl_map_trans)
         {
            trans_prop->RemoveAllConnectedAssets();
            trans_prop->AddConnectedAsset(ogl_map_trans);
            trans_prop->SetConnectedAssetEnabled(true);
         }
      }
   }

   if(new_trans_prop)
   {
      ogl_asset->AddProperty(new_trans_prop);
      delete new_trans_prop;
   }

   return true;
}

static bool ConvertCutoutMap(FbxTexture* fbx_tex,
                       ProteinMtlData& ptn_mtl_data,
                       FbxUVMappingData fbx_uv,
                       const LtWChar* map_dir,
                       bool is_temp_map_dir)
{
   LcOglProteinAsset* ogl_asset(ptn_mtl_data.ogl_asset);
   IAssetLibraryManager* lib_manager(ptn_mtl_data.lib_manager);
   if(!ogl_asset || !fbx_tex || !lib_manager)
      return false;

   LcOglProteinProperty* new_cutout_prop = NULL;
   // cutout property
   LcOglProteinProperty* cutout_prop = NULL;
   cutout_prop = ogl_asset->FindProperty(L"generic_cutout_opacity");
   if (!cutout_prop) // if there is no cutout property, just new one.
   {
      new_cutout_prop = new LcOglProteinProperty;
      new_cutout_prop->SetIdentifier(L"generic_cutout_opacity");
      cutout_prop = new_cutout_prop;
   }

   bool ret = false;
   if (cutout_prop)
   {
      if(!LcUWideString::IsEmpty(map_dir))
      {
         LcOglProteinAsset* ogl_map_trans = CreateUnifiedBitmap(fbx_tex, ptn_mtl_data.scene_units, fbx_uv, map_dir, lib_manager, true, true);
         if(ogl_map_trans)
         {
            cutout_prop->RemoveAllConnectedAssets();
            cutout_prop->AddConnectedAsset(ogl_map_trans);
            cutout_prop->SetConnectedAssetEnabled(true);

         	ret= true;
         }
      }
   }

   if(new_cutout_prop)
   {
      ogl_asset->AddProperty(new_cutout_prop);
      delete new_cutout_prop;
   }

   return ret;
}

// lazy initialization
// if returns true, and *protein_inst is not null, caller has to destroy it.
bool InitProteinAsset(IAssetLibraryManager* lib_manager, IAssetInstance** protein_inst, LcOglProteinAsset** protein_asset)
{
   if(!lib_manager||!protein_inst||!protein_asset)
      return false;

   *protein_inst = NULL;
   *protein_asset = NULL;

   // Create asset instance
   IAssetInstance* asset_inst = NULL; 
   LtWChar* schema_id = L"Generic";
   {
      // The old way of instantiate protein asset.
      // but it can initialize the thumbnail and catagory
      const IAssetLibrary* asset_lib = lib_manager->GetLibraryByIdentifier(WideString(IAssetLibraryManager::kSystemLibraryIdentifier));
      if (asset_lib != NULL)
      {
         IAssetDefinition::IIterator& def_it = asset_lib->GetAssetByIdentifier(WideString(schema_id));
         if (!def_it.IsEmpty())
         {
            IAssetDefinition* def = def_it.GetAssetDefinition();
            if (def != NULL)
            {
               asset_inst = asset_lib->InstantiateAsset(*def);
               def->Destroy();
            }
         }
         asset_lib->DestroyIterator(def_it);
      }
   }

   // Get protein asset
   if (asset_inst)
   {
      LcOglProteinAssetEncoder* ogl_encoder = LcOglProteinAssetFactory::CreateEncoder(NULL, NULL);;
      LcOglProteinAsset* ogl_asset = NULL;
      if (ogl_encoder)
      {
         ogl_asset = ogl_encoder->Encode(asset_inst);
         LcOglProteinAssetFactory::DestroyEncoder(ogl_encoder);
         ogl_encoder = NULL;
      }
      if(ogl_asset)
      {
         *protein_inst = asset_inst;
         *protein_asset = ogl_asset;

         return true;
      }
      else
      {
         asset_inst->Destroy();
         return false;
      }
   }

   return false;
}


static bool
ConvertTexture(FbxTexture* ltexture,
                LtChar const* property_type,
                ProteinMtlData& ptn_data,
                FbxUVMappingData& fbx_uv)
{
   if (!ltexture||!ptn_data.lib_manager)
   {
      return false;
   }
   // ptn_data.asset_inst and ptn_data.ogl_asset will be lazy initialized when convert diffuse texture
   // but if the texture type is not diffuse, the ogl_asset should have been initialized.
   if((0 != LcUString::Stricmp(property_type,FbxSurfaceMaterial::sDiffuse)) && !ptn_data.ogl_asset)
   {
      return false;
   }

   // navisworks only support one uv channel
   // so only returns the uv proterties for diffuse map.
   bool t_uv_swap = ltexture->GetSwapUV();
   FbxTexture::EUnifiedMappingType t_mapping_type = ltexture->CurrentMappingType.Get();
   FbxUVMappingData temp_uv;
   temp_uv.uv_swap = t_uv_swap;
   temp_uv.mapping_type = t_mapping_type;

   LtChar const* texture_name = ltexture->GetName();
   FbxFileTexture* ftexture = FbxCast<FbxFileTexture>(ltexture);
   
   LtChar const* pff = NULL;
   LtChar const* pfr = NULL;
   FbxString  kstr_pff;
   FbxString  Kstr_pfr;
   if(ftexture)
   {
      kstr_pff = ftexture->GetFileName(); 
      Kstr_pfr = ftexture->GetRelativeFileName();
      pff = kstr_pff.Buffer();
      pfr = Kstr_pfr.Buffer();
   }

   LcUWideStringBuffer pf;
   
   if (!LiFBXResolveFile(ptn_data.scene->GetPathName(),pff,&pf))
   {
      pf.SetEmptyString();
      if(!LiFBXResolveFile(ptn_data.scene->GetPathName(),pfr,&pf))
         pf.SetEmptyString();
   }

   LcUWideStringBuffer pfw;
   if (!pf.IsEmpty())
   {
      pfw = pf;

      // if the image format is not supported by protein
      // convert it to jpg
      if(!IsProteinMapReadable(pfw.GetString()))
         LiFBXConvert2JPG(pfw);
   }
   
   // diffuse texture
   if ( 0 == LcUString::Stricmp(property_type,FbxSurfaceMaterial::sDiffuse) )
   {
      fbx_uv = temp_uv;
      if(!pfw.IsEmpty() && InitProteinAsset(ptn_data.lib_manager, &ptn_data.asset_inst, &ptn_data.ogl_asset)
         && ptn_data.ogl_asset )
      {
         bool ret = ConvertDiffuseMap(ltexture, ptn_data, pfw.GetString(), temp_uv);
         if(ret)
         {
            /*
            FBX support textures with alpha channel,
            but Protein can’t support the textures with alpha channel as good as Maya/Sketchup, 
            Protein only can support black as alpha source (Black is 100% transparency, white is opaque.). 
            To work around this, when get a texture, fbx loader will check if it has alpha channel, 
            if so, extract the alpha channel and make a mask image based on it, 
            then set the mask as the Cutout texture in protein.
            */
            // check the alpha channel of the texture map
            LcUWideString op_mask;
            if(LiFBXTryGetOpacityMask(pfw.GetString(), &op_mask))
            {
               // FBX can use the alpha channel of the texture map to set transparency
               // so here need to set the cutout texture
               ConvertCutoutMap(ltexture, ptn_data, temp_uv, op_mask.GetString(), true);

               // For FBX file from Maya
               // In Maya, when add a diffuse texture with alpha channel
               // Maya will add a transparency texture automatically, and it references to the same image as diffuse map
               // As we have added the cutout map for the alpha channel,
               // the transparency texture added by Maya is redundant here.
               // So here save the diffuse map file path,
               // when processing transparency texture, check the image file path to see if it is the same texture file as diffuse map,
               // if so, just ignore the transparency texture.
               ptn_data.diffuse_map.file_path = pfw.GetString();
               ptn_data.diffuse_map.has_alpha = true;
            }
         }
         return ret;
      }
   }

   // transparent texture
   else if ( 0 == LcUString::Stricmp(property_type,FbxSurfaceMaterial::sTransparentColor) )
   {
      if(!pfw.IsEmpty())
      {
         // see the comment above "For FBX file from Maya"
         if( ptn_data.diffuse_map.file_path.IsEmpty() || 
            !ptn_data.diffuse_map.has_alpha || 
            !LcUWideString::Equal(pfw.GetString(), ptn_data.diffuse_map.file_path.GetString()) )
            return ConvertTransparencyMap(ltexture, ptn_data, pfw.GetString(), temp_uv);
      }
   }
   // bump texture
   else if (0 == LcUString::Stricmp(property_type,FbxSurfaceMaterial::sBump) )
   {
      if(!pfw.IsEmpty())
         return ConvertBumpMap(ltexture, ptn_data, pfw.GetString(), temp_uv);
   }

   return false;
}


static bool
ConvertTexture(FbxProperty& lproperty,
                LtChar const* property_type,
                ProteinMtlData& ptn_data,
                FbxUVMappingData& fbx_uv)
{
   if( lproperty.IsValid() )
   {

      FbxTexture * ltexture= NULL;

      //Here we have to check if it's layeredtextures, or just textures:
      LtInt32 llayered_texture_count = lproperty.GetSrcObjectCount<FbxLayeredTexture>();
      
      if(llayered_texture_count > 0)
      {
         for(LtInt32 j=0; j<llayered_texture_count; ++j)
         {
            FbxLayeredTexture *llayered_texture = FbxCast <FbxLayeredTexture> (lproperty.GetSrcObject<FbxLayeredTexture>(j));
 
            LtInt32 lnbtextures = llayered_texture->GetSrcObjectCount<FbxTexture>();
            for(LtInt32 k =0; k<lnbtextures; ++k)
            {
               ltexture = FbxCast<FbxTexture>(llayered_texture->GetSrcObject<FbxTexture>(k));
               if(ltexture)
               {
                  
                  bool temp_create_shader = false;
                  temp_create_shader = ConvertTexture(ltexture,property_type,ptn_data, fbx_uv);
                  if (temp_create_shader)
                  {
                     return true;
                  }
                  
                 
                  //NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the  texture.
                  //Why is that?  because one texture can be shared on different layered  textures and might
                  //have different blend modes.

                  FbxTexture::EBlendMode lBlendMode = llayered_texture->GetBlendMode();
                  
               }

            }
         }
      }
      else
      {
         //no layered texture simply get on the property
         LtInt32 lnbtextures = lproperty.GetSrcObjectCount<FbxTexture>();
         
         for(LtInt32 j =0; j<lnbtextures; ++j)
         {

            FbxTexture* ltexture = FbxCast <FbxTexture> (lproperty.GetSrcObject<FbxTexture>(j));
            if(ltexture)
            {
               bool temp_create_shader = false;
               temp_create_shader = ConvertTexture(ltexture,property_type,ptn_data, fbx_uv);
               if (temp_create_shader)
               {
                  return true;
               }
            }
         }
      }
      

   }//end if pProperty

   return NULL;
}

LcNwcAutodeskMaterial LcFBXAdskMtlUtil::CreateAdskMtl(FbxSurfaceMaterial* material,
      LcFBXScene& fbx_scene,
      LcFBXMaterialFactors& material_factors,
      bool& uv_swap,
      FbxTexture::EUnifiedMappingType& mapping_type)
{
   if(!fbx_scene.IsAutodeskGraphics())
      return NULL;

   if (!material)
   {
      return NULL;
   }

   IAssetLibraryManager* lib_manager = NULL;
   if (LcOglProteinAssetFactory::InitialiseProteinAssetFactory())
   {
      lib_manager = GetIAssetLibraryManager();
   }
   if(!lib_manager)
      return NULL;
         
   LcNwcAutodeskMaterial adsk_mtl(NULL);

   bool got_textures = false;
   bool temp_got_texture =false;
   FbxProperty lproperty;
   FbxUVMappingData di_uv; // diffuse

   // diffuse map
   {
      ProteinMtlData ptn_data;
      ptn_data.lib_manager = lib_manager;
      // asset_inst&ogl_asset lazy initialization
      ptn_data.asset_inst = NULL;
      ptn_data.ogl_asset = NULL;
      ptn_data.scene_units = fbx_scene.GetSceneUnits();
      ptn_data.scene = &fbx_scene;
      if(material_factors.m_got_diffuse)
      {
         ptn_data.color[0] = material_factors.m_diffuse[0];
         ptn_data.color[1] = material_factors.m_diffuse[1];
         ptn_data.color[2] = material_factors.m_diffuse[2];
      }
      if(material_factors.m_got_reflection)
      {
         ptn_data.got_refl = true;
         ptn_data.refl_factor = material_factors.m_reflection_factor;
      }
      if(material_factors.m_got_shininess)
      {
         ptn_data.got_shininess = true;
         ptn_data.shininess = material_factors.m_shininess;
      }
      if(material_factors.m_got_transparency_factor)
         ptn_data.transparency = material_factors.m_transparency_factor;
      else
         ptn_data.transparency = 0;
      ptn_data.diffuse_map.has_alpha = false;

      //Diffuse Textures
      lproperty = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
      temp_got_texture = ConvertTexture(lproperty, FbxSurfaceMaterial::sDiffuse, ptn_data, di_uv);
      got_textures = got_textures || temp_got_texture; 
            
      LcOglProteinAsset* ogl_asset = NULL;
      ogl_asset = ptn_data.ogl_asset;
      if(temp_got_texture && ogl_asset) // create protein material, only if there is diffuse map
      {
         uv_swap = di_uv.uv_swap;
         mapping_type = di_uv.mapping_type;

         // self illumination 
         if(material_factors.m_got_emissive)
         {
            LcOglProteinProperty* self_illu_prop = NULL;
            self_illu_prop = ogl_asset->FindProperty(L"generic_self_illum_luminance");
            if(self_illu_prop)
               self_illu_prop->SetScalar(material_factors.m_emissive_factor);
            // self illumination color
            LcOglProteinProperty* illu_color_prop = NULL;
            illu_color_prop = ogl_asset->FindProperty(L"generic_self_illum_filter_map");
            if(illu_color_prop)
            {
               LtFloat color[3];
               color[0] = material_factors.m_emissive[0];
               color[1]=material_factors.m_emissive[1];
               color[2]=material_factors.m_emissive[2];
               illu_color_prop->SetColorRGB(1);
               illu_color_prop->SetColorRGBAtIndex(0,color);
            }
         }

         //Bump Textures
         FbxUVMappingData bu_uv;
         lproperty = material->FindProperty(FbxSurfaceMaterial::sBump);
         temp_got_texture = ConvertTexture(lproperty, FbxSurfaceMaterial::sBump, ptn_data, bu_uv);
         got_textures = got_textures || temp_got_texture;  
            
         //Transparent Textures
         FbxUVMappingData tr_uv;
         lproperty = material->FindProperty(FbxSurfaceMaterial::sTransparentColor);
         temp_got_texture = ConvertTexture(lproperty, FbxSurfaceMaterial::sTransparentColor, ptn_data, tr_uv);
         got_textures = got_textures || temp_got_texture; 

         const LtChar* mname = material->GetName();
         LcUWideStringBuffer wmname;
         if (mname)
         {
            LcUString::UTF8ToWideString(mname, &wmname);
         }
            
         // Set Asset Name
         ogl_asset->SetUIName(wmname.GetString());

#ifdef _DEBUG
         ogl_asset->Dump();
#endif

         LcNwcAutodeskMaterial tmp_mtl;
         tmp_mtl.SetName(wmname.GetString());
         tmp_mtl.SetMaterialAsset(reinterpret_cast<LtNwcAutodeskAsset>(ogl_asset));
         adsk_mtl = tmp_mtl;
      } //if(ogl_asset)

      if(ptn_data.asset_inst)
         ptn_data.asset_inst->Destroy();
   } 

   return adsk_mtl;
}

bool LcFBXAdskMtlUtil::AttachAdskMtl(FbxNode* node,
      FbxGeometry* pgeometry,
      LcFBXScene& fbx_scene,
      LcNwcGeometry& geo,
      bool& uv_swap,
      FbxTexture::EUnifiedMappingType& mapping_type,
      bool& adjust_uv)
{
   adjust_uv = false;

   if(!fbx_scene.IsAutodeskGraphics())
      return false;
   if (!pgeometry || !node)
   {
      return false;
   }

   bool got_nwc_texture = false;

   // first try getting the protein materials from Blob
   int proxyMatCount = node->GetSrcObjectCount(); 
   for(int matIndex=0; matIndex <proxyMatCount; ++matIndex)
   {
      LcNwcAutodeskMaterial material = LcFBXAdskMtlUtil::GetAdskMtlFromBlob(fbx_scene, node, matIndex);
      if(material.GetHandle() != NULL)
      {
         geo.AddAttribute(material);
         got_nwc_texture = true;
         adjust_uv = false;
         break;
      }
   }
   if(got_nwc_texture)
   {
      return true;
   }

   // then try converting fbx materials to protein
   LtInt32 lmaterial_index = -1;
   FbxProperty lproperty;

   LtInt32 lnbmat = node->GetSrcObjectCount<FbxSurfaceMaterial>();
   
   if (lnbmat<=0)
   {
      return false;
   }
   for (lmaterial_index = 0; lmaterial_index < lnbmat; lmaterial_index++)
   {
      FbxSurfaceMaterial *lmaterial = (FbxSurfaceMaterial *)node->GetSrcObject<FbxSurfaceMaterial>(lmaterial_index);

      //go through all the possible textures
      if(lmaterial)
      {
         bool got_textures = false;

         LcNwcAutodeskMaterial nwc_pmaterial;

         FbxUInt64 m_id = lmaterial->GetUniqueID();

         // find the material in the dict
         AttributeMap::const_iterator result = fbx_scene.GetMaterialDict().find(m_id);
         if(result != fbx_scene.GetMaterialDict().end())
         {
            nwc_pmaterial = result->second.m_adsk_material;
            uv_swap = result->second.m_uv_swap;
            mapping_type = result->second.m_mapping_type;
            got_textures = true;
         }
         else
         {
            LcFBXMaterialFactors material_factors;
            bool temp_uv_swap = false;

            LcFBXMaterial::GetNwcMaterial(lmaterial,material_factors);

            nwc_pmaterial = LcFBXAdskMtlUtil::CreateAdskMtl(lmaterial, fbx_scene, material_factors, temp_uv_swap, mapping_type);
               
            if (nwc_pmaterial.GetHandle())
            {
               MaterialData pm_data;
               pm_data.m_adsk_material = nwc_pmaterial;
               pm_data.m_uv_swap = temp_uv_swap; //  temp uv ??
               fbx_scene.GetMaterialDict().insert(AttributeMap::value_type(m_id,pm_data));
               uv_swap = temp_uv_swap;
               got_textures = true;
            }  
         }

         if (got_textures)
         {
            LcNwcScene* nwc_scene = fbx_scene.GetNwcScene();
            if (nwc_scene)
            {
               geo.AddAttribute(nwc_pmaterial);
               got_nwc_texture = true;
               adjust_uv = true;

               break;
            }

         }

      }//end if(lmaterial)

   }// end for lmateria_lindex
   return got_nwc_texture;
}

LcNwcAutodeskMaterial LcFBXAdskMtlUtil::GetAdskMtlFromBlob(LcFBXScene& fbx_scene,
                                                           FbxNode* node,
                                                           LtNat32 mtl_id)
{
   if(!node)
      return NULL;

   if(fbx_scene.HasAdskAsset())
   {
      FbxObject* proxyObj = node->GetSrcObject(mtl_id);
      if(proxyObj)
      {
         FbxProperty proxyProp = proxyObj->FindProperty(LcOglProteinStringDefinitions::GetAdskReferencesBlobId());
         if(proxyProp.IsValid())
         {
            FbxString lassetReferenceID = proxyProp.Get<FbxString>();
            LtNwcAutodeskMaterial material = fbx_scene.GetSceneProcessor().FindResolvedMaterial(lassetReferenceID);
            if(material != NULL)
            {
               return material;
            }
         }
      }
   }

   return NULL;
}
