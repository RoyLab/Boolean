//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXScene.h#24 $
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

#ifndef LCFBXSCENE_HDR
#define LCFBXSCENE_HDR
#pragma once

#include "LcFBXParameters.h"
#include "LcFBXSceneProcessor.h"
#include "LiFBXMisc.h"

#include <map>

#include <lcutil/LcUString.h>

///
/// Presenter material map element
///
struct MaterialData
{
   MaterialData(): m_adsk_material(NULL), m_nwc_material(NULL), m_uv_swap(false), m_mapping_type(FbxTexture::eUMT_UV) {}

   LcNwcMaterial             m_nwc_material;
   LcNwcAutodeskMaterial     m_adsk_material;
   bool                      m_uv_swap;
   FbxTexture::EUnifiedMappingType m_mapping_type; 

};

typedef std::map<FbxUInt64, MaterialData> AttributeMap;

typedef enum 
{   
   eFBX_UNIDENTIFIED,
   eFBX_NULL,
   eFBX_MARKER,
   eFBX_SKELETON, 
   eFBX_MESH, 
   eFBX_NURB, 
   eFBX_PATCH, 
   eFBX_CAMERA, 
   eFBX_CAMERA_SWITCHER,
   eFBX_LIGHT,
   eFBX_OPTICAL_REFERENCE,
   eFBX_OPTICAL_MARKER,
   eFBX_NURBS_CURVE,
   eFBX_TRIM_NURBS_SURFACE,
   eFBX_BOUNDARY,
   eFBX_NURBS_SURFACE,
   eFBX_SHAPE,
   eFBX_LODGROUP,
   eFBX_LAST
} EFBXAttributeType;

typedef struct ConversionStat
{
   LtBoolean supported;
   LtNat32 nSuccess;
   LtNat32 nFailed;
   LtNat32 nIgnored;
   LtString sName;
} ConversionStat;

static ConversionStat sttcs[eFBX_LAST] = 
{ 
   { FALSE, 0, 0, 0, "lcldfbx_unidentified" },
   { FALSE, 0, 0, 0, "lcldfbx_null" }, 
   { FALSE, 0, 0, 0, "lcldfbx_marker" },
   { TRUE, 0, 0, 0, "lcldfbx_skeleton" },
   { TRUE, 0, 0, 0, "lcldfbx_mesh" },
   { TRUE, 0, 0, 0, "lcldfbx_nurb"}, 
   { TRUE, 0, 0, 0, "lcldfbx_patch" },
   { TRUE, 0, 0, 0, "lcldfbx_camera" },
   { FALSE, 0, 0, 0, "lcldfbx_camera_switcher" },
   { TRUE, 0, 0, 0, "lcldfbx_light" },
   { FALSE, 0, 0, 0, "lcldfbx_optical_reference" },
   { FALSE, 0, 0, 0, "lcldfbx_optical_marker" },
   { TRUE, 0, 0, 0, "lcldfbx_nurbs_curve" },
   { TRUE, 0, 0, 0, "lcldfbx_trim_nurbs_surface" },
   { FALSE, 0, 0, 0, "lcldfbx_boundary" },
   { TRUE, 0, 0, 0, "lcldfbx_nurbs_surface" },
   { FALSE, 0, 0, 0, "lcldfbx_shape" },
   { FALSE, 0, 0, 0, "lcldfbx_lodgroup" }
};

///
/// scene statistic class is used to store statistic
///
class LcFBXSceneStats
{
public:

   ///
   /// conversion status
   /// Successfully converted element.
   /// Failed to convert supported element.
   /// Ignored element because not yet supported.
   ///
   enum ConversionResult
   {
      eSUCCESS, 
      eFAILED,  
      eIGNORED  
   };


   /// 
   /// Registers the result of a conversion.
   /// Might want to specify the type as either an integer (like
   /// here) or as a string (might be useful for name file lookup).
   /// 
   /// @param attribute_type attribute type
   /// @param result conversion status
   /// 
   void RegisterResult(EFBXAttributeType attribute_type,
      ConversionResult result);

   /// 
   /// Generates scene statistics. Will look up element names
   /// from the name file.
   /// 
   /// @param stats statistic string
   /// 
   void GetSceneStats(LcUWideStringBuffer &stats);

   ///
   /// Reset the values to zero.
   ///
   void Reset();
};


///
/// FBX Scene class.
/// Encapsulates our NavisWorks scene and adds methods for dealing with scene statistics
///
class LcFBXScene
{
public:

   /// 
   /// Creates a new scene object.
   /// 
   /// @param scene nwc scene
   /// @param progress nwc progress
   /// @param stats FBX scene statistic
   /// @param params FBX parameters
   /// 
   LcFBXScene(LtNwcLoader loader_handle,
      LcNwcScene* scene, 
      LcNwcProgress* progress, 
      LcFBXSceneStats& stats, 
      const LcFBXParameters& params);


   /// 
   /// destructor
   /// 
   virtual ~LcFBXScene();

   /// 
   /// Main loading function
   /// 
   /// @param pathname input FBX file path
   /// 
   LtNwcLoadStatus LoadFile(LtWideString pathname);


   LtNwcLoader GetNwcLoaderHandle() const { return m_loader_handle; }
   const LtWChar* GetPathName() const { return m_path_name.GetString(); }

   /// 
   /// Get nwc scene
   /// 
   LcNwcScene* GetNwcScene() {return m_scene;}


   /// 
   /// Get material map
   /// 
   AttributeMap& GetMaterialDict() {return m_material_dict;}


   /// 
   /// Get FBX parameters
   /// 
   const LcFBXParameters& GetFBXParameters() { return m_params; }


   /// 
   /// store all the specific node's materials in material map 
   /// 
   bool StoreMaterials(FbxNode* node);

   /// 
   /// Get the current animation stack
   /// 
   FbxAnimStack* GetCurrentAnimationStack() {return m_CurrentAnimationStack;};

   /// 
   /// Get the current animation layer
   /// 
   FbxAnimLayer* GetCurrentAnimationLayer() {return m_CurrentAnimationLayer;};

   /// 
   /// Get the scene processor
   /// 
   LcFBXSceneProcessor& GetSceneProcessor() {return m_SceneProcessor;};

   bool IsAutodeskGraphics() {return m_convert_adsk_mtl;};
   bool HasAdskAsset(){return m_has_adsk_asset;}

   LtNwcLinearUnits GetSceneUnits(){return m_scene_units;}
private:

   /// 
   /// Draw the fbx scene
   /// 
   bool DrawScene();

   /// 
   /// Draw one specific fbx node
   /// 
   /// @param node FBX node
   /// @param nwc_parent_node nwc parent node
   /// @param parent_global_position parent global position
   /// 
   bool DrawNode(FbxNode* node, LcNwcGroup& nwc_parent_node, FbxAMatrix* parent_global_position);

   /// 
   /// convert from a FBX file into the FBX scene
   /// 
   /// @param pathname import FBX file path
   /// 
   LtNwcLoadStatus ImportFile(LtWideString pathname);

   /// 
   /// Get the current time, and get the current animation stack and layer.
   /// 
   void GetTakeInfomation();

private:

   LtNwcLoader m_loader_handle;
   LcNwcScene* m_scene;
   LcUWideString m_path_name;
   LcNwcProgress* m_progress; 
   FbxManager* m_sdk_manager; 
   FbxScene* m_sdk_scene;
   FbxPose*  m_first_pose;
   LcFBXSceneStats& m_stats;
   const LcFBXParameters& m_params;

   FbxArray<FbxNode*> m_cameras;
   FbxTime m_start_time;
   LtInt32 m_node_count;
   LtInt32 m_processed_node_count;

   AttributeMap m_material_dict;
   FbxAnimStack* m_CurrentAnimationStack;
   // This is the animation layer that is currently active and where
   // the animation curves are extracted from
   FbxAnimLayer* m_CurrentAnimationLayer; 
   LcFBXSceneProcessor m_SceneProcessor;

   // whether the fbx file has adsk material asset
   bool m_has_adsk_asset;
   // whether needs to convert the adsk materials (the original material could be protein materal or not)
   bool m_convert_adsk_mtl;

   LtNwcLinearUnits m_scene_units;

   bool m_cancelled;
};


#endif // LCFBXSCENE_HDR
