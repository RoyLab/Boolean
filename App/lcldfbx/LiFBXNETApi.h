//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LiFBXNETApi.h#3 $
//   $Change: 107559 $
//   $Date: 2013/12/11 $
//
//   Description:
//      Fbx .NET functions
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

#ifndef LIFBXNETAPI_HDR
#define LIFBXNETAPI_HDR

#include <lcutil/LcUString.h>

/// 
/// convert one specific image into jpg format image
/// 
/// @param tiff image file name
///
bool 
LiFBXConvert2JPG(LcUWideStringBuffer& tiff);


// @Param op_mask, caller have to delete the output file.

// get the opacity mask (grayscale) from the texture map which may have alpha channel
// FBX can use the alpha channel of the texture map to set the transparency of the model,
// but Protein can't do this, it only can use Black/White as alpha source
// this method will extract the alpha channel from the source file, and export the alpha channel to grayscale
bool 
LiFBXTryGetOpacityMask(const LtWChar* src_file, LcUWideString* op_mask);

#endif //LIFBXNETAPI_HDR
