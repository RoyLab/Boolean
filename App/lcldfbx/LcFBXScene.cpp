//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXScene.cpp#42 $
//   $Change: 110043 $
//   $Date: 2014/02/05 $
//
//   Description:
//      Fbx Scene - loading FBX file
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
#include "LiFBXMisc.h"
#include "LcFBXScene.h"
#include "LcFBXGeometry.h"
#include "LcFBXLight.h"
#include "LcFBXCamera.h"
#include "LcFBXAdskMtlUtil.h"
#include "LcFBXMaterial.h"

static LtWideString 
f_get_recource_string(LtString name)
{
   // returns a resource sting
   LtWideString str = LiNwcApiFindName(name);
   LiUAssertAny(str);
   return str;
}

static bool
f_get_unit(LtNwcLinearUnits& nwc_unit, LtFloat unit_scale)
{
   const LtNwcLinearUnits to = LI_NWC_UNITS_CENTIMETERS;

   if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_METERS , to) ) )
   {
      nwc_unit = LI_NWC_UNITS_METERS;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_CENTIMETERS, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_CENTIMETERS;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_MILLIMETERS, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_MILLIMETERS;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_FEET, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_FEET;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_INCHES, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_INCHES;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_YARDS, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_YARDS;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_KILOMETERS, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_KILOMETERS;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_MILES, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_MILES;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_MICROMETERS, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_MICROMETERS;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_MILS, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_MILS;
   }
   else if ( LiFBXFloatEqual(unit_scale, LiNwcSceneConvertLinearUnit(1.0, LI_NWC_UNITS_MICROINCHES, to) ) )
   {
      nwc_unit = LI_NWC_UNITS_MICROINCHES;
   }
   else
   {
      LiUAssertAny(!L"can't match unit!");
      return false;
   }
   return true;

}


void LcFBXSceneStats::Reset()
{
   for(LtNat32 i=0; i<eFBX_LAST; i++)
   {
      sttcs[i].nSuccess=0;
      sttcs[i].nFailed=0;
      sttcs[i].nIgnored=0;
   }
}

void LcFBXSceneStats::GetSceneStats(LcUWideStringBuffer &stats)
{

   LcUWideStringBuffer temp;
   LcUWideStringBuffer converted;
   LcUWideStringBuffer failed;
   LcUWideStringBuffer ignored;

   converted.Append(f_get_recource_string("lcldfbx_converted"));
   converted.Append(L"\r\n");
   failed.Append(f_get_recource_string("lcldfbx_failed"));
   failed.Append(L"\r\n");
   ignored.Append(f_get_recource_string("lcldfbx_ignored"));
   ignored.Append(L"\r\n");

   for(LtNat32 i=0; i<eFBX_LAST; i++)
   {
      if(sttcs[i].sName == NULL) continue;
      if(sttcs[i].supported)
      {
         if(sttcs[i].nSuccess > 0)
         {
            temp.Format(L"\t%d %s\r\n", sttcs[i].nSuccess, f_get_recource_string(sttcs[i].sName));
            converted.Append(temp.GetString());

            sttcs[0].nSuccess+=sttcs[i].nSuccess;
         }

         if(sttcs[i].nFailed > 0)
         {
            temp.Format(L"\t%d %s\r\n", sttcs[i].nFailed, f_get_recource_string(sttcs[i].sName));
            failed.Append(temp.GetString());

            sttcs[0].nFailed+=sttcs[i].nFailed;
         }

         if (sttcs[i].nIgnored > 0)
         {
            temp.Format(L"\t%d %s\r\n", sttcs[i].nIgnored, f_get_recource_string(sttcs[i].sName));
            ignored.Append(temp.GetString());

            sttcs[0].nFailed+=sttcs[i].nIgnored;
         }
      }
      else
      {
         if(sttcs[i].nIgnored > 0)
         {
            temp.Format(L"\t%d %s\r\n", sttcs[i].nIgnored, f_get_recource_string(sttcs[i].sName));
            ignored.Append(temp.GetString());

            sttcs[0].nIgnored+=sttcs[i].nIgnored;
         }
      }
   }

   temp.Format(L"%s\r\n\t %s %d, %s %d, %s %d\r\n", 
      f_get_recource_string("lcldfbx_summary"), 
      f_get_recource_string("lcldfbx_converted"), 
      sttcs[0].nSuccess, 
      f_get_recource_string("lcldfbx_failed"),
      sttcs[0].nFailed, 
      f_get_recource_string("lcldfbx_ignored"),
      sttcs[0].nIgnored);

   stats.Append(L"\r\n");
   stats.Append(converted.GetString());
   stats.Append(L"\r\n");
   stats.Append(failed.GetString());
   stats.Append(L"\r\n");
   stats.Append(ignored.GetString());
   stats.Append(L"\r\n");

   stats.Append(temp.GetString());
}

void LcFBXSceneStats::RegisterResult(EFBXAttributeType attribute_type,
                                     ConversionResult result)
{
   LiUAssertAny(attribute_type<eFBX_LAST);
   if(result == eSUCCESS)
   {
      sttcs[attribute_type].nSuccess++;
   }
   else if(result == eFAILED)
   {
      sttcs[attribute_type].nFailed++;
   }
   else
   {
      sttcs[attribute_type].nIgnored++;
   }
}


static void 
f_initialize_sdk_objects(FbxManager*& Sdk_Manager, FbxScene*& pscene)
{
   // The first thing to do is to create the FBX SDK manager which is the 
   // object allocator for almost all the classes in the SDK.
   Sdk_Manager = FbxManager::Create();

   if (Sdk_Manager)
   {

      // create an IOSettings object
      FbxIOSettings * ios = FbxIOSettings::Create(Sdk_Manager, IOSROOT );
      Sdk_Manager->SetIOSettings(ios);

      // Create the entity that will hold the scene.
      pscene = FbxScene::Create(Sdk_Manager,"FBX_Sdk_Scene");
   }
}

static void 
f_destroy_sdk_objects(FbxManager*& sdk_Manager)
{
   // Delete the FBX SDK manager. All the objects that have been allocated 
   // using the FBX SDK manager and that haven't been explicitely destroyed 
   // are automatically destroyed at the same time.
   if (sdk_Manager) sdk_Manager->Destroy();
   sdk_Manager = NULL;
}

LcFBXScene::LcFBXScene(LtNwcLoader loader_handle, LcNwcScene* scene,
                       LcNwcProgress* progress, LcFBXSceneStats& stats, const LcFBXParameters& params)
: m_loader_handle(loader_handle)
, m_scene(scene)
, m_progress(progress)
, m_sdk_manager(NULL)
, m_sdk_scene(NULL)
, m_first_pose(NULL)
, m_stats(stats)
, m_params(params)
, m_node_count(0)
, m_processed_node_count(0)
, m_CurrentAnimationStack(NULL)
, m_CurrentAnimationLayer(NULL)
, m_has_adsk_asset(false)
, m_convert_adsk_mtl(false)
, m_scene_units(LI_NWC_UNITS_CENTIMETERS)
, m_cancelled(false)
{
   f_initialize_sdk_objects(m_sdk_manager,m_sdk_scene);
}

LcFBXScene::~LcFBXScene()
{
   f_destroy_sdk_objects(m_sdk_manager);
   m_sdk_scene = NULL;
   m_first_pose = NULL;
}


LtNwcLoadStatus 
LcFBXScene::LoadFile(LtWideString pathname)
{
   if (!pathname)
   {
      return LI_NWC_LOAD_CANT_OPEN;
   }

   m_path_name = pathname;

   if (!m_sdk_manager || !m_sdk_scene)
   {
      return LI_NWC_LOAD_ERROR;
   }

   LtNwcLoadStatus import_status = ImportFile(pathname);

   if ( LI_NWC_LOAD_OK != import_status )
   {
      return import_status;
   }

   if(GetFBXParameters().GetIsConvertAutodeskMaterials())
   {
      m_convert_adsk_mtl=true; // need to load FBX materials to adsk material
      m_SceneProcessor.InitializeAssetLibrary();
      m_SceneProcessor.PostRead(this, m_sdk_scene, &m_has_adsk_asset);

      m_scene->SetPreferredGraphicsSystem(LI_NWC_GRAPHICS_AUTODESK);

   }
   
   if (DrawScene())
   {
      return LI_NWC_LOAD_OK;
   }
   else
   {
      if(m_cancelled)
      {
         return LI_NWC_LOAD_CANCELED;
      }
      else
      {
         return LI_NWC_LOAD_ERROR;
      }
   }
}

bool
LcFBXScene::DrawScene()
{
   FbxGlobalSettings& global_setting = m_sdk_scene->GetGlobalSettings();

   FbxAxisSystem scene_axis_system = m_sdk_scene->GetGlobalSettings().GetAxisSystem();

   LtInt32 up_sign = 0;
   FbxAxisSystem::EUpVector up_vec = scene_axis_system.GetUpVector(up_sign);
   if ( (1==up_sign) || (-1==up_sign))
   {
      if(FbxAxisSystem::eXAxis == up_vec)
      {
         m_scene->SetWorldUpVector(up_sign, 0, 0);
      }
      else if(FbxAxisSystem::eYAxis == up_vec)
      {
         m_scene->SetWorldUpVector(0, up_sign, 0);
      }
      else if (FbxAxisSystem::eZAxis == up_vec)
      {
         m_scene->SetWorldUpVector(0, 0, up_sign);
      }
   }

   FbxSystemUnit scene_unit_system = global_setting.GetSystemUnit();
   LtNwcLinearUnits nwc_unit = LI_NWC_UNITS_CENTIMETERS;
   LtFloat unit_scale = scene_unit_system.GetScaleFactor();

   bool got_unit = false;

   got_unit = f_get_unit(nwc_unit, unit_scale);
   if (got_unit)
   {
      m_scene->SetLinearUnits(nwc_unit);
      m_scene_units = nwc_unit;
   }
   else
   {
      m_scene->SetLinearUnits(LI_NWC_UNITS_CENTIMETERS);
      m_scene_units = LI_NWC_UNITS_CENTIMETERS;
   }

   m_node_count = m_sdk_scene->GetNodeCount();
   m_processed_node_count = 0;
   if (m_node_count<1)
   {
      return false;
   }

   FbxNode* lnode = m_sdk_scene->GetRootNode();
   LiUAssertAny(lnode);
   if (!lnode)
   {
      return false;
   }

   // get root node, and set its name
   LtChar const * root_name = lnode->GetName();
   LcUWideStringBuffer wroot_name;
   if (root_name)
   {
      LcUString::UTF8ToWideString(root_name, &wroot_name);
   }
   LcNwcGroup nwc_root_node;
   if (!wroot_name.IsEmpty())
   {
      nwc_root_node.SetName(wroot_name.GetString());
   }

   GetTakeInfomation();

   // recursively draw the node.
   FbxAMatrix parent_global_position;
   if(lnode)
   {
      for(LtInt32 i = 0 ; i < lnode->GetChildCount() && !m_cancelled ; i++)
      {
         DrawNode(lnode->GetChild(i), nwc_root_node, &parent_global_position);
      }
   }

   for (LtInt32 i=0; i<nwc_root_node.GetNumChildren(); ++i)
   {
      m_scene->AddNode(nwc_root_node.GetChild(i));
   }

   // add viewpoints to scene
   LcFBXCamera fbx_camera(this);
   fbx_camera.SetViewpointsFromCamera(m_scene, m_sdk_scene, &m_start_time, m_cameras);

   return true;
}

bool
LcFBXScene::DrawNode(FbxNode* pnode, LcNwcGroup& nwc_parent_node, FbxAMatrix* parent_global_position)
{
   LtChar const* nodename = pnode->GetName();

   LiUAssertAny(pnode);
   if (!pnode)
   {
      return false;
   }

   LtChar const* cname = pnode->GetName();  
   LcUWideStringBuffer wname;
   if (cname)
   {
      LcUString::UTF8ToWideString(cname, &wname);
   }

   LcNwcNode nwc_node(NULL);

   FbxAMatrix global_position = LiFBXGetGlobalPosition(pnode, m_first_pose, &m_start_time, parent_global_position);
   FbxAMatrix geometry_offset =  LiFBXGetGeometryOffset(pnode);
   FbxAMatrix global_off_position = global_position*geometry_offset; 

   // Extract protein light here, one node can have none attribute, but may have 
   // protein light attached.
   int count = pnode->GetSrcObjectCount();
   for (int i=0; i<count; i++)
   {
      FbxObject* lProxyObject = pnode->GetSrcObject(i); 
      if (!lProxyObject)
         continue;

      FbxProperty lProxyIdProperty = lProxyObject->FindProperty(LcOglProteinStringDefinitions::GetAdskReferencesBlobId());
      if (!lProxyIdProperty.IsValid())
         continue;

      FbxString lProxyId = lProxyIdProperty.Get<FbxString>();
      LcOglProteinAsset* asset = m_SceneProcessor.FindResolvedAsset(lProxyId);
      
      if (asset)
      {
         // Process protein light and add it to Navisworks scene
         m_SceneProcessor.ProcessProteinLight(this, asset, global_position);
      }
   }

   FbxNodeAttribute::EType attribute_type = FbxNodeAttribute::eUnknown;
   if( NULL!=pnode->GetNodeAttribute() )
   {
      attribute_type = (pnode->GetNodeAttribute()->GetAttributeType());
      switch (attribute_type)
      {
      case FbxNodeAttribute::eUnknown:
         {
            m_stats.RegisterResult(eFBX_UNIDENTIFIED, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eNull:  
         {
            m_stats.RegisterResult(eFBX_NULL, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eMarker:  
         {
            m_stats.RegisterResult(eFBX_MARKER, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eSkeleton:  
      case FbxNodeAttribute::eMesh:
      case FbxNodeAttribute::eNurbs: 
      case FbxNodeAttribute::ePatch:
      case FbxNodeAttribute::eNurbsSurface:
      case FbxNodeAttribute::eNurbsCurve:
      case FbxNodeAttribute::eTrimNurbsSurface:
         {
            if (FbxNodeAttribute::eSkeleton == attribute_type && 
               !m_params.GetIsConvertSkeleton() )
            {
               m_stats.RegisterResult(static_cast<EFBXAttributeType>(attribute_type), LcFBXSceneStats::eIGNORED);
               break;
            }

            LcFBXGeometry mesh(m_sdk_manager, this, attribute_type);
            nwc_node = mesh.Draw(pnode, 
               global_off_position, 
               *parent_global_position, 
               *this, 
               wname.GetString(),
               &m_start_time, 
               m_first_pose);

            if (nwc_node.GetHandle())
            {  
               m_stats.RegisterResult(static_cast<EFBXAttributeType>(attribute_type), LcFBXSceneStats::eSUCCESS);                  
            }
            else
            {
               m_stats.RegisterResult(static_cast<EFBXAttributeType>(attribute_type), LcFBXSceneStats::eFAILED);
            } 
         }
         break;

      case FbxNodeAttribute::eCamera:    
         m_cameras.Add(pnode);
         break;

      case FbxNodeAttribute::eCameraSwitcher:
         {
            m_stats.RegisterResult(eFBX_CAMERA_SWITCHER, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eLight: 
         {
            LcFBXLight light(this);
            bool add_light =false;
            if (m_params.GetIsConvertLight())
            {
               add_light = light.AddNwcLight(pnode, m_start_time,*this, global_position);
               if (add_light)
               {
                  m_stats.RegisterResult(eFBX_LIGHT, LcFBXSceneStats::eSUCCESS);
               }
               else
               {
                  m_stats.RegisterResult(eFBX_LIGHT, LcFBXSceneStats::eFAILED);
               }
            }
            else
            {
               m_stats.RegisterResult(eFBX_LIGHT, LcFBXSceneStats::eIGNORED);
            }
         }
         break;

      case FbxNodeAttribute::eOpticalReference:     
         {
            m_stats.RegisterResult(eFBX_OPTICAL_REFERENCE, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eOpticalMarker:     
         {
            m_stats.RegisterResult(eFBX_OPTICAL_MARKER, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eBoundary:
         {
            m_stats.RegisterResult(eFBX_BOUNDARY, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eShape:     
         {
            m_stats.RegisterResult(eFBX_SHAPE, LcFBXSceneStats::eIGNORED);
         }
         break;

      case FbxNodeAttribute::eLODGroup:
         {
            m_stats.RegisterResult(eFBX_LODGROUP, LcFBXSceneStats::eIGNORED);
         }
         break;
      }
   }

   LcNwcGroup nwc_group;

   for(LtInt32 i = 0; i < pnode->GetChildCount() && !m_cancelled; i++)
   {
      DrawNode(pnode->GetChild(i), nwc_group, &global_position);
   }

   if (pnode->GetChildCount() > 0)
   {
      if(!wname.IsEmpty())
      {
         nwc_group.SetName(wname.GetString());
      }
      nwc_group.AddNode(nwc_node);
      nwc_parent_node.AddNode(nwc_group);
   }
   else
   {
      if(!wname.IsEmpty())
      {
         nwc_node.SetName(wname.GetString());
      }
      nwc_parent_node.AddNode(nwc_node);
   }

   m_processed_node_count++;
   m_cancelled = !(m_progress->Update(static_cast<LtFloat>(m_processed_node_count) / static_cast<LtFloat>(m_node_count)));

   return true;
}

static bool LiFbxProgressCallback(void* pArgs, float pPercentage, const char* pStatus)
{
   LcNwcProgress *progress = static_cast<LcNwcProgress*>(pArgs);
   return progress->Update(pPercentage / 100.0);
}

LtNwcLoadStatus 
LcFBXScene::ImportFile(LtWideString pathname)
{
   if (!pathname)
   {
      return LI_NWC_LOAD_CANT_OPEN;
   }

   LcUStringBuffer utf8_pathname;
   LcUWideString::ExtractUTF8String(pathname, &utf8_pathname);
   if (utf8_pathname.IsEmpty())
   {
      return LI_NWC_LOAD_CANT_OPEN;
   }

   // Get the version number of the FBX files generated by the
   // version of FBX SDK that you are using.
   LtInt32 lsdk_major = -1;
   LtInt32 lsdk_minor = -1;
   LtInt32 lsdk_revision = -1;
   FbxManager::GetFileFormatVersion(lsdk_major, lsdk_minor, lsdk_revision);


   // Create an importer.
   FbxImporter* limporter = FbxImporter::Create(m_sdk_manager,"FBX_Sdk_Importer");
   if (!limporter)
   {
      return LI_NWC_LOAD_ERROR;
   }

   // Initialize the importer by providing a filename.
   const bool limport_status = limporter->Initialize(utf8_pathname.GetString(), -1, m_sdk_manager->GetIOSettings());

   //
   // If we have progress object, register progress callback.
   //
   if(m_progress)
   {
      limporter->SetProgressCallback(&LiFbxProgressCallback, m_progress);
   }

   // Get the version number of the FBX file format.
   LtInt32 lfile_major = -1;
   LtInt32 lfile_minor = -1;
   LtInt32 lfile_revision = -1;
   limporter->GetFileVersion(lfile_major, lfile_minor, lfile_revision);

   if( !limport_status )  // Problem with the file to be imported
   {
      FbxStatus status = limporter->GetStatus(); 
      if (status == FbxStatus::eInvalidFileVersion)
      {
         return LI_NWC_LOAD_BAD_VERSION;
      }
      else if(status == FbxStatus::eInvalidFile)
      {
         return LI_NWC_LOAD_FILE_CORRUPT;
      }

      return LI_NWC_LOAD_ERROR;
   }

   if (limporter->IsFBX())
   {
      // In FBX, a scene can have one or more "takes". A take is a
      // container for animation data.
      // You can access a file's take information without
      // the overhead of loading the entire file into the scene.

      // this code segment is not necessary.
      LtInt32 ltake_count = limporter->GetAnimStackCount();

      for(LtInt32 i = 0; i < ltake_count; i++)
      {
         FbxTakeInfo* ltake_info = limporter->GetTakeInfo(i);
      }
   }

   // Import the scene.
   bool lStatus = false;
   if(m_progress)
   {
      m_progress->BeginSubOp(0.4);
   }

   lStatus = limporter->Import(m_sdk_scene);

   if(m_progress)
   {
      m_progress->EndSubOp();
   }

   //
   // Clear progress callback.
   //
   limporter->SetProgressCallback(NULL, NULL);

   if(lStatus == false &&     // The import file may have a password
      limporter->GetStatus() == FbxStatus::ePasswordError)
   {
      LiUAssertAny(!L"Password Error!");// to do sth about password problems
   }

   // Destroy the importer
   if (limporter)
   {
      limporter->Destroy();
   }

   if (lStatus)
   {
      return LI_NWC_LOAD_OK;
   }
   else
   {
      return LI_NWC_LOAD_FILE_CORRUPT;
   }
}


bool 
LcFBXScene::StoreMaterials(FbxNode* node)
{
   if (!node)
   {
      return false;
   }

   LtInt32 lmaterial_index = -1;
   FbxProperty lproperty;

   LtInt32 lnbmat = node->GetMaterialCount();

   if (lnbmat<=0)
   {
      return false;
   }

   bool got_nwc_texture = false;
   for (lmaterial_index = 0; lmaterial_index < lnbmat; lmaterial_index++)
   {
      FbxSurfaceMaterial *lmaterial = (FbxSurfaceMaterial *)node->GetMaterial(lmaterial_index);

      //go through all the possible textures
      if(lmaterial)
      {
         bool got_textures = false;
         bool temp_got_texture = false;

         LcNwcAutodeskMaterial adsk_material(NULL);
         LcNwcMaterial          nwc_material(NULL);

         FbxUInt64 m_id = lmaterial->GetUniqueID();

         // find the material in the dict
         AttributeMap::const_iterator result = m_material_dict.find(m_id);
         if(result == m_material_dict.end())
         {      
            LcFBXMaterialFactors material_factors;
            nwc_material = LcFBXMaterial::GetNwcMaterial(lmaterial,material_factors);
            bool uv_swap =false;

            FbxTexture::EUnifiedMappingType mapping_type = FbxTexture::eUMT_UV;
            adsk_material = LcFBXAdskMtlUtil::CreateAdskMtl(lmaterial, *this, material_factors, uv_swap, mapping_type);

            if (nwc_material.GetHandle() || adsk_material.GetHandle())
            {
               MaterialData pm_data;
               pm_data.m_adsk_material = adsk_material;
               pm_data.m_nwc_material = nwc_material;
               pm_data.m_uv_swap = uv_swap;
               pm_data.m_mapping_type = mapping_type;
               m_material_dict.insert(AttributeMap::value_type(m_id,pm_data));
            }  
         }

      }//end if(lmaterial)

   }// end for lmateria_lindex
   return got_nwc_texture;  
}

void
LcFBXScene::GetTakeInfomation()
{
   FbxTimeSpan lTimeLineTimeSpan;
   m_sdk_scene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);
   m_start_time = lTimeLineTimeSpan.GetStart();

   FbxArray<FbxString*>   lTakeNameArray;
   m_sdk_scene->FillAnimStackNameArray(lTakeNameArray);

   if ( lTakeNameArray.GetCount() > 0 )
   {
      LtInt32 lCurrentTakeIndex = lTakeNameArray.GetCount();
      // Add the take names.
      for (LtInt32 i = 0; i < lTakeNameArray.GetCount(); i++)
      {
         // Track the current take index.
         if (lTakeNameArray[i]->Compare(m_sdk_scene->ActiveAnimStackName.Get()) == 0)
         {
            lCurrentTakeIndex = i;
            break;
         }
      }

      if (lCurrentTakeIndex < lTakeNameArray.GetCount())
      {
         // select the base layer from the animation stack
         m_CurrentAnimationStack = m_sdk_scene->FindMember<FbxAnimStack>(lTakeNameArray[lCurrentTakeIndex]->Buffer());
         if (m_CurrentAnimationStack)
         {
            // we assume that the first animation layer connected to the animation stack is the base layer
            // (this is the assumption made in the FBXSDK)
            m_CurrentAnimationLayer = m_CurrentAnimationStack->GetMember<FbxAnimLayer>();
            m_sdk_scene->GetEvaluator()->SetContext(m_CurrentAnimationStack);

            FbxTakeInfo* lCurrentTakeInfo = m_sdk_scene->GetTakeInfo(*(lTakeNameArray[lCurrentTakeIndex]));
            if (lCurrentTakeInfo)
            {
               m_start_time = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
            }
         }
      }
   }

   FbxArrayDelete(lTakeNameArray);
   return;
}