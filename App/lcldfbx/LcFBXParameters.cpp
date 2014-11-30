//
//    $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXParameters.cpp#10 $
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


#include "precompiled.h"
#include "LcFBXParameters.h"
#include <nwcreate\LiNwcData.h>
#include <lcutil/LcUError.h>

static const LtChar* f_faceting_factor = "faceting_factor";
static const LtChar* f_max_deviation = "max_facet_deviation";
static const LtChar* f_convert_skeleton = "convert_skeleton";
static const LtChar* f_convert_light = "convert_light";
static const LtChar* f_convert_autodesk_materials = "convert_autodesk_materials";

void
LcFBXParameters::DefineOptionSet(const LcNwcOptionSet& set)
{
   m_set = set;
   LcNwcData data;

   data.SetFloat(1.0);
   m_set.DefineOption(f_faceting_factor, data);

   data.SetLinearFloat(0.0);
   m_set.DefineOption(f_max_deviation, data);

   data.SetBoolean(true);
   m_set.DefineOption(f_convert_skeleton, data);

   data.SetBoolean(true);
   m_set.DefineOption(f_convert_light, data);
   
   data.SetBoolean(true);
   m_set.DefineOption(f_convert_autodesk_materials, data);

}

LtFloat
LcFBXParameters::FacetingFactor() const
{
   LcNwcData data;
   LtBoolean result = m_set.GetOption(f_faceting_factor, data);
   LiUAssertAny(result && "Expect to be able to get options correctly");
   return data.GetFloat();
}

LtFloat
LcFBXParameters::MaxFacetDeviation() const
{
   LcNwcData data;
   LtBoolean result = m_set.GetOption(f_max_deviation, data);
   LiUAssertAny(result && "Expect to be able to get options correctly");
   return data.GetLinearFloat();
}

LtBoolean 
LcFBXParameters::GetIsConvertSkeleton() const
{
   LcNwcData data;
   LtBoolean result = m_set.GetOption(f_convert_skeleton, data);
   LiUAssertAny(result && "Expect to be able to get options correctly");
   return data.GetBoolean();
}

LtBoolean 
LcFBXParameters::GetIsConvertLight() const
{
   LcNwcData data;
   LtBoolean result = m_set.GetOption(f_convert_light, data);
   LiUAssertAny(result && "Expect to be able to get options correctly");
   return data.GetBoolean();
}

LtBoolean 
LcFBXParameters::GetIsConvertAutodeskMaterials() const
{
   LcNwcData data;
   LtBoolean result = m_set.GetOption(f_convert_autodesk_materials, data);
   LiUAssertAny(result && "Expect to be able to get options correctly");
   return data.GetBoolean();
}
