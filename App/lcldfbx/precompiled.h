//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/precompiled.h#11 $
//   $Change: 85129 $
//   $Date: 2012/03/31 $
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


///
/// Some sample header files.
/// Review these and add/remove as required.
///
#include <lcutil/LcUString.h>
#include <lcutil/LcUSysFile.h>
#include <lcutil/LcUError.h>
#include <lcutil/LcUMemorySTLAlloc.h>

#include <lclinear/LcLVec.h>

#include <nwcreate/LiNwcAll.h>
///
/// very important to add those defines
/// before using fbxsdk.h and a link static with a .lib of the FBXSDK
///
#define KFBX_PLUGIN
#define KFBX_SDK
#define KFBX_NODLL

#pragma warning (push)
// disable warning '_InterlockedCompareExchange' : inconsistent dll linkage
#pragma warning(disable: 4273)
#include <memory>
#pragma warning (pop)

///
/// use the fbxsdk.h
///
#define FBXSDK_NEW_API
#pragma warning (push)
// disable warning C4996: 'stricmp': The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _stricmp. See online help for details.
#pragma warning(disable: 4996)
#include <fbxsdk.h>
#pragma warning (pop)