//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXSceneProcessor.cpp#31 $
//   $Change: 108044 $
//   $Date: 2013/12/19 $
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

#include <precompiled.h>

///
/// Some sample header files.
/// Review these and add/remove as required.
///
#include <lcutil/LcUString.h>
#include <lcutil/LcUSysFile.h>
#include <lcutil/LcUError.h>
#include <lcutil/LcUMemorySTLAlloc.h>
#include <lcutil/LcUResource.h>
#include <lclinear\LcLRotation.h>
#include <lclinear\LcLTransform.h>

#include "LcFBXSceneProcessor.h"
#include "LcFBXScene.h"

// -----------------------------------------------------------------------------------------------
// Utility class to allow read of data from existing binary blob without need for copy
// Based on AIRMax Janus Turnpike ADPMemStream sample

namespace adpio = autodesk::platform::core::io;

class LcFBXMemReadStream : public adpio::IDataStream
{
public:
    LcFBXMemReadStream(BYTE* buffer, std::size_t size)
       : m_buffer(buffer), m_offset(0), m_dataSize(size)
    {
    }

    virtual bool open() { return true; }
    virtual bool close() { return true; }
    virtual bool truncate() { return true; }

    /// IDataReadStream methods
    ///
    virtual uint64_t rseek(int64_t iOffset, adpio::teStreamPosition eOrigin)
    {
       switch (eOrigin) 
       {
       case adpio::eBegin:
          // do nothing
          break;
       case adpio::eCurrent:
          iOffset += m_offset;
          break;
       case adpio::eEnd:
          iOffset += m_dataSize;
       }

       m_offset = iOffset < 0 ? 0 
          : ((uint64_t)iOffset > m_dataSize ? m_dataSize : toSizeT_(iOffset));

       return m_offset;
    }
    virtual uint64_t rfind(int64_t iOffset, adpio::teStreamPosition eOrigin)
    {
       switch (eOrigin) {
       case adpio::eBegin:
          if (iOffset < 0 || (uint64_t)iOffset > m_dataSize)
             return m_offset;  // Should I throw an exception instead??

          m_offset = toSizeT_(iOffset);
          break;
       case adpio::eCurrent:
          {
             int64_t tmpPos = m_offset + iOffset;
             if (tmpPos < 0 || (uint64_t)tmpPos > m_dataSize)
                return m_offset;  // Should I throw an exception instead??

             m_offset = toSizeT_(tmpPos);
          }
          break;
       case adpio::eEnd:
          if (iOffset > 0 || (uint64_t)(-iOffset) > m_dataSize)
             return m_offset;  // Should I throw an exception instead??

          m_offset = toSizeT_(m_dataSize + iOffset);
       }
       return m_offset;
    }

    /// IDataReader methods
    virtual bool openForRead()
    { m_offset = 0; return true; }
    virtual bool closeForRead()
    { return true; }
    virtual bool availableToRead() const
    { return m_offset < m_dataSize; }
    virtual size_t read(void* pData, size_t outSize)
    {
       if (!availableToRead()
          || !outSize || !pData)
       {
          return 0;
       }
       size_t bytesToCopy = (size_t)(m_dataSize - m_offset);
       if (bytesToCopy >= outSize)
          bytesToCopy = outSize;

       ::memcpy(pData, &m_buffer[m_offset], bytesToCopy);
       m_offset += bytesToCopy;
       return bytesToCopy;
    }
    virtual bool fill(void* pBuffer, size_t nBytes)
    {
       if (m_dataSize < nBytes || !pBuffer)
          return false;

       ::memcpy(pBuffer, &m_buffer[0], nBytes);
       return true;
    }


    /// IDataWriteStream methods
    virtual uint64_t wseek(int64_t iOffset, adpio::teStreamPosition eOrigin)
    { LiUAssertAny(FALSE); return 0; }
    virtual uint64_t wfind(int64_t iOffset, adpio::teStreamPosition eOrigin)
    { LiUAssertAny(FALSE); return 0; }

    /// IDataWriter methods

    virtual bool openForWrite()
    { LiUAssertAny(FALSE); return false; }
    virtual bool closeForWrite()
    { LiUAssertAny(FALSE); return false; }
    virtual bool availableToWrite() const
    { LiUAssertAny(FALSE); return false; }
    virtual size_t write(const void* pBuffer, size_t nBytes)
    { LiUAssertAny(FALSE); return 0; }
    virtual bool commit(const void* pBuffer, size_t nBytes)
    { LiUAssertAny(FALSE); return false; }

private:
    static std::size_t toSizeT_(uint64_t s)  {
        return static_cast<std::size_t>(s);
    }
    static std::size_t toSizeT_(int64_t s)   {
        return static_cast<std::size_t>(s);
    }

    BYTE* m_buffer;
    std::size_t m_dataSize;
    std::size_t m_offset;
};

// -----------------------------------------------------------------------------------------------
// Utility class to allow protein encoder to resolve file references
class LcFbxProteinFileResolver : public LcOglProteinAssetFileResolver
{
public:
   LcFbxProteinFileResolver(LcFBXScene* scene) : m_scene(scene) {}

   virtual bool Resolve(const LtWChar* filename, LcUWideStringBuffer* buffer)
   {
      buffer->Reserve(2000);
      bool ok = (LiNwcLoaderResolveXRef(m_scene->GetNwcLoaderHandle(), m_scene->GetPathName(), 
         filename, TRUE, buffer->BeginEditBuffer()) == LI_NWC_XREF_OK);
      buffer->EndEditBuffer();
      return ok;
   }

private:
   LcFBXScene* m_scene;
};

// -----------------------------------------------------------------------------------------------

LcFBXSceneProcessor::LcFBXSceneProcessor()
   : m_pAssetLibraryManager(NULL)
   , m_pLibraryRequest(NULL)
{

}

LcFBXSceneProcessor::~LcFBXSceneProcessor()
{
   m_resolvedMaterials.clear();

   LtFBXAutodeskAssetMap::iterator it = m_asset_map.begin();
   for (; it != m_asset_map.end(); ++it)
   {
      LcOglProteinAsset* asset = it->second;
      if (asset)
         asset->Unref();
   }
   m_asset_map.clear();

   if(m_pLibraryRequest)
   {
      delete m_pLibraryRequest;
      m_pLibraryRequest = NULL;
   }
}

const bool LcFBXSceneProcessor::InitializeAssetLibrary()
{
   bool ret = false;
   if (m_pAssetLibraryManager == NULL)
   {
      // *** Protein Interop ***
      // Get IAssetLibraryManager and initialize it if it's the first time
      //
      if (LcOglProteinAssetFactory::InitialiseProteinAssetFactory())
      {
         ret = true;
         m_pAssetLibraryManager = GetIAssetLibraryManager();
         m_pLibraryRequest = new LcFBXLibraryRequest(m_pAssetLibraryManager);
      }
   }

   // If return false here, mean we couldn't find loaded system asset library
   //
   // LiUAssertAny(ret);
   return ret;
}

LtNat32 
LcFBXSceneProcessor::GetCurrentSystemLibraryVersion()
{
   LtNat32 version = 0;
   LiUAssertAny(m_pAssetLibraryManager != NULL);

   // If a system asset library has been loaded it must be the current version (older versions are not suppported).
   if (m_pAssetLibraryManager != NULL)
   {
      WideString master_lib_id(LcOglProteinStringDefinitions::GetMasterLibId());
      const IAssetLibrary* system_library = m_pAssetLibraryManager->GetLibraryByIdentifier(master_lib_id);
      if (system_library != NULL)
         version = IAssetLibraryManager::kCurrentSystemLibraryVersion;
   }

   return version;
}

/// 
/// The enum from OGS translation code
/// 
enum 
{
   Light_Origin = 0,
   Light_Spot,
   Light_Directional
};

enum 
{
   Distribution_Isotropic,
   Distribution_Spot, 
   Distribution_Diffuse,
   Distribution_Web,
};

// Get the choice value
void 
get_prop_choice(LcOglProteinAsset* inst, const LtWChar* name, LtInt32& value)
{
   LcOglProteinProperty* prop = inst->FindProperty(name);
   if (prop == NULL)
      return;

   if (prop->GetType() == LcOglProteinProperty::eTYPE_CHOICELIST)
   {
      value = prop->GetChoiceList()->default_value;
   }
}

// Get integer property from asset instance
static void 
get_prop_int(LcOglProteinAsset* inst, const LtWChar* name, LtInt32& value)
{
   LcOglProteinProperty* prop = inst->FindProperty(name);
   if (prop == NULL)
      return;

   if (prop->GetType() == LcOglProteinProperty::eTYPE_INTEGER)
   {
      value = prop->GetInteger();
   }
}

// Get float property from asset instance
static void 
get_prop_float(LcOglProteinAsset* inst, const LtWChar* name, LtFloat& value)
{
   LcOglProteinProperty* prop = inst->FindProperty(name);
   if (prop == NULL)
      return;

   if (prop->GetType() == LcOglProteinProperty::eTYPE_SCALAR)
   {
      value = prop->GetScalar();
   }
}

// Get color property from asset instance
static void 
get_prop_color(LcOglProteinAsset* inst, const LtWChar* name, LtFloat value[])
{
   LcOglProteinProperty* prop = inst->FindProperty(name);
   if (prop == NULL)
      return;

   if (prop->GetType() == LcOglProteinProperty::eTYPE_COLOR_RGB )
   {
      const LtFloat* c = prop->GetColorRGB(0);
      memcpy(value, c, sizeof(LtFloat)*3);
      value[3] = 1;
   }
   else if (prop->GetType() == LcOglProteinProperty::eTYPE_COLOR_RGBA)
   {
      const LtFloat* c = prop->GetColorRGBA(0);
      memcpy(value, c, sizeof(LtFloat)*4);
   }
}

static void 
get_prop_bool(LcOglProteinAsset* inst, const LtWChar* name, bool& b)
{
   LcOglProteinProperty* prop = inst->FindProperty(name);
   if (prop == NULL)
      return;

   if (prop->GetType() == LcOglProteinProperty::eTYPE_BOOLEAN )
   {
      b = prop->GetBoolean();
   }
}

void 
LcFBXSceneProcessor::ProcessProteinLight(LcFBXScene* scene, LcOglProteinAsset* asset, FbxAMatrix& position_mat)
{
   // extract light color: diffuse, specular, ambient.
   LtFloat color[4];
   get_prop_color(asset, L"rgbLightColor", color);
   LtFloat specular_color[4];
   get_prop_color(asset, L"specularColor", specular_color);
   LtFloat ambient_color[4];
   get_prop_color(asset, L"ambientColor", ambient_color);
   // set light on/off, some protein lights in FBX file may marked as off.
   bool light_on = true;
   get_prop_bool(asset, L"on", light_on);

   // Calculate position
   FbxVector4 ly_negative_axis(-90.0, 0.0, 0.0);
   FbxAMatrix llight_rotation;
   llight_rotation.SetR(ly_negative_axis);
   FbxAMatrix llight_global_position = position_mat * llight_rotation;
   FbxVector4 tran = llight_global_position.GetT();
   // Calculate direction
   LcLVec3f dir(0,0,1);
   FbxQuaternion quaternion = llight_global_position.GetQ();
   LcLRotation3f quaternion3f(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
   LcLTransform3f rot3f(quaternion3f);
   LcLVec3f rot_dir = dir * rot3f;

   WideString idName(asset->GetDefinitionId());
   bool is_photometric = (idName == L"GenericPhotometricLight" || idName == L"SpotLight-001" || idName == L"PointLight-001");
   // Generic photometric light will be handled separately
   if (is_photometric)
   {
      int intensityUnits = 0;
      get_prop_int(asset, L"intensityUnits", intensityUnits);
      is_photometric = (intensityUnits != 4); // Intens_Unitless
   }    
   if (!is_photometric)
   {
      LtFloat intensity = 1;
      get_prop_float(asset, L"multiplier", intensity);
      for (LtInt32 i=0; i<4; i++)
         color[i] *= intensity;
   }

   LtInt32 distribute = 0;
   get_prop_choice(asset, L"distribution", distribute);
   // Distribute: 0 - isotropic,
   //             1 - spot,
   //             2 - diffuse,
   //             3 - web,
   bool photometric_spot = is_photometric && distribute == Distribution_Spot;
   bool photometric_web = is_photometric && distribute == Distribution_Web;
   bool photometric_point = is_photometric && (distribute == Distribution_Isotropic || distribute == Distribution_Diffuse);

   LtInt32 type = 0;
   get_prop_choice(asset, L"lightobjecttype", type);
   if (type == Light_Directional)
   {
      LcNwcDistantLight dist_light;

      dist_light.SetOn(light_on);
      dist_light.SetDiffuseColor(color[0], color[1], color[2]);
      dist_light.SetAmbientColor(ambient_color[0], ambient_color[1], ambient_color[2]);
      dist_light.SetSpecularColor(specular_color[0], specular_color[1], specular_color[2]);
      dist_light.SetEye(false);
      dist_light.SetDirection(rot_dir[0], rot_dir[1], rot_dir[2]);

      dist_light.SetAutodeskLight(reinterpret_cast<LtNwcAutodeskAsset>(asset));
      scene->GetNwcScene()->AddLight(dist_light);
   }
   else if (photometric_point || (!is_photometric && type == Light_Origin) )
   {
      LcNwcPointLight point_light;

      point_light.SetOn(light_on);
      point_light.SetDiffuseColor(color[0], color[1], color[2]);
      point_light.SetAmbientColor(ambient_color[0], ambient_color[1], ambient_color[2]);
      point_light.SetSpecularColor(specular_color[0], specular_color[1], specular_color[2]);
      point_light.SetEye(false);
      point_light.SetPosition(tran[0], tran[1], tran[2]);

      point_light.SetAutodeskLight(reinterpret_cast<LtNwcAutodeskAsset>(asset));
      scene->GetNwcScene()->AddLight(point_light);
   }
   else if (photometric_spot || photometric_web || (!is_photometric && type == Light_Spot))
   {
      LcNwcSpotLight spot_light;

      spot_light.SetOn(light_on);
      spot_light.SetDiffuseColor(color[0], color[1], color[2]);
      spot_light.SetAmbientColor(ambient_color[0], ambient_color[1], ambient_color[2]);
      spot_light.SetSpecularColor(specular_color[0], specular_color[1], specular_color[2]);

      LtFloat hot_spot_scale = 0;
      get_prop_float(asset, L"hotSpot", hot_spot_scale);
      spot_light.SetDropOffRate(hot_spot_scale);

      LtFloat cone_angle = 0;
      get_prop_float(asset, L"fallOff", cone_angle);
      // Protein light use full cone angle, and we store it as half of the angle.
      spot_light.SetCutOffAngle(cone_angle/2);

      LtInt32 decay_type = 0;
      get_prop_int(asset, L"decayType", decay_type);
      LtFloat attenuation[3] = {
         decay_type == FbxLight::eNone ? 1.0 : 0.0, 
         decay_type == FbxLight::eLinear ? 1.0 : 0.0, 
         decay_type == FbxLight::eQuadratic ? 1.0 : 0.0
      };
      spot_light.SetAttenuation(attenuation[0], attenuation[1], attenuation[2]);

      spot_light.SetEye(false);
      spot_light.SetPosition(tran[0], tran[1], tran[2]);
      spot_light.SetDirection(rot_dir[0], rot_dir[1], rot_dir[2]);

      spot_light.SetAutodeskLight(reinterpret_cast<LtNwcAutodeskAsset>(asset));
      scene->GetNwcScene()->AddLight(spot_light);
   }
}

bool LcFBXSceneProcessor::PostRead(LcFBXScene* scene, FbxScene* pFbxScene, bool* hasblob)
{
   LtNat32 highest_lib_version = LcOglProteinStringDefinitions::GetHighestReferenceBlobVersion();
   LtNat32 sys_lib_version = GetCurrentSystemLibraryVersion();
   if (sys_lib_version > highest_lib_version)
   {
      // Now protein library has upgraded to 4.0, but it the file version it
      // write to fbx file is still 3.0.
      sys_lib_version = highest_lib_version;
   }

   void* blobBegin = NULL;
   size_t blobSize = 0;
   const LtChar* references_blob = NULL;
   const LtChar* references_blob_property = NULL;
   for (LtNat32 i=sys_lib_version; i>=1; --i)
   {
      references_blob = LcOglProteinStringDefinitions::GetAdskBlobObjectName(i);
      references_blob_property = LcOglProteinStringDefinitions::GetAdskBlobPropertyName(i);
      if (references_blob == NULL || references_blob_property == NULL)
         continue;

      // Read binary blob.
      FbxObject* lassetReferences =  pFbxScene->FindSrcObject(references_blob);
      if (lassetReferences != NULL)
      {
         // Read binary blob property.
         FbxProperty lprop = lassetReferences->FindProperty(references_blob_property);
         if (lprop.IsValid())
         {
            FbxBlob binaryBlob = lprop.Get<FbxBlob>();
            blobSize = binaryBlob.Size();
            blobBegin = const_cast<void*>(binaryBlob.Access());
         }
         if (blobBegin != NULL)
            break;
      }
   }

   if (m_pAssetLibraryManager != NULL && blobBegin != NULL)
   {
      if(hasblob) *hasblob = true;

      // write blob to temp file. This will be loaded as an asset library and used to
      // resolve any embedded references. This will only work with a file, not an in memory stream.
      LcUWideStringBuffer ltempP;
      LcUSysFile::MakeTempFileNameW(L"", L"ADSKAssets_", L"adsklib", &ltempP);
      WideString ltempPath(ltempP.GetString());
      FileDataStream ldataStreamOut(ltempPath, L"wb+");
      bool bRet = ldataStreamOut.open();
      if (!bRet)
      {
         FileSystem::Remove(ltempPath);
         return false;
      }
      ldataStreamOut.write(blobBegin, blobSize);
      ldataStreamOut.close();

      // load temp file
      const IAssetLibrary* pTempLibrary = m_pAssetLibraryManager->LoadLibraryByURI(ltempPath);
      if(pTempLibrary == NULL)
      {
         FileSystem::Remove(ltempPath);
         return false;
      }

      LcFbxProteinFileResolver file_resolver(scene);
      LcOglProteinAssetEncoder* oglEncoder = LcOglProteinAssetFactory::CreateEncoder(const_cast<IAssetLibrary*>(pTempLibrary),
         &file_resolver);
      if(oglEncoder == NULL)
      {
         bool ret = m_pAssetLibraryManager->UnloadLibrary(*pTempLibrary);
         LiUAssertAny(ret);
         FileSystem::Remove(ltempPath);
         return false;
      }

      // get a deserializer
      IAssetDeSerializer* pAssetDeserializer = m_pAssetLibraryManager->CreateDeSerializer();

      // Create an in memory stream to deserialize the data. We can't deserialize from the temporary
      // file as underlying file access for deserialization and asset library management conflict.
      LcFBXMemReadStream ldataStreamIn(static_cast<BYTE*>(blobBegin), blobSize);
      ldataStreamIn.openForRead();

      bool bret = pAssetDeserializer->DeSerializeAssets(ldataStreamIn, *m_pLibraryRequest);
      LiUAssertAny(bret);

      bool add_env = false;
      int proxyCount = pFbxScene->GetSrcObjectCount();
      for (int i = 0; i < proxyCount; ++i)
      {
         FbxObject* lProxyObject = pFbxScene->GetSrcObject(i);
         if(!lProxyObject)
            continue;

         // Judge whether it's a proxy by looking for the proxy's unique property name.
         //
         FbxProperty lProxyIdProperty = lProxyObject->FindProperty(LcOglProteinStringDefinitions::GetAdskReferencesBlobId());
         if(!lProxyIdProperty.IsValid())
         {
            continue;
         }

         FbxString lProxyId = lProxyIdProperty.Get<FbxString>();

         // Extract the asset
         //
         UTF8String strAsset(lProxyId.Buffer());

         IAssetInstance* lResolvedInstance = pAssetDeserializer->ResolveAssetReference(strAsset);
         if(lResolvedInstance && !lResolvedInstance->IsUnresolved())
         {
            // Encoder will extract any embedded resources
            LcOglProteinAsset* OglAsset = oglEncoder->Encode(lResolvedInstance);

            // Get asset instance type
            UTF8String type;
            lResolvedInstance->GetAssetSchema()->GetAssetType(type);

            if (!type.compare(ADSK_ASSET_TYPE_LIGHTAPPEARANCE))
            {	
               // Add protein light here.
               OglAsset->Ref();
               m_asset_map.insert(stlport::pair<FbxString, LcOglProteinAsset*>(lProxyId, OglAsset));
            }
            else if (!type.compare(ADSK_ASSET_TYPE_ENVIRONMENT)) 
            {
               // If the asset instance is a environment, we can add it to scene here.
               if (!add_env)
               {
                  scene->GetNwcScene()->AddAutodeskEnvironment(reinterpret_cast<LtNwcAutodeskAsset>(OglAsset));
                  add_env = true;
               }
            }
            else 
            {
               LtFBXAutodeskMaterialMap::iterator it= m_resolvedMaterials.find(lProxyId);
               if( it==m_resolvedMaterials.end())
               {
                  // Else, directly convert it to Autodesk material,

                  // LtNwcAutodeskAsset is actually LcOglProteinAsset*
                  LtNwcAutodeskMaterial material = LiNwcAutodeskMaterialCreate();

                  LcUWideStringBuffer primary, secondary;
                  LcUResource::LocaleInfo(&primary, &secondary, LcUResource::LOCALE_INFO_TYPE_LOCALE_CODES);
                  WideString locale(primary.GetString());
                  WideString uiName;
                  lResolvedInstance->GetUIName(uiName, locale);
                  LiNwcAttributeSetName(material, uiName);
                  LiNwcAutodeskMaterialSetMaterialAsset(material, reinterpret_cast<LtNwcAutodeskAsset>(OglAsset));
                  m_resolvedMaterials.insert(stlport::pair<FbxString, LcNwcAutodeskMaterial>(lProxyId, LcNwcAutodeskMaterial(material)));
                  LiNwcAutodeskMaterialDestroy(material);
               }
            }
            lResolvedInstance->Destroy();

         }
      }

      // Deserializer must be destroyed before stream it's working on is closed
      pAssetDeserializer->Destroy();
      pAssetDeserializer = NULL;
      ldataStreamIn.closeForRead();

      bool ret = m_pAssetLibraryManager->UnloadLibrary(*pTempLibrary);
      LiUAssertAny(ret);

      FileSystem::Remove(ltempPath);
      if(FileSystem::Exists(ltempPath))
      {
         LiUAssertAny(false && ltempPath);
      }

      LcOglProteinAssetFactory::DestroyEncoder(oglEncoder);
   }
   return true;
}

LtNwcAutodeskMaterial LcFBXSceneProcessor::FindResolvedMaterial(const FbxString& assetID)
{
   LtFBXAutodeskMaterialMap::iterator iter = m_resolvedMaterials.find(assetID);
   if(iter!=m_resolvedMaterials.end())
   {
      return iter->second;
   }

   return NULL;
}

LcOglProteinAsset* LcFBXSceneProcessor::FindResolvedAsset(const FbxString& assetID)
{
   LtFBXAutodeskAssetMap::iterator it = m_asset_map.find(assetID);
   if (it!=m_asset_map.end())
   {
      return it->second;
   }

   return NULL;
}