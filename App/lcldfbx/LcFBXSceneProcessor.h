//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXSceneProcessor.h#14 $
//   $Change: 101631 $
//   $Date: 2013/07/09 $
//
//   Description:
//      Scene Processor - Loading protein material
//
//
//   Copyright Notice:
//
//     Copyright 2010 Autodesk, Inc.  All rights reserved.
//  
//     This computer source code and related instructions and 
//     comments are the unpublished confidential and proprietary
//     information of Autodesk, Inc. and are protected under 
//     United States and foreign intellectual property laws.
//     They may not be disclosed to, copied or used by any third
//     party without the prior written consent of Autodesk, Inc.
//

#ifndef LCFBXSCENEPROCESSOR_HDR
#define LCFBXSCENEPROCESSOR_HDR

#include <stlport\map>

// *** Protein Interop ***
// String definitions, these strings are required by locating the Protein resource correctly.
//
#pragma warning(push)
#pragma warning(disable: 4819 4189)

#include "adskassetapi_new/assetlibmgrutils.h"
#include "adskassetapi_new/ipropertytable.h"
#include "adskassetapi_new/iassetinstance.h"
#include "adskassetapi_new/assettypes.h"
#include "adskassetapi_new/ipathresolver.h"
#include "adskassetapi_new/iassetlibrary.h"
#include "adskassetapi_new/iassetlibrarymanager.h"
#include "adskassetapi_new/iassetserializer.h"

#include "core/io/FileSystem.h"
#include "core/io/FileDataStream.h"
#include "core/string/UTF8String.h"
#include "core/string/WideString.h"

using namespace autodesk::platform::core::io;
using namespace autodesk::platform::core::string;
using namespace autodesk::platform::assets;

#pragma warning(pop)

#include "LcFBXLibraryRequest.h"
#include <lcogl/LcOglProteinAssetFactory.h>
#include <lcogl/LcOglProteinAsset.h>

class LcFBXScene;
class LcOglProteinAsset;

class LcFBXSceneProcessor
{
   typedef stlport::map<FbxString, LcNwcAutodeskMaterial> LtFBXAutodeskMaterialMap;
   typedef stlport::map<FbxString, LcOglProteinAsset*> LtFBXAutodeskAssetMap;

public :
   LcFBXSceneProcessor();
   ~LcFBXSceneProcessor();;
   LcFBXSceneProcessor& operator=( const LcFBXSceneProcessor& ) {}

public: 
   // This function is used to process a FbxScene after application imports it.
   bool PostRead(LcFBXScene* scene, FbxScene* pFbxScene, bool* hasblob);

   // Function for application to query resolved asset instance
   LtNwcAutodeskMaterial FindResolvedMaterial(const FbxString& assetID);

   LcOglProteinAsset* FindResolvedAsset(const FbxString& assetID);

   /// 
   /// Process protein light, create corresponding NW lights
   /// 
   void ProcessProteinLight(LcFBXScene* scene, LcOglProteinAsset* asset, FbxAMatrix& position_mat);

   /// <description>
   /// Load the system library
   /// </description>
   const bool InitializeAssetLibrary();

private:
   LtNat32 GetCurrentSystemLibraryVersion();

private:
   IAssetLibraryManager*        m_pAssetLibraryManager;
   LcFBXLibraryRequest*         m_pLibraryRequest;
   LtFBXAutodeskMaterialMap     m_resolvedMaterials;
   LtFBXAutodeskAssetMap        m_asset_map;
};



#endif //LCFBXSCENEPROCESSOR_HDR
