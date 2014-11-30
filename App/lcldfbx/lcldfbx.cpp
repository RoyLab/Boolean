//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/lcldfbx.cpp#7 $
//   $Change: 68118 $
//   $Date: 2011/01/12 $
//
//   Description:
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
#include "LcFBXParameters.h"

///
/// NWCreate options load callback.
///
static void LI_NWC_API
def_options_cb(LtNwcLoader loader,
               LtNwcOptionSet option_set,
               void *user_data)
{
   LiUAssertAny(user_data != NULL);
   LcFBXParameters* const params = static_cast<LcFBXParameters* const>(user_data);
   params->DefineOptionSet(LcNwcOptionSet(option_set));
}


///
/// NWCreate scene complete callback
/// This function is called by NWcreate when the scene
/// has finished loading.
///
static void LI_NWC_API
scene_complete_cb(LtNwcScene scene_handle,
                  void* user_data)
{
}

///
/// NWCreate scene load callback.
/// This function is called by NWcreate to load an FBX file.
///
static LtNwcLoadStatus LI_NWC_API 
load_file_ex_cb(LtNwcLoader loader_handle, 
                LtWideString pathname, 
                LtNwcScene scene_handle, 
                LtNwcProgress progress_handle,
                void *user_data)
{
   LiNwcSceneSetSceneCompleteCallback(scene_handle,
                                      &scene_complete_cb,
                                      NULL);
   LcNwcScene nwc_scene(scene_handle);
   LcNwcProgress nwc_progress(progress_handle);

   LcFBXSceneStats stats;
   stats.Reset();

   const LcFBXParameters* const params = static_cast<LcFBXParameters*>(user_data);
   LcFBXScene fbx_scene(loader_handle, &nwc_scene,&nwc_progress,stats, *params);

   LtNwcLoadStatus load_status = fbx_scene.LoadFile(pathname);

   LcUWideStringBuffer buf;
   stats.GetSceneStats(buf);
   nwc_scene.SetStatistics(buf.GetString());
   nwc_scene.SetBackgroundColor(211.0/255, 231.0/255, 241.0/255);

   return load_status;
}

void LI_NWC_API
boom_cb(LtNwcLoader loader, void* user_data)
{
}

static LcFBXParameters f_parameters;

static LtNat8 f_license_data[] =
{
   0xa0, 0x15, 0x32, 0x1a, 0xe3, 0x15, 0x98, 0xad, 
   0x73, 0x95, 0x19, 0x7a, 0x6c, 0xad, 0x1d, 0xb7, 
   0xab, 0x0b, 0xd2, 0x75, 0x8a, 0x89, 0xd3, 0x54
};

///
/// Entry point for loader.
/// Setup callbacks for loading from file and changing loader
/// parameters.
///
extern "C" __declspec(dllexport) LtBoolean LI_NWC_API 
LiNwcLoaderEntry(LtNwcLoader loader_handle)
{
   LcNwcLoader loader(loader_handle);
   loader.SetLicenseData(f_license_data, sizeof(f_license_data));
   loader.SetLoadFileExCallback(&load_file_ex_cb, &f_parameters);
   loader.SetDefineOptionsCallback(&def_options_cb, &f_parameters);
   loader.SetTerminateCallback(&boom_cb, &f_parameters);
   return TRUE;
}