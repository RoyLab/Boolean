//
//    $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXParameters.h#10 $
//    $Change: 107559 $
//    $Date: 2013/12/11 $
//
//    Description:
//
//       FBX parameters class
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

#ifndef LCFBXPARAMETERS_HDR
#define LCFBXPARAMETERS_HDR
#pragma once

#include <nwcreate\LiNwcTypes.h>
#include <nwcreate\LiNwcOptionSet.h>

///
/// FBX parameters class
///
class LcFBXParameters
{
public:

   /// 
   /// Called to define the loader's option set
   /// 
   /// @param set nwc option set
   /// 
   void DefineOptionSet(const LcNwcOptionSet& set);

   /// 
   /// Get faceting factor
   /// 
   LtFloat FacetingFactor() const;

   /// 
   /// Get max facet deviation
   /// 
   LtFloat MaxFacetDeviation() const;   

   /// 
   /// Judge if Skeleton will be converted
   /// 
   LtBoolean GetIsConvertSkeleton() const;

   /// 
   /// Judge if light will be converted
   /// 
   LtBoolean GetIsConvertLight() const;

   /// 
   /// Judge if autodesk materials will be converted
   /// 
   LtBoolean GetIsConvertAutodeskMaterials() const;

private:
   LcNwcOptionSet m_set;
};

#endif // LCFBXPARAMETERS_HDR