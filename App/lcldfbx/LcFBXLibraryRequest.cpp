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

#include <precompiled.h>

#include "LcFBXLibraryRequest.h"

LcFBXLibraryRequest::LcFBXLibraryRequest(IAssetLibraryManager* pmanager)
{
    mpManager = pmanager;
}

LcFBXLibraryRequest::~LcFBXLibraryRequest()
{
    if (!_oLibList.empty() && mpManager)
    {
        tLibList::iterator oIter = _oLibList.begin();

        for ( ; oIter != _oLibList.end(); ++oIter )
        {
            const IAssetLibrary* pLibrary = *oIter;
            if (pLibrary != NULL)
            {
                mpManager->UnloadLibrary(*pLibrary);
            }
        }
        _oLibList.clear();
    }
}

IAssetLibraryManager* LcFBXLibraryRequest::assetLibManager() const
{
    return mpManager;
}

const IAssetLibrary* LcFBXLibraryRequest::RequestLibrary(const LibraryInfo& info)
{
    if (!info.mUri.value() || !assetLibManager())
    {
        return NULL;
    }

    UTF8String file = info.mUri;
    WideString uuid = info.mUUID;

    WideString fileName(file);

    if (fileName.isEmpty() || !assetLibManager())
    {
        return NULL;
    }

    //
    // Check to see if a library with the proper id is already loaded
    //
    const IAssetLibrary* pLib = assetLibManager()->GetLibraryByIdentifier(info.mUUID);
    if (pLib)
    {
        return pLib;
    }

    // Next try loading the name passed in in case it's a full path.
    //
    pLib = _load(fileName, info.mUUID);
    if (pLib)
    {
        return pLib;
    }

    // Ok, so we couldn't load it as passed in, so let's strip off any leading path and then try to
    // find the filename.
    //
    const wchar_t* fname = wcsrchr(fileName.string(), L'\\');
    if (fname)
    {
        fname++; // increment to the next char after the '\'
    }
    else 
    {
        // there's no path, only a filename
        //
        fname = fileName.string();
    }


    // First see if Protein has the lib.  If that fails, then try acad's findFile.
    //
    size_t cnt = assetLibManager()->GetNumberOfSystemLibraryPaths();
    WideString libPath;
    for (size_t i = 0; i < cnt; i++) 
    {
        assetLibManager()->GetSystemLibraryPath(i, libPath);
        if (LcUSysString::WideStrstr(libPath, fname)) 
        {
            // verify that this file actually exists.  We do this because
            // it is possible for the registry where these paths come from
            // to contain invalid paths (I had it happen on my machine)
            // if the Protein files are manually deleted rather than being
            // uninstalled.   Artc  9/3/09
            //

            if (FileSystem::Exists(libPath.string()))
            {
                pLib = _load(libPath, info.mUUID);
                if (pLib)
                {
                    return pLib;
                }
            }
        }
        //
        // try appending the file name to the path portion of the system library path. This may catch some user libraries
        //
        off_t nOff = libPath.rfind(L'\\');
        if (nOff != -1)
        {
            libPath = libPath.substr(0, nOff + 1);
            libPath = libPath.concat(WideString(fname));

            if (FileSystem::Exists(libPath.string()))
            {
                pLib = _load(libPath, info.mUUID);
                if (pLib)
                {
                    return pLib;
                }
            }
        }
    }

    return pLib;
}


const IAssetLibrary* LcFBXLibraryRequest::_load(WideString path, WideString id)
{
    //
    // attempt to load the file and, if successful, make sure the id mathes the one we're looking for
    //
    const IAssetLibrary* pLib = assetLibManager()->LoadLibraryByURI(path);
    if (pLib)
    {
        WideString zId;
        pLib->GetIdentifier(zId);
        if (zId == id)
        {
            _oLibList.push_back(pLib);
            return pLib;
        }
        else
        {
            //
            // This is really more of a remove reference call
            //
            assetLibManager()->UnloadLibrary(*pLib);
        }
    }

    return NULL;
}

