//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LiFBXMisc.h#10 $
//   $Change: 107559 $
//   $Date: 2013/12/11 $
//
//   Description:
//      Fbx utility functions
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
#ifndef LIFBXMISC_HDR
#define LIFBXMISC_HDR

#include <stlport\hash_map>

/// 
/// Get one specific pose matrix
/// 
/// @param pose FBX pose
/// @param pnode_index node index
///
FbxAMatrix 
LiFBXGetPoseMatrix(FbxPose* pose, 
                   LtInt32 pnode_index);

/// 
/// Get global position
/// 
/// @param pnode FBX node
/// @param pose FBX pose
/// @param time animation time
/// @param parent_global_position parent global position
///
FbxAMatrix 
LiFBXGetGlobalPosition(FbxNode* pnode, 
                       FbxPose* pose, 
                       FbxTime* time, 
                       FbxAMatrix* parent_global_position=NULL);


/// 
/// Get geometry offset
/// 
/// @param pnode FBX node
///
FbxAMatrix
LiFBXGetGeometryOffset(FbxNode* pnode);


/// 
/// Add one matrix to another
/// 
/// @param pDstMatrix Destination matrix
/// @param pSrcMatrix source matrix
///
void 
LiFBXMatrixAdd(FbxAMatrix& pDstMatrix, 
                    FbxAMatrix& pSrcMatrix);


/// 
/// Get matrix scale
/// 
/// @param pMatrix matrix
/// @param pValue scale factor
///
void 
LiFBXMatrixScale(FbxAMatrix& pMatrix, 
                      LtFloat pValue);


/// 
/// add one matrix to one diagnoal
/// 
/// @param pMatrix matrix
/// @param pValue value
///
void 
LiFBXMatrixAddToDiagnoal(FbxAMatrix& pMatrix, 
                              LtFloat pValue);

/// 
/// Judge one specific  translation matrix is valid
/// 
/// @param matrix matrix
///
bool 
LiFBXIsValidTranMatirx(FbxAMatrix& matrix);


/// 
/// float equal operation
/// 
/// @param v value
/// @param u value
///
bool 
LiFBXFloatEqual(LtFloat v, LtFloat u);


/// 
/// Get vertex id
/// 
/// @param mesh FBX mesh
/// @param polygon_id polygon id
/// @param vertex_index vertex index
///
LtInt32 
LiFBXGetVertexId(FbxMesh* mesh, LtInt32 polygon_id, LtInt32 vertex_index);


/// 
/// Judge if one specific file exsits
/// 
/// @param file_mame file name
///
bool 
LiFBXFileExists(const LtChar* file_mame);

// Resolve texture files
// @param fbx_path, the importing fbx file path
// @param file_name, texture file name to be resolved
// @param resolved_file, return value
bool 
LiFBXResolveFile(const LtWChar* fbx_path, const LtChar* file_mame, LcUWideStringBuffer* resolved_file);


#endif //LIFBXMISC_HDR
