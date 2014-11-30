//
//   $Id: 
//   $Change:  
//   $Date:  
//
//   Description:
//      Library Request
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

#ifndef LCFBXLIBRARYREQUEST_HDR
#define LCFBXLIBRARYREQUEST_HDR

// Protein 2 includes
//
#pragma warning(push)
#pragma warning(disable: 4819 4189)

#include "adskassetapi_new/assetlibmgrutils.h"
#include "adskassetapi_new/ipropertytable.h"
#include "adskassetapi_new/iassetinstance.h"
#include "adskassetapi_new/assettypes.h"
#include "adskassetapi_new/ipathresolver.h"
#include "adskassetapi_new/iassetlibrary.h"
#include "adskassetapi_new/ilibraryrequest.h"
#include "adskassetapi_new/iassetlibrarymanager.h"
#include "core/io/FileSystem.h"
#include "core/io/FileDataStream.h"
#include "core/string/UTF8String.h"
#include "core/string/WideString.h"
#pragma warning(pop)

using namespace autodesk::platform::core::io;
using namespace autodesk::platform::core::string;
using namespace autodesk::platform::assets;

// *** Protein Interop ***
// ILibraryRequest is used when de-serializing assets, to request that libraries be loaded 
// (or returned if they are already loaded) in order to resolve dependencies. This interface 
// is to be implemented by the API client. Here provides a LcFBX implementation.
//
#include "stlport\list"

class LcFBXLibraryRequest : public ILibraryRequest
{
public:
    LcFBXLibraryRequest(IAssetLibraryManager* pmanager);
    virtual ~LcFBXLibraryRequest();
    virtual const IAssetLibrary* RequestLibrary(const LibraryInfo& pInfo);
    IAssetLibraryManager* assetLibManager() const;

private:

    const IAssetLibrary* _load(autodesk::platform::core::string::WideString path, autodesk::platform::core::string::WideString id);

private:    
    IAssetLibraryManager* mpManager;

    typedef stlport::list<const IAssetLibrary*>   tLibList;
    tLibList _oLibList;
};

#endif //LCFBXLIBRARYREQUEST_HDR