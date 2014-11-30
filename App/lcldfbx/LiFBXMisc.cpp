//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LiFBXMisc.cpp#11 $
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

#include "precompiled.h"
#include "LiFBXMisc.h"
#include "WinBase.h"

bool 
LiFBXFileExists(const LtChar* file_mame)   
{  
   if (!file_mame)
   {
      return false;
   }
   WIN32_FIND_DATAA find_file_data;
   HANDLE hfind;  
   hfind = FindFirstFileA(file_mame, &find_file_data);

   if (hfind == INVALID_HANDLE_VALUE) 
   {
      return false;
   } 
   else 
   {

      FindClose(hfind);
      if ( find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         return false;
      }
      else
      {
         return true;
      }      
   }
}   

bool
LiFBXResolveFile(const LtWChar* fbx_path, const LtChar* file_name, LcUWideStringBuffer* resolved_file)
{
   if(!resolved_file)
      return false;

   bool ret = false;

   ret = LiFBXFileExists(file_name);
   if(ret)
   {
      if(LcUString::UTF8ToWideString(file_name, resolved_file)==TRUE && !resolved_file->IsEmpty())
         return true;
      else
      {
         resolved_file->SetEmptyString();
         return false;
      }
   }

   LcUWideStringBuffer buffer;
   LcUString::UTF8ToWideString(file_name, &buffer);
   LcUWideStringBuffer cur_dir;
   LcUSysFile::PopPathW(fbx_path, &cur_dir);
   LcUSysFile::SearchPathW(cur_dir.GetString(), buffer.GetString(),resolved_file);

   LcUSysFileInfoHandle* info = LcUSysFile::OpenFileInfoW(resolved_file->GetString());
   if(info)
   {
      bool is_dir = info->IsDirectory();
      LcUSysFile::CloseFileInfo(info);

      if(!is_dir)
         return true;
      else
      {
         resolved_file->SetEmptyString();
         return false;
      }
   }

   resolved_file->SetEmptyString();
   return false;
}

FbxAMatrix 
LiFBXGetPoseMatrix(FbxPose* pose, LtInt32 pnode_index)
{
   FbxAMatrix pose_matrix;
   LiUAssertAny(pose&&pnode_index>=0);
   if (!pose || pnode_index<0 )
   {
      return pose_matrix;
   }
   FbxMatrix matrix = pose->GetMatrix(pnode_index);

   memcpy((LtFloat*)pose_matrix, (LtFloat*)matrix, sizeof(matrix.mData));

   return pose_matrix;
}

FbxAMatrix 
LiFBXGetGlobalPosition(FbxNode* pnode, 
                       FbxPose* pose, 
                       FbxTime* time, 
                       FbxAMatrix* parent_global_position)
{
   FbxAMatrix lglobal_position;
   bool        lposition_found = false;
   if (pose)
   {
      LtInt32 lnode_index = pose->Find(pnode);

      if (lnode_index > -1)
      {
         // The bind pose is always a global matrix.
         // If we have a rest pose, we need to check if it is
         // stored in global or local space.
         if (pose->IsBindPose() || !pose->IsLocalMatrix(lnode_index))
         {
            lglobal_position = LiFBXGetPoseMatrix(pose, lnode_index);
         }
         else
         {
            // We have a local matrix, we need to convert it to
            // a global space matrix.
            FbxAMatrix lparent_global_position;

            if (parent_global_position)
            {
               lparent_global_position = *parent_global_position;
            }
            else
            {

               if (pnode->GetParent())
               {
                  lparent_global_position = LiFBXGetGlobalPosition(pnode->GetParent(), pose, time);
               }
            }

            FbxAMatrix lLocalPosition = LiFBXGetPoseMatrix(pose, lnode_index);
            lglobal_position = lparent_global_position * lLocalPosition;
         }

         lposition_found = true;
      }
   }

   if (!lposition_found)
   {
      lglobal_position = pnode->GetScene()->GetEvaluator()->GetNodeGlobalTransform(pnode, *time);
   }

   return lglobal_position;

}

FbxAMatrix 
LiFBXGetGeometryOffset(FbxNode* pnode) 
{
   LiUAssertAny(pnode);
   FbxVector4 lt, lr, ls;
   FbxAMatrix lgeometry;

   if (pnode)
   {
      lt = pnode->GetGeometricTranslation(FbxNode::eSourcePivot);
      lr = ( pnode->GetGeometricRotation(FbxNode::eSourcePivot) * LI_PI)/180.0;
      ls = pnode->GetGeometricScaling(FbxNode::eSourcePivot);

      lgeometry.SetT(lt);
      lgeometry.SetR(lr);
      lgeometry.SetS(ls);
   }

   return lgeometry;
}

void 
LiFBXMatrixAdd(FbxAMatrix& pDstMatrix, 
               FbxAMatrix& pSrcMatrix)
{
   LtInt32 i,j;

   for (i = 0; i < 4; i++)
   {
      for (j = 0; j < 4; j++)
      {
         pDstMatrix[i][j] += pSrcMatrix[i][j];
      }
   }
}

void 
LiFBXMatrixScale(FbxAMatrix& pMatrix, 
                 LtFloat pValue)
{
   LtInt32 i,j;

   for (i = 0; i < 4; i++)
   {
      for (j = 0; j < 4; j++)
      {
         pMatrix[i][j] *= pValue;
      }
   }
}

void 
LiFBXMatrixAddToDiagnoal(FbxAMatrix& pMatrix, 
                         LtFloat pValue)
{
   pMatrix[0][0] += pValue;
   pMatrix[1][1] += pValue;
   pMatrix[2][2] += pValue;
   pMatrix[3][3] += pValue;
}

bool 
LiFBXIsValidTranMatirx(FbxAMatrix& matrix)
{
   FbxVector4 vect(0, 0, 0, 0);
   if (matrix.GetRow(0) == vect || 
      matrix.GetRow(1) == vect || 
      matrix.GetRow(2) == vect)
   {
      return false;
   }
   return true;
}

bool 
LiFBXFloatEqual(LtFloat v, LtFloat u)
{
   LtFloat tol = 1e-6;
   if ( fabs(v-u)<tol )
   {
      return true;
   }
   return false;
}


LtInt32 
LiFBXGetVertexId(FbxMesh* mesh, LtInt32 polygon_id, LtInt32 vertex_index)
{ 
   if (!mesh || polygon_id<0 || vertex_index<0)
   {
      return -1;
   }

   if ( polygon_id>=mesh->GetPolygonCount() )
   {
      return -1;
   }

   if ( vertex_index>=mesh->GetPolygonSize(polygon_id) )
   {
      return -1;
   }

   LtInt32 vertex_id =0;
   for (LtInt32 i =0; i<polygon_id; i++)
   {
      vertex_id += mesh->GetPolygonSize(i);
   }

   vertex_id += vertex_index;

   return vertex_id;
}
