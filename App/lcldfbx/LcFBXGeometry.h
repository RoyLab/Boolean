//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXGeometry.h#22 $
//   $Change: 107559 $
//   $Date: 2013/12/11 $
//
//   Description:
//      Fbx Geometry - this class is used to draw mesh, nurbs curve, nurbs surface and Skeleton,
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
#ifndef LCFBXGEOMETRY_HDR
#define LCFBXGEOMETRY_HDR

#pragma once
#include "LcFBXScene.h"
#include <stlport\hash_map>
#include <map>

typedef stlport::vector<LtInt32, LcUMemorySTLAllocSizeOf<LtInt32>  > LtFBXListOfPolygonIndices;

///
/// material map element, it is used to store one specific polygon with its material and texture.
///
struct MaterialMapData
{
   MaterialMapData(): m_nwc_material(NULL), m_adsk_material(NULL), m_uv_swap(false), m_mapping_type(FbxTexture::eUMT_UV), m_is_mtl_from_blob(false) { m_polygon_list.clear(); }
   LtFBXListOfPolygonIndices m_polygon_list;
   LcNwcMaterial             m_nwc_material;
   LcNwcAutodeskMaterial     m_adsk_material;   
   bool                      m_uv_swap;
   FbxTexture::EUnifiedMappingType m_mapping_type;
   bool                      m_is_mtl_from_blob; // the original materal is protein.
};

///
/// store polygon sets, one mesh may have different textures, each texture map some polygons.
/// therefore, split mesh into some poltygon sets that each set map corresponding texture.
///
typedef std::map<FbxUInt64, MaterialMapData> MaterialMap;

///
/// This class is used to create UV parameters.
/// Create UV parameters when the texture mapping type is xy, xz, yz mapping.
/// In fbx files, when texture is the above three mapping type, their corresponding UV parameters are not stored in the fbx files.
/// Therefore, UV parameters should be create by ourselves.
///
class LcFBXUVParametersCreator
{
public:
   /// 
   /// Constructor
   /// 
   /// @param pvertex mesh vertex array
   /// @param count vertex array size
   ///
   LcFBXUVParametersCreator(FbxVector4* pvertex, LtInt32 count);

   /// 
   /// Get U parameter
   /// 
   /// @param u u parameter
   /// @param index mesh vertex index
   /// @param mapping_type texture mapping type
   ///
   bool GetU(LtFloat& u, LtInt32 index, FbxTexture::EUnifiedMappingType mapping_type) const;

   /// 
   /// Get v parameter
   /// 
   /// @param v v parameter
   /// @param index mesh vertex index
   /// @param mapping_type texture mapping type
   ///
   bool GetV(LtFloat& v, LtInt32 index, FbxTexture::EUnifiedMappingType mapping_type) const;

private:
   FbxVector4* m_mesh_vertexs;
   LtInt32 m_vertex_count;
   LtFloat m_x_min;
   LtFloat m_x_max;
   LtFloat m_y_min;
   LtFloat m_y_max;
   LtFloat m_z_min;
   LtFloat m_z_max;
};


typedef stlport::vector<FbxLayer*, LcUMemorySTLAllocSizeOf<FbxLayer*>  > LtFBXListOfLayerPtr;
typedef stlport::vector<LtInt32, LcUMemorySTLAllocSizeOf<LtInt32>  > LtFBXListOfPolygonSizes;

///
/// FBX Mesh information class is used to cache all the mesh information. 
/// It will improve the speed of drawing mesh.
///
class LcFBXMeshInfo
{
public:

   /// 
   /// Constructor
   /// 
   /// @param mesh FBX mesh
   ///
   LcFBXMeshInfo(FbxMesh* mesh);

   /// 
   /// Destructor
   ///
   ~LcFBXMeshInfo();

   /// 
   /// Tell whether the mesh info is valid
   /// 
   bool IsValid() const;

   /// 
   /// Get polygon count
   ///
   LtInt32 GetPolygonCount() const;

   /// 
   /// Get polygon size
   /// 
   /// @param i polygon index
   ///
   LtInt32 GetPolygonSize(LtInt32 i) const;

   /// 
   /// Get control points count
   ///
   LtInt32 GetControlPointsCount() const;

   /// 
   /// Get layer count
   ///
   LtInt32 GetLayerCount() const;

   /// 
   /// Get layer
   /// 
   /// @param pindex layer index
   ///
   FbxLayer* GetLayer(LtInt32 pindex) const;

   /// 
   /// Get vertex id 
   /// 
   /// @param polygon_index polygon index
   /// @param vertex_index vertex index
   ///
   LtInt32 GetVertexId(LtInt32 polygon_index, LtInt32 vertex_index) const;

private:

   FbxMesh* m_mesh;
   LtInt32 m_polygon_count;
   LtInt32 m_control_points_count;
   LtFBXListOfPolygonSizes m_polygon_sizes;
   LtInt32* m_cached_vertex_sum;
   LtInt32 m_layer_count;
   LtFBXListOfLayerPtr m_layers;
   bool m_valid;

};

class LcFBXScene;
///
/// FBX geometry class is used to draw mesh, nurbs curve, nurbs surface and Skeleton.
///
class LcFBXGeometry
{
public:

   /// 
   /// Constructor
   /// 
   /// @param manager FBXSDK manager
   /// @param type attribure type
   ///
   LcFBXGeometry(FbxManager* manager, LcFBXScene* scene, 
      FbxNodeAttribute::EType type = FbxNodeAttribute::eMesh);

   /// 
   /// Destructor
   ///
   ~LcFBXGeometry();


   /// 
   /// Draw geometry
   /// 
   /// @param node FBX node
   /// @param nwc_group nwc group
   /// @param global_position global position
   /// @param parent_global_position parent global position
   /// @param fbx_scene FBX scene
   /// @param name geometry's name
   /// @param time preserved for animation
   /// @param pose preserved for animation
   /// 
   LcNwcNode Draw(FbxNode* node,
      FbxAMatrix& global_position, 
      FbxAMatrix& parent_global_position,
      LcFBXScene& fbx_scene,
      const wchar_t* name,
      FbxTime* time = NULL, 
      FbxPose* pose = NULL);

protected:

   /// 
   /// Draw mesh
   /// 
   /// @param node FBX node
   /// @param nwc_group nwc group
   /// @param mesh FBX mesh
   /// @param global_pos global position
   /// @param fbx_scene FBX scene
   /// @param name geometry's name
   /// @param time preserved for animation
   /// @param pose preserved for animation
   /// 
   LcNwcNode DrawMesh(FbxNode* node,
      FbxMesh* mesh, 
      FbxAMatrix& global_pos, 
      LcFBXScene& fbx_scene,
      const wchar_t* name,
      FbxTime* time = NULL, 
      FbxPose* pose = NULL);

   /// 
   /// Draw Skeleton
   /// 
   /// @param node FBX node
   /// @param parent_global_pos parent global position
   /// @param global_pos global position
   /// 
   LcNwcGeometry DrawSkeleton(FbxNode*node, 
      FbxAMatrix& parent_global_pos, 
      FbxAMatrix& global_pos);

   /// 
   /// Draw Nurbs curve
   /// 
   /// @param nurbs_curve FBX Nurbs curve
   /// @param global_pos global position
   /// @param time preserved for animation
   /// @param pose preserved for animation
   /// 
   LcNwcGeometry DrawNurbsCurve(FbxNurbsCurve* nurbs_curve, 
      FbxAMatrix& global_pos, 
      FbxTime* time = NULL, 
      FbxPose* pose = NULL);

   /// 
   /// Draw Nurbs Surface
   /// 
   /// @param nurbs FBX Nurbs curve
   /// @param global_pos global position
   /// @param is_trimmed flag is used to explain whether this nurbs surface is trimed nurbs surface.
   /// @param time preserved for animation
   /// @param pose preserved for animation
   ///
   LcNwcGeometry DrawNurbsSurface(FbxGeometry* nurbs, 
      FbxAMatrix& global_pos, 
      LtBoolean is_trimmed = FALSE,
      FbxTime* time = NULL, 
      FbxPose* pose = NULL);

private:
   /// 
   /// Draw nwc curve
   /// 
   /// @param nurbs_curve FBX nurbs curve
   /// @param vertex vertex point
   ///
   LcNwcCurve DrawNwcCurve(FbxNurbsCurve* nurbs_curve, 
      LtPoint vertex);

   /// 
   /// Compute shape deformation
   /// 
   /// @param mesh FBX mesh
   /// @param time FBX animation time
   /// @param vertex_array FBX mesh vertex array
   ///
   void ComputeShapeDeformation(FbxMesh* mesh, 
      FbxTime* time, 
      FbxVector4* vertex_array);

   /// 
   /// Compute cluster deformation
   /// 
   /// @param node FBX node
   /// @param mesh FBX mesh
   /// @param global_pos global position
   /// @param vectex_array FBX mesh vertex array
   /// @param time preserved for animation
   /// @param pose preserved for animation
   ///
   void ComputeClusterDeformation(FbxNode* node,
      FbxMesh* mesh, 
      FbxAMatrix& global_pos, 
      FbxVector4* vectex_array, 
      FbxTime* time, 
      FbxPose* pose);

   /// 
   /// Read vertex cache data
   /// 
   /// @param pMesh FBX mesh
   /// @param pTime preserved for animation
   /// @param pVertexArray FBX mesh vertex array
   ///
   void ReadVertexCacheData(FbxMesh* pMesh, 
      FbxTime* pTime, 
      FbxVector4* pVertexArray);

   /// 
   /// Get vectex properties
   /// 
   /// @param mesh FBX mesh
   /// @param mapping_type texture mapping type
   ///
   LtBitfield GetVectexProperties(FbxGeometry* mesh, FbxTexture::EUnifiedMappingType mapping_type);

   /// 
   /// Get vectex property
   /// 
   /// @param element FBX layer element
   /// @param flag nwc vertex property
   ///
   LtBitfield GetVectexProperty(FbxLayerElement* element, LtBitfield flag);

   /// 
   /// draw specific polygon sets
   /// 
   /// @param geometry FBX geometry
   /// @param mesh FBX mesh
   /// @param pvertex FBX mesh vertex array
   /// @param polygons_list polygon set
   /// @param vertex_properties nwc vertex properties
   /// @param global_pos global position
   /// @param swap_uv UV swap flag
   /// @param uvcreator UV parameters creator for xy, xz. yz texture mapping type
   /// @param mapping_type texture mapping type
   /// @param adjust_uv if the original material is protein, the uv doesn't need to adjust
   ///
   bool DrawPolygons(LcFBXScene& fbx_scene,
      LcNwcGeometry& geometry,
      FbxMesh* mesh,
      const LcFBXMeshInfo& mesh_info,
      FbxVector4* pvertex,
      LtFBXListOfPolygonIndices& polygons_list,
      LtBitfield vertex_properties,
      FbxAMatrix& global_pos,
      bool swap_uv,
      LcFBXUVParametersCreator& uvcreator,
      FbxTexture::EUnifiedMappingType mapping_type,
      bool adjust_uv);

   /// 
   /// Get remaining polygons after process all polygon sets that are mapped by corresponding texture
   /// 
   /// @param mesh FBX mesh
   /// @param material_map material map
   /// @param remaining_polgons remaining polygons set
   ///
   void GetRemainingPolygons(FbxMesh* mesh,
      const LcFBXMeshInfo& mesh_info,
      MaterialMap& material_map,
      LtFBXListOfPolygonIndices& remaining_polgons);

   /// 
   /// Judge if specific layer element is sense mode
   /// 
   /// @param lelement FBX layer element
   ///
   bool HaveSenseMode(FbxLayerElement* lelement);

   /// 
   /// Get vertex Color
   /// 
   /// @param mesh FBX mesh
   /// @param plygon_index polygon id
   /// @param vertex_index polygon vertex id
   /// @param vertexid mesh vertex id
   /// @param color FBX color
   ///
   bool GetVertexColor(FbxMesh* mesh,
      const LcFBXMeshInfo& mesh_info,
      LtInt32 plygon_index,
      LtInt32 vertex_index,
      LtInt32 vertexid,
      LtInt32 control_point_index,
      FbxColor& color);

   /// 
   /// Get Vertex UV parameters
   /// 
   /// @param mesh FBX mesh
   /// @param plygon_index polygon id
   /// @param vertex_index polygon vertex id
   /// @param vertexid mesh vertex id
   /// @param uv FBX UV parameters
   ///
   bool GetVertexUV(FbxMesh*mesh, 
      const LcFBXMeshInfo& mesh_info,
      LtInt32 plygon_index,
      LtInt32 vertex_index,
      LtInt32 vertexid,
      LtInt32 control_point_index,
      FbxVector2& uv);

   /// 
   /// Get Vertex Normal
   /// 
   /// @param mesh FBX mesh
   /// @param plygon_index polygon id
   /// @param vertex_index polygon vertex id
   /// @param vertexid mesh vertex id
   /// @param normal FBX Normal
   ///
   bool GetVertexNormal(FbxMesh* mesh,
      const LcFBXMeshInfo& mesh_info,
      LtInt32 plygon_index,
      LtInt32 vertex_index,
      LtInt32 vertexid,
      LtInt32 control_point_index,
      FbxVector4& normal);

   /// 
   /// Test if the input data is valid
   /// 
   /// @param mesh FBX mesh
   /// @param plygon_index polygon id
   /// @param vertex_index polygon vertex id
   /// @param vertexid mesh vertex id
   ///
   bool IsValidData(FbxMesh* mesh,
      const LcFBXMeshInfo& mesh_info,
      LtInt32 plygon_index,
      LtInt32 vertex_index,
      LtInt32 vertexid,
      LtInt32 control_point_index);

   /// 
   /// set material map data
   /// 
   /// @param node FBX node
   /// @param fbx_scene FBX scene
   /// @param mat_id FBX surface material id
   /// @param material_map material map
   /// @param polygon_id polygon id
   ///
   void SetMapData(FbxNode* node,
      LcFBXScene& fbx_scene,
      int mat_id,
      MaterialMap& material_map,
      LtInt32 polygon_id);


private:
   FbxNodeAttribute::EType m_type;

   FbxManager* m_manager;
   LcFBXScene* m_fbx_scene;
};

#endif //LCFBXGEOMETRY_HDR
