//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LcFBXGeometry.cpp#30 $
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
#include "precompiled.h"
#include "LcFBXGeometry.h"
#include "LcFBXMaterial.h"
#include "LcFBXScene.h"
#include "LiFBXMisc.h"
#include <stlport\hash_map>
#include <stlport\vector>
#include "LcFBXAdskMtlUtil.h"

LcFBXUVParametersCreator::LcFBXUVParametersCreator(FbxVector4* pvertex, LtInt32 count)
: m_mesh_vertexs(NULL)
, m_vertex_count(0)
, m_x_min(0)
, m_x_max(0)
, m_y_min(0)
, m_y_max(0)
, m_z_min(0)
, m_z_max(0)
{
   if (!pvertex || count<=0)
   {
      return;
   }

   m_mesh_vertexs =pvertex;
   m_vertex_count = count;

   m_x_min = m_mesh_vertexs[0][0];
   m_x_max = m_mesh_vertexs[0][0];
   m_y_min = m_mesh_vertexs[0][1];
   m_y_max = m_mesh_vertexs[0][1];
   m_z_min = m_mesh_vertexs[0][2];
   m_z_max = m_mesh_vertexs[0][2];
   for (LtInt32 i =1; i<m_vertex_count;  i++)
   {
      if (m_mesh_vertexs[i][0]<m_x_min)
      {
         m_x_min = m_mesh_vertexs[i][0];
      }
      if (m_mesh_vertexs[i][0]>m_x_max)
      {
         m_x_max = m_mesh_vertexs[i][0];
      }

      if (m_mesh_vertexs[i][1]<m_y_min)
      {
         m_y_min = m_mesh_vertexs[i][1];
      }
      if (m_mesh_vertexs[i][1]>m_y_max)
      {
         m_y_max = m_mesh_vertexs[i][1];
      }

      if (m_mesh_vertexs[i][2]<m_z_min)
      {
         m_z_min = m_mesh_vertexs[i][2];
      }
      if (m_mesh_vertexs[i][2]>m_z_max)
      {
         m_z_max = m_mesh_vertexs[i][2];
      }
   }

}

bool 
LcFBXUVParametersCreator::GetU(LtFloat& u, LtInt32 index, FbxTexture::EUnifiedMappingType mapping_type) const
{
   if( index<0 || index>=m_vertex_count )
   {
      return false;
   }

   if (mapping_type==FbxTexture::eUMT_XY || mapping_type==FbxTexture::eUMT_XZ)
   {
      if (LiFBXFloatEqual(m_x_max-m_x_min, 0))
      {
         return false;
      }

      u = (m_mesh_vertexs[index][0]-m_x_min)/(m_x_max-m_x_min);

      return true;
   }
   else if (mapping_type==FbxTexture::eUMT_YZ)
   {
      if (LiFBXFloatEqual(m_y_max-m_y_min, 0))
      {
         return false;
      }

      u = (m_mesh_vertexs[index][1]-m_y_min)/(m_y_max-m_y_min);

      return true;
   }

   return false;
}

bool 
LcFBXUVParametersCreator::GetV(LtFloat& v, LtInt32 index, FbxTexture::EUnifiedMappingType mapping_type) const
{
   if( index<0 || index>=m_vertex_count )
   {
      return false;
   }

   if (mapping_type==FbxTexture::eUMT_YZ || mapping_type==FbxTexture::eUMT_XZ)
   {
      if (LiFBXFloatEqual(m_z_max-m_z_min, 0))
      {
         return false;
      }

      v = (m_mesh_vertexs[index][2]-m_z_min)/(m_z_max-m_z_min);

      return true;
   }
   else if (mapping_type==FbxTexture::eUMT_XY)
   {
      if (LiFBXFloatEqual(m_y_max-m_y_min, 0))
      {
         return false;
      }

      v = (m_mesh_vertexs[index][1]-m_y_min)/(m_y_max-m_y_min);

      return true;
   }

   return false;
}



LcFBXMeshInfo::LcFBXMeshInfo(FbxMesh* mesh)
: m_mesh(mesh)
, m_polygon_count(0)
, m_control_points_count(0)
, m_layer_count(0)
, m_cached_vertex_sum(NULL)
, m_valid(false)
{
   if (!m_mesh)
   {
      return;
   }

   LtInt32 pcount = m_mesh->GetPolygonCount();
   if (pcount<=0)
   {
      return;
   }
   m_polygon_count = pcount;

   LtInt32 ccount = m_mesh->GetControlPointsCount();
   if (ccount<=0)
   {
      return;
   }
   m_control_points_count = ccount;


   LtInt32 vertex_id =-1;
   m_cached_vertex_sum = new LtInt32[m_polygon_count];

   if (!m_cached_vertex_sum)
   {
      return;
   }

   LtInt32 pre_polygon_size =0;
   for (LtInt32 polygon_index =0; polygon_index<m_polygon_count; polygon_index++)
   {
      LtInt32 polygon_size = m_mesh->GetPolygonSize(polygon_index);
      if (polygon_size<=0)
      {
         return;
      }

      m_polygon_sizes.push_back(polygon_size);
      if (0==polygon_index)
      {
         m_cached_vertex_sum[polygon_index] = 0;
      }
      else
      {
         m_cached_vertex_sum[polygon_index] = m_cached_vertex_sum[polygon_index-1]+pre_polygon_size;
      }

      pre_polygon_size = polygon_size;

   }

   m_layer_count = m_mesh->GetLayerCount();
   for (LtInt32 layer_index =0; layer_index<m_layer_count; layer_index++)
   {
      FbxLayer* layer = m_mesh->GetLayer(layer_index);
      m_layers.push_back(layer);
   }

   m_valid = true;

}

LcFBXMeshInfo::~LcFBXMeshInfo()
{
   if (m_cached_vertex_sum)
   {
      delete []m_cached_vertex_sum;
      m_cached_vertex_sum = NULL;
   }
}

bool 
LcFBXMeshInfo::IsValid() const 
{
   return m_valid;
} 

LtInt32 
LcFBXMeshInfo::GetPolygonCount() const
{
   if (IsValid())
   {
      return m_polygon_count;
   }
   else
   {
      return -1;
   }
}

LtInt32 
LcFBXMeshInfo::GetPolygonSize(LtInt32 i) const
{
   if (IsValid())
   {
      if ( i>=0 && i<GetPolygonCount() )
      {
         return m_polygon_sizes[i];
      }
      else
      {
         return -1;
      }
   }
   else
   {
      return -1;
   }
}

LtInt32 
LcFBXMeshInfo::GetControlPointsCount() const
{
   return m_control_points_count;
}

LtInt32 
LcFBXMeshInfo::GetLayerCount() const
{
   return m_layer_count;
}

FbxLayer* 
LcFBXMeshInfo::GetLayer(LtInt32 pindex) const
{
   if ( (pindex>=0) && (pindex<(LtInt32)m_layers.size()) )
   {
      return m_layers[pindex];
   }
   else
   {
      return NULL;
   }
}


LtInt32 
LcFBXMeshInfo::GetVertexId(LtInt32 polygon_index, LtInt32 vertex_index) const
{
   if ( polygon_index<0 || polygon_index>=m_polygon_count)
   {
      return -1;
   }

   if ( vertex_index<0 || vertex_index>=m_polygon_sizes[polygon_index] )
   {
      return -1;
   }

   return (m_cached_vertex_sum[polygon_index]+vertex_index); 
}


LcFBXGeometry::LcFBXGeometry(FbxManager* manager, LcFBXScene* scene, 
                             FbxNodeAttribute::EType type)
{
   m_type = type;
   m_manager = manager;
   m_fbx_scene = scene;
}

LcFBXGeometry::~LcFBXGeometry()
{

}

LcNwcNode
LcFBXGeometry::Draw(FbxNode* node, 
                    FbxAMatrix& global_position, 
                    FbxAMatrix& parent_global_position,
                    LcFBXScene& fbx_scene,
                    const wchar_t* name,
                    FbxTime* time, 
                    FbxPose* pose)
{
   FbxNodeAttribute* mesh_attr = NULL;
   FbxGeometryConverter converter(m_manager);
   LcNwcNode node_ret(NULL);
   if (FbxNodeAttribute::eTrimNurbsSurface == m_type)
   {
      FbxTrimNurbsSurface* trim_attr = (FbxTrimNurbsSurface*) node->GetNodeAttribute();
      LcNwcGeometry geo = DrawNurbsSurface(trim_attr, global_position, TRUE);
      if (geo.GetHandle())
      {
         geo.SetName(name);
         node_ret = geo;
         LcFBXMaterialFactors factor;
         geo.AddAttribute(LcFBXMaterial::GetNwcMaterial(node, factor));
      }
   }
   else if (FbxNodeAttribute::eNurbsCurve == m_type)
   {
      FbxNurbsCurve* nurb_curve_attr = (FbxNurbsCurve*) node->GetNodeAttribute();
      LcNwcGeometry geo = DrawNurbsCurve(nurb_curve_attr, global_position);
      if (geo.GetHandle())
      {
         geo.SetName(name);
         node_ret = geo;
      }
   }
   else if (FbxNodeAttribute::eNurbs == m_type)
   {
      // Convert nurbs into mesh
      FbxNurbs* nurb_attr = (FbxNurbs*) node->GetNodeAttribute();
      mesh_attr = converter.Triangulate(nurb_attr, true);
   }
   else if (FbxNodeAttribute::ePatch == m_type)
   {
      // Convert patch into mesh
      FbxPatch* patch_attr = (FbxPatch*) node->GetNodeAttribute();
      mesh_attr = converter.Triangulate(patch_attr, true);
   }
   else if (FbxNodeAttribute::eNurbsCurve == m_type)
   {
      // Convert nurbs surface to nurbs, then trangulate to mesh.
      FbxNurbsSurface* nurb_surface = (FbxNurbsSurface*) node->GetNodeAttribute();
      FbxNurbs* nurb_attr = converter.ConvertNurbsSurfaceToNurbs(nurb_surface);
      mesh_attr = converter.Triangulate(nurb_attr, true);
   }
   else if (FbxNodeAttribute::eSkeleton == m_type)
   {
      LcNwcGeometry geometry = DrawSkeleton(node, parent_global_position, global_position);
      if (geometry.GetHandle())
      {
         geometry.SetName(name);
         node_ret = geometry;
      }
   }
   else if (FbxNodeAttribute::eMesh == m_type)
   {
      mesh_attr = (FbxMesh*) node->GetNodeAttribute();
   }

   // Draw mesh
   if (NULL != mesh_attr && LiFBXIsValidTranMatirx(global_position))
   {  
      node_ret = DrawMesh(node, (FbxMesh*)mesh_attr, global_position, fbx_scene, name, time, pose);
   }

   return node_ret;
}

LcNwcNode
LcFBXGeometry::DrawMesh(FbxNode* node,
                        FbxMesh* mesh, 
                        FbxAMatrix& global_pos, 
                        LcFBXScene& fbx_scene,
                        const wchar_t* name,
                        FbxTime* time, 
                        FbxPose* pose)
{
   if (NULL == mesh)
      return NULL;

   LcNwcNode node_ret(NULL);
   LcNwcGroup nwc_group;
   LtChar const* cmesh_name = mesh->GetName();
   LcUWideStringBuffer wmesh_name;
   if (cmesh_name)
   {
      LcUString::UTF8ToWideString(cmesh_name, &wmesh_name);
   }
   
   LcFBXMeshInfo mesh_info(mesh);
   if (!mesh_info.IsValid())
   {
      return NULL;
   }

   // Create a copy of the vertex array to receive vertex deformations.
   // Discuss: remove compute shape deformation or not.
   LtInt32 vertex_count = mesh_info.GetControlPointsCount();
   if (vertex_count<=0)
   {
      return NULL;
   }
   FbxVector4* pvertex = new FbxVector4[vertex_count];
   memcpy(pvertex, mesh->GetControlPoints(), vertex_count * sizeof(FbxVector4));


   if (mesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
      (static_cast<FbxVertexCacheDeformer*>(mesh->GetDeformer(0, FbxDeformer::eVertexCache)))->IsActive())
   {
      ReadVertexCacheData(mesh, time, pvertex);
   }
   else
   {
      if (mesh->GetShapeCount())
      {
         // Deform the vertex array with the shapes.
         ComputeShapeDeformation(mesh, time, pvertex);
      }

      //we need to get the number of clusters
      LtInt32 skin_count = 0, cluster_count = 0;
      skin_count = mesh->GetDeformerCount(FbxDeformer::eSkin);
      for( LtInt32 i=0; i< skin_count; i++)
         cluster_count += ((FbxSkin *)(mesh->GetDeformer(i, FbxDeformer::eSkin)))->GetClusterCount();
      if (cluster_count)
      {
         // Deform the vertex array with the links.
         ComputeClusterDeformation(node, mesh, global_pos, pvertex, time, pose);
      }
   }

   bool bvisibility = node->GetVisibility();

   // store all this node's material in order to improve efficiency
   fbx_scene.StoreMaterials(node);

   MaterialMap material_map;

   // In this mesh, store all the polygon sets which are mapped by corresponding texture.
   FbxLayerElementArrayTemplate<LtInt32>* material_index_array = NULL;
   bool got_mat_indices = mesh->GetMaterialIndices(&material_index_array);
   FbxSurfaceMaterial* sur_mat = NULL;
   if (got_mat_indices && material_index_array)
   {
      for (LtInt32 polygon_index =0; polygon_index<mesh->GetPolygonCount(); polygon_index++)
      {
         LtInt32 mat_id = material_index_array->GetAt(polygon_index);
         SetMapData(node,fbx_scene, mat_id, material_map, polygon_index);
      }
   }

   // draw all the polygon sets
   MaterialMap::iterator iter;
   LcFBXUVParametersCreator uvcreator(pvertex,vertex_count);

   

   for (iter = material_map.begin(); iter!=material_map.end(); iter++)
   {
      if (material_map.size()==1)
      {
         break;
      }
      FbxTexture::EUnifiedMappingType map_type = iter->second.m_mapping_type;
      LtBitfield vertex_properties = GetVectexProperties(mesh, map_type);
      
      LcNwcGeometry geo;

      bool uv_swap = iter->second.m_uv_swap;

      if ( iter->second.m_polygon_list.size()<1 )
      {
         continue;
      }

      DrawPolygons(fbx_scene, geo, mesh, mesh_info, pvertex, 
         iter->second.m_polygon_list, vertex_properties, global_pos,
         uv_swap, uvcreator, map_type, !(iter->second.m_is_mtl_from_blob));
      if (geo.GetHandle())
      {
         // differentiate the name with different materials
         LcUWideStringBuffer node_name;
         LtNat32 mat_id = (LtNat32)iter->first;
         if (!wmesh_name.IsEmpty())
         {
            node_name.Format(L"%s_%d", wmesh_name.GetString(), mat_id);
         }
         else
         {
            node_name.Format(L"%s_%d", name, mat_id);
         }
         geo.SetName(node_name.GetString());

         LcNwcMaterial nwc_mat = iter->second.m_nwc_material;
         if (nwc_mat.GetHandle())
         {
            geo.AddAttribute(nwc_mat);
         }
         LcNwcAutodeskMaterial nwc_adsk_mat = iter->second.m_adsk_material;
         if(nwc_adsk_mat.GetHandle())
         {
            geo.AddAttribute(nwc_adsk_mat);
         }

         if (!bvisibility)
         {
            geo.SetHidden(true);
         }
         nwc_group.AddNode(geo);
      }
   }

   // draw remaining polygons after draw all the polygon sets that are mapped by the corresponding texture.
   LtFBXListOfPolygonIndices remaining_polygons;
   if (material_map.size()==1)
   {
      for (LtInt32 i =0; i< mesh_info.GetPolygonCount(); i++)
      {
         remaining_polygons.push_back(i);
      }
   }
   else
   {
      GetRemainingPolygons(mesh, mesh_info, material_map, remaining_polygons);
   }

   LcNwcGeometry remaining_geo;
   if (remaining_polygons.size()>=1)
   {
      

      LcFBXMaterialFactors mat_factors;
      LcNwcMaterial nwc_mat = LcFBXMaterial::GetNwcMaterial(node, mat_factors);
      if (nwc_mat.GetHandle())
      {
         remaining_geo.AddAttribute(nwc_mat);
      }

      bool uv_swap = false;
      LtBitfield remaining_vertex_properties = LI_NWC_VERTEX_NONE;
      FbxTexture::EUnifiedMappingType mapping_type = FbxTexture::eUMT_UV;

      // try convert fbx material to adsk material, and attach it to node
      bool adjust_uv = false;
      LcFBXAdskMtlUtil::AttachAdskMtl(node, mesh, fbx_scene,remaining_geo,uv_swap,mapping_type,adjust_uv);

      remaining_vertex_properties = GetVectexProperties(mesh,mapping_type);
      DrawPolygons(fbx_scene, remaining_geo , mesh, mesh_info, pvertex, remaining_polygons, remaining_vertex_properties, global_pos, uv_swap, uvcreator, mapping_type,adjust_uv);

      if (!bvisibility)
      {
         remaining_geo.SetHidden(true);
      }

      nwc_group.AddNode(remaining_geo);
   }

   delete []pvertex;

   if (material_map.size()<=1)
   {
      node_ret = remaining_geo;
   }
   else
   {
      node_ret = nwc_group;
   }

   if (!wmesh_name.IsEmpty())
   {
      node_ret.SetName(wmesh_name.GetString());
   }
   else
   {
      LcUWideString wname(name);
      if(!wname.IsEmpty())
      {
         node_ret.SetName(wname.GetString());
      }
   }
   return node_ret;
}

LcNwcGeometry 
LcFBXGeometry::DrawSkeleton(FbxNode*node, 
                            FbxAMatrix& parent_global_pos, 
                            FbxAMatrix& global_pos)
{
   FbxSkeleton* lSkeleton = (FbxSkeleton*) node->GetNodeAttribute();

   if (!lSkeleton)
   {
      return LcNwcGeometry(NULL);
   }

   LtChar const* cname = lSkeleton->GetName();
   LcUWideStringBuffer wname;
   if (cname)
   {
      LcUString::UTF8ToWideString(cname, &wname);
   }


   // Only draw the skeleton if it's a limb node and if 
   // the parent also has an attribute of type skeleton.
   if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode &&
      node->GetParent() &&
      node->GetParent()->GetNodeAttribute() &&
      node->GetParent()->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
   {
      LcNwcGeometry geo;

      if (!wname.IsEmpty())
      {
         geo.SetName(wname.GetString());
      }

      LcNwcGeometryStream stream = geo.OpenStream();
      stream.Begin(LI_NWC_VERTEX_COLOR);
      stream.Color(0.4, 0.4, 0.4, 1);
      LtPoint pt1 = {parent_global_pos.GetT()[0],
         parent_global_pos.GetT()[1],
         parent_global_pos.GetT()[2]};
      LtPoint pt2 = {global_pos.GetT()[0],
         global_pos.GetT()[1],
         global_pos.GetT()[2]};
      stream.LineVertex(pt1);
      stream.LineVertex(pt2);
      stream.End();
      geo.CloseStream(stream);

      return geo;
   }

   return LcNwcGeometry(NULL);
}

LcNwcGeometry 
LcFBXGeometry::DrawNurbsCurve(FbxNurbsCurve* nurbs_curve, 
                              FbxAMatrix& global_pos, 
                              FbxTime* time, 
                              FbxPose* pose)
{
   LiUAssertAny(nurbs_curve);
   if (NULL==nurbs_curve)
      return LcNwcGeometry(NULL);

   LcNwcGeometry geometry;
   LcNwcGeometryStream stream = geometry.OpenStream();
   stream.Begin(LI_NWC_VERTEX_NONE);
   stream.SetTransform((LtFloat*)global_pos);
   LtPoint pt;
   LcNwcCurve nwc_curve = DrawNwcCurve(nurbs_curve, pt);
   stream.Curve(nwc_curve);
   stream.End();
   geometry.CloseStream(stream);

   return geometry;
}

LcNwcCurve
LcFBXGeometry::DrawNwcCurve(FbxNurbsCurve* nurbs_curve, LtPoint vertex)
{
   LiUAssertAny(nurbs_curve);
   if (NULL==nurbs_curve)
      return LcNwcCurve(NULL);

   LtInt32 i;
   LtInt32 pole_nums = nurbs_curve->GetControlPointsCount();
   FbxVector4* poles_vector = nurbs_curve->GetControlPoints();
   LcUAutoArray<LtPoint> poles(new LtPoint[pole_nums]);
   for (i=0; i<pole_nums; ++i)
   {
      poles[i][0] = poles_vector[i][0];
      poles[i][1] = poles_vector[i][1];
      poles[i][2] = poles_vector[i][2];
   }
   LtInt32 knot_nums = nurbs_curve->GetKnotCount();
   LtFloat* knots = nurbs_curve->GetKnotVector();
   LtInt32 order = nurbs_curve->GetOrder();
   order = order-1;

   stlport::vector<LtInt32> multi;
   stlport::vector<LtFloat> knots1;
   LtInt32 j = 1;
   for (i=0; i<knot_nums; ++i)
   {
      if (i == knot_nums-1)
      {
         multi.push_back(j);
         knots1.push_back(knots[i]);        
      }
      else
      {
         if (knots[i] == knots[i+1])
         {
            j++;
         }
         else
         {
            multi.push_back(j);
            knots1.push_back(knots[i]);
            j = 1;
         }
      }
   }
   LtInt32 multi_size = static_cast<LtInt32>(multi.size());
   LtInt32 knot_size = static_cast<LtInt32>(knots1.size());
   LiUAssertAny(multi_size == knot_size);
   FbxNurbsCurve::EType type = nurbs_curve->GetType();
   LtInt32 poles_use_num = pole_nums;
   if (FbxNurbsCurve::ePeriodic == type)
      poles_use_num += order;
   else if (FbxNurbsCurve::eClosed == type)
      poles_use_num += 1;
   LcUAutoArray<LtPoint> poles_use(new LtPoint[poles_use_num]);
   if (FbxNurbsCurve::eOpen == type)
      poles_use = poles;
   else
   {
      for (i=0; i<poles_use_num; ++i)
      {
         LtInt32 k = i%pole_nums;
         poles_use[i][0] = poles[k][0];
         poles_use[i][1] = poles[k][1];
         poles_use[i][2] = poles[k][2];
      }
   }

   LcNwcBSplineCurve nwc_curve(poles_use_num, 
      poles_use.GetArray(), 
      knot_size, 
      &knots1[0], 
      &multi[0], 
      order);
   LiUAssertAny(nwc_curve.GetHandle());
   vertex[0] = poles_use[poles_use_num-1][0];
   vertex[1] = poles_use[poles_use_num-1][1];
   vertex[2] = poles_use[poles_use_num-1][2];

   return LcNwcCurve(nwc_curve);
}

LcNwcGeometry
LcFBXGeometry::DrawNurbsSurface(FbxGeometry* nurbs, 
                                FbxAMatrix& global_pos, 
                                LtBoolean is_trimmed,
                                FbxTime* time /* = NULL */, 
                                FbxPose* pose /* = NULL */)
{
   LiUAssertAny(nurbs);
   if (NULL == nurbs)
      return LcNwcGeometry(NULL);
   const FbxNurbsSurface* nurb_surface = NULL;
   if (is_trimmed)
      nurb_surface = FbxCast<FbxTrimNurbsSurface>(nurbs)->GetNurbsSurface();
   else 
      nurb_surface = FbxCast<FbxNurbsSurface>(nurbs);

   LiUAssertAny(nurb_surface);
   if (NULL == nurb_surface)
      return LcNwcGeometry(NULL);

   // Get nurb type in u, v direction respectively.
   FbxNurbsSurface::EType u_type = nurb_surface->GetNurbsUType();
   FbxNurbsSurface::EType v_type = nurb_surface->GetNurbsVType();

   // Get nurb order in u, v direction respectively.
   LtInt32 u_order = nurb_surface->GetUOrder() - 1;
   LtInt32 v_order = nurb_surface->GetVOrder() - 1;

   // Get the number of elements in the knot vector in u, v direction respectively. 
   LtInt32 u_knot_count = nurb_surface->GetUKnotCount();
   LtInt32 v_knot_count = nurb_surface->GetVKnotCount();

   // Get knot vector in u, v direction respectively.
   LtFloat* u_knot_t = nurb_surface->GetUKnotVector();
   LtFloat* v_knot_t = nurb_surface->GetVKnotVector();
   stlport::vector<LtInt32> u_multi, v_multi;
   stlport::vector<LtFloat> u_knot, v_knot;
   LtInt32 i, j = 1;
   for (i=0; i<u_knot_count; ++i)
   {
      if (i == u_knot_count-1)
      {
         u_multi.push_back(j);
         u_knot.push_back(u_knot_t[i]);        
      }
      else
      {
         if (u_knot_t[i] == u_knot_t[i+1])
         {
            j++;
         }
         else
         {
            u_multi.push_back(j);
            u_knot.push_back(u_knot_t[i]);
            j = 1;
         }
      }
   }
   j = 1;
   for (i=0; i<v_knot_count; ++i)
   {
      if (i == v_knot_count-1)
      {
         v_multi.push_back(j);
         v_knot.push_back(v_knot_t[i]);        
      }
      else
      {
         if (v_knot_t[i] == v_knot_t[i+1])
         {
            j++;
         }
         else
         {
            v_multi.push_back(j);
            v_knot.push_back(v_knot_t[i]);
            j = 1;
         }
      }
   }

   // Get number of control points in u,v direction respectively.
   LtInt32 u_count = nurb_surface->GetUCount();
   LtInt32 v_count = nurb_surface->GetVCount();
   LtInt32 u_use_count = u_count;
   LtInt32 v_use_count = v_count;
   if (FbxNurbs::ePeriodic == u_type)
      u_use_count += u_order;
   else if (FbxNurbs::eClosed == u_type)
      u_use_count += 1;

   if (FbxNurbs::ePeriodic == v_type)
      v_use_count += v_order;
   else if (FbxNurbs::eClosed == v_type)
      v_use_count += 1;

   LtInt32 control_count = nurb_surface->GetControlPointsCount();
   LiUAssertAny(control_count == u_count*v_count);
   FbxVector4* poles_vector = nurb_surface->GetControlPoints();
   LtInt32 total_count = u_use_count*v_use_count;
   LtPoint* poles = new LtPoint[total_count];
   for (i=0; i<v_use_count; ++i)
   {
      LtInt32 k=i%v_count;
      for (j=0; j<u_use_count; ++j)
      {
         LtInt32 p=j%u_count;
         poles[i*u_use_count+j][0] = poles_vector[k*u_count+p][0];
         poles[i*u_use_count+j][1] = poles_vector[k*u_count+p][1];
         poles[i*u_use_count+j][2] = poles_vector[k*u_count+p][2];
      }
   }

   //LtInt32 s1 = u_knot.size();
   //LtInt32 s2 = u_multi.size();
   //LtInt32 s3 = v_knot.size();
   //LtInt32 s4 = v_multi.size();

   LcNwcGeometry geometry;
   LcNwcGeometryStream stream = geometry.OpenStream();
   stream.Begin(LI_NWC_VERTEX_NONE);
   stream.SetTransform((LtFloat*)global_pos);
   LcNwcBSplineSurface nwc_surface(u_use_count, 
      v_use_count, 
      poles, 
      static_cast<LtInt32>(u_knot.size()), 
      &(u_knot[0]), 
      &(u_multi[0]), 
      static_cast<LtInt32>(v_knot.size()), 
      &(v_knot[0]), 
      &(v_multi[0]), 
      u_order, 
      v_order, 
      u_type == FbxNurbs::ePeriodic?1:0, 
      v_type == FbxNurbs::ePeriodic?1:0
      );
   LcNwcFace face(nwc_surface, LI_NWC_SENSE_POSITIVE);

   //
   // Currently, we ignore the trimmed implementation, 
   // because fbx toolkit does not give enough boundary information to form the loop
   // LtInt32 trim_region_num = nurbs->GetTrimRegionCount();
   //

   LcNwcBRepEntity entity(face);
   stream.BRepEntity(entity);

   stream.End();
   geometry.CloseStream(stream);

   delete []poles;

   return geometry;
}

LtBitfield 
LcFBXGeometry::GetVectexProperties(FbxGeometry* mesh, FbxTexture::EUnifiedMappingType mapping_type)
{
   LiUAssertAny(mesh);
   LtBitfield vertex_pro = LI_NWC_VERTEX_NONE;
   if (!mesh)
   {
      return vertex_pro;
   }

   if (mapping_type == FbxTexture::eUMT_XY ||
      mapping_type == FbxTexture::eUMT_YZ ||
      mapping_type == FbxTexture::eUMT_XZ)
   {
      vertex_pro |= LI_NWC_VERTEX_TEX_COORD;
   }

   for (LtInt32 i = 0; i < mesh->GetLayerCount(); i++)
   {
      if ( !(mesh->GetLayer(i)) )
      {
         continue;
      }

      vertex_pro |= GetVectexProperty(mesh->GetLayer(i)->GetUVs(), LI_NWC_VERTEX_TEX_COORD);
      vertex_pro |= GetVectexProperty(mesh->GetLayer(i)->GetVertexColors(), LI_NWC_VERTEX_COLOR);
      vertex_pro |= GetVectexProperty(mesh->GetLayer(i)->GetNormals(), LI_NWC_VERTEX_NORMAL);
      break;
   }
   return vertex_pro;
}

LtBitfield
LcFBXGeometry::GetVectexProperty(FbxLayerElement* element, LtBitfield flag)
{
   LtBitfield vertex_pro = LI_NWC_VERTEX_NONE;
   if (!element)
      return vertex_pro;

   return HaveSenseMode(element) ? flag : vertex_pro;
}

// Deform the vertex array with the shapes contained in the mesh.
void LcFBXGeometry::
   ComputeShapeDeformation(FbxMesh* mesh, 
                           FbxTime* time, 
                           FbxVector4* vertex_array)
{
   int lVertexCount = mesh->GetControlPointsCount();

   FbxVector4* lSrcVertexArray = vertex_array;
   FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
   memcpy(lDstVertexArray, vertex_array, lVertexCount * sizeof(FbxVector4));

   int lBlendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
   for(int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
   {
      FbxBlendShape* lBlendShape = (FbxBlendShape*)mesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

      int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
      for(int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; ++lChannelIndex)
      {
         FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);

         if(lChannel)
         {
            // Get the percentage of influence of the shape.
            FbxAnimCurve* lFCurve = mesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, m_fbx_scene->GetCurrentAnimationLayer());
            if (!lFCurve) continue;
            double lWeight = lFCurve->Evaluate(*time);

            //Find which shape should we use according to the weight.
            int lShapeCount = lChannel->GetTargetShapeCount();
            double* lFullWeights = lChannel->GetTargetShapeFullWeights();
            for(int lShapeIndex = 0; lShapeIndex<lShapeCount; ++lShapeIndex)
            {
               FbxShape* lShape = NULL;
               if(lWeight > 0 && lWeight <= lFullWeights[0])
               {
                  lShape = lChannel->GetTargetShape(0);
               }
               if(lWeight > lFullWeights[lShapeIndex] && lWeight < lFullWeights[lShapeIndex+1])
               {
                  lShape = lChannel->GetTargetShape(lShapeIndex+1);
               }

               if(lShape)
               {				
                  for (int j = 0; j < lVertexCount; j++)
                  {
                     // Add the influence of the shape vertex to the mesh vertex.
                     FbxVector4 lInfluence = (lShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight * 0.01;
                     lDstVertexArray[j] += lInfluence;
                  }						
               }
            }//For each target shape
         }//If lChannel is valid
      }//For each blend shape channel
   }//For each blend shape deformer

   memcpy(vertex_array, lDstVertexArray, lVertexCount * sizeof(FbxVector4));

   delete [] lDstVertexArray;
}

// void 
// LcFBXGeometry::ComputeShapeDeformation(FbxMesh* mesh, 
//                                        FbxTime* time, 
//                                        FbxVector4* vertex_array)
// {
//    LtInt32 i, j;
//    LtInt32 lShapeCount = mesh->GetShapeCount();
//    LtInt32 lVertexCount = mesh->GetControlPointsCount();
// 
//    FbxVector4* lSrcVertexArray = vertex_array;
//    FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
//    memcpy(lDstVertexArray, vertex_array, lVertexCount * sizeof(FbxVector4));
// 
//    for (i = 0; i < lShapeCount; i++)
//    {
//       FbxShape* lShape = mesh->GetShape(i);
// 
//       // Get the percentage of influence of the shape.
//       FbxAnimCurve* lFCurve = mesh->GetShapeChannel(i, m_fbx_scene->GetCurrentAnimationLayer());
//       if (!lFCurve) continue;
//       LtFloat lWeight = lFCurve->Evaluate(*time) / 100.0;
// 
//       for (j = 0; j < lVertexCount; j++)
//       {
//          // Add the influence of the shape vertex to the mesh vertex.
//          FbxVector4 lInfluence = (lShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight;
//          lDstVertexArray[j] += lInfluence;
//       }
//    }
// 
//    memcpy(vertex_array, lDstVertexArray, lVertexCount * sizeof(FbxVector4));
// 
//    delete [] lDstVertexArray;
// }

void 
LcFBXGeometry::ComputeClusterDeformation(FbxNode* node, 
                                         FbxMesh* mesh, 
                                         FbxAMatrix& global_pos, 
                                         FbxVector4* vectex_array, 
                                         FbxTime* time, 
                                         FbxPose* pose)
{
   // All the links must have the same link mode.
   FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

   LtInt32 i, j;
   LtInt32 lClusterCount=0;

   LtInt32 lVertexCount = mesh->GetControlPointsCount();
   LtInt32 lSkinCount=mesh->GetDeformerCount(FbxDeformer::eSkin);

   FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
   memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));
   LtFloat* lClusterWeight = new LtFloat[lVertexCount];
   memset(lClusterWeight, 0, lVertexCount * sizeof(LtFloat));

   if(lClusterMode == FbxCluster::eAdditive)
   {   
      for (i = 0; i < lVertexCount; i++)
      {   
         lClusterDeformation[i].SetIdentity();
      }
   }

   for(i=0; i<lSkinCount; ++i)
   {
      lClusterCount =( (FbxSkin *)mesh->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();
      for (j=0; j<lClusterCount; ++j)
      {
         FbxCluster* lCluster =((FbxSkin *) mesh->GetDeformer(i, FbxDeformer::eSkin))->GetCluster(j);
         if (!lCluster->GetLink())
            continue;
         FbxAMatrix lReferenceGlobalInitPosition;
         FbxAMatrix lReferenceGlobalCurrentPosition;
         FbxAMatrix lClusterGlobalInitPosition;
         FbxAMatrix lClusterGlobalCurrentPosition;
         FbxAMatrix lReferenceGeometry;
         FbxAMatrix lClusterGeometry;

         FbxAMatrix lClusterRelativeInitPosition;
         FbxAMatrix lClusterRelativeCurrentPositionInverse;
         FbxAMatrix lVertexTransformMatrix;

         if (lClusterMode == FbxCluster::eAdditive && lCluster->GetAssociateModel())
         {
            lCluster->GetTransformAssociateModelMatrix(lReferenceGlobalInitPosition);
            lReferenceGlobalCurrentPosition = LiFBXGetGlobalPosition(lCluster->GetAssociateModel(), pose, time);
            // Geometric transform of the model
            lReferenceGeometry = LiFBXGetGeometryOffset(lCluster->GetAssociateModel());
            lReferenceGlobalCurrentPosition *= lReferenceGeometry;
         }
         else
         {
            lCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
            lReferenceGlobalCurrentPosition = global_pos;
            // Multiply lReferenceGlobalInitPosition by Geometric Transformation
            lReferenceGeometry = LiFBXGetGeometryOffset(node);
            lReferenceGlobalInitPosition *= lReferenceGeometry;
         }
         // Get the link initial global position and the link current global position.
         lCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
         lClusterGlobalCurrentPosition = LiFBXGetGlobalPosition(lCluster->GetLink(), pose, time);

         // Compute the initial position of the link relative to the reference.
         lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

         // Compute the current position of the link relative to the reference.
         lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

         // Compute the shift of the link relative to the reference.
         lVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;

         LtInt32 k;
         LtInt32 lVertexIndexCount = lCluster->GetControlPointIndicesCount();

         if ( (lCluster->GetControlPointIndices())&&(lCluster->GetControlPointWeights()) )
         {
            for (k = 0; k < lVertexIndexCount; ++k) 
            {   
               LtInt32 lIndex = lCluster->GetControlPointIndices()[k];

               // Sometimes, the mesh can have less points than at the time of the skinning
               // because a smooth operator was active when skinning but has been deactivated during export.
               if (lIndex<0 || lIndex>=lVertexCount)
               {
                  continue;
               }

               LtFloat lWeight = lCluster->GetControlPointWeights()[k];

               if (lWeight == 0.0)
               {
                  continue;
               }

               // Compute the influence of the link on the vertex.
               FbxAMatrix lInfluence = lVertexTransformMatrix;
               LiFBXMatrixScale(lInfluence, lWeight);

               if (lClusterMode == FbxCluster::eAdditive)
               {   
                  // Multiply with to the product of the deformations on the vertex.
                  LiFBXMatrixAddToDiagnoal(lInfluence, 1.0 - lWeight);
                  lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

                  // Set the link to 1.0 just to know this vertex is influenced by a link.
                  lClusterWeight[lIndex] = 1.0;
               }
               else // lLinkMode == FbxLink::eNORMALIZE || lLinkMode == FbxLink::eTOTAL1
               {
                  // Add to the sum of the deformations on the vertex.
                  LiFBXMatrixAdd(lClusterDeformation[lIndex], lInfluence);

                  // Add to the sum of weights to either normalize or complete the vertex.
                  lClusterWeight[lIndex] += lWeight;
               }

            }
         }

      }
   }

   for (i = 0; i < lVertexCount; i++) 
   {
      FbxVector4 lSrcVertex = vectex_array[i];
      FbxVector4& lDstVertex = vectex_array[i];
      LtFloat lWeight = lClusterWeight[i];

      // Deform the vertex if there was at least a link with an influence on the vertex,
      if (lWeight != 0.0) 
      {
         lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);

         if (lClusterMode == FbxCluster::eNormalize)
         {
            // In the normalized link mode, a vertex is always totally influenced by the links. 
            lDstVertex /= lWeight;
         }
         else if (lClusterMode == FbxCluster::eTotalOne)
         {
            // In the total 1 link mode, a vertex can be partially influenced by the links. 
            lSrcVertex *= (1.0 - lWeight);
            lDstVertex += lSrcVertex;
         }
      } 
   }

   delete [] lClusterDeformation;
   delete [] lClusterWeight;
}

void 
LcFBXGeometry::ReadVertexCacheData(FbxMesh* pMesh, 
                                   FbxTime* pTime, 
                                   FbxVector4* pVertexArray)
{
   FbxVertexCacheDeformer* lDeformer     = static_cast<FbxVertexCacheDeformer*>(pMesh->GetDeformer(0, FbxDeformer::eVertexCache));
   FbxCache*               lCache        = lDeformer->GetCache();
   LtInt32                      lChannelIndex = -1;
   LtNat32             lVertexCount  = (LtNat32)pMesh->GetControlPointsCount();
   bool                     lReadSucceed  = false;
   LtFloat*                  lReadBuf      = new LtFloat[3*lVertexCount];

   if (lCache->GetCacheFileFormat() == FbxCache::eMayaCache)
   {
      if ((lChannelIndex = lCache->GetChannelIndex(lDeformer->GetCacheChannel())) > -1)
      {
         lReadSucceed = lCache->Read(lChannelIndex, *pTime, lReadBuf, lVertexCount);
      }
   }
   else // ePC2
   {
      lReadSucceed = lCache->Read((LtNat32)pTime->GetFrameCount(), lReadBuf, lVertexCount);
   }

   if (lReadSucceed)
   {
      LtNat32 lReadBufIndex = 0;

      while (lReadBufIndex < 3*lVertexCount)
      {
         // In statements like "pVertexArray[lReadBufIndex/3].SetAt(2, lReadBuf[lReadBufIndex++])", 
         // on Mac platform, "lReadBufIndex++" is evaluated before "lReadBufIndex/3". 
         // So separate them.
         pVertexArray[lReadBufIndex/3][0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
         pVertexArray[lReadBufIndex/3][1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
         pVertexArray[lReadBufIndex/3][2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
      }
   }

   delete [] lReadBuf;
}


bool
LcFBXGeometry::DrawPolygons(LcFBXScene& fbx_scene,
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
                            bool adjust_uv)  
{
   if (!mesh || polygons_list.size()<1)
   {
      return false;
   }

   LcNwcGeometryStream stream = geometry.OpenStream();
   stream.Begin(vertex_properties);

   stream.SetTransform((LtFloat*)global_pos);
   for (LtNat32 list_index =0; list_index<polygons_list.size(); list_index++)
   {
      LtInt32 polygon_id = polygons_list[list_index];
      if (polygon_id<0)
      {
         continue;
      }

      LtInt32 polygon_size = mesh_info.GetPolygonSize(polygon_id);

      bool is_triangle =false;
      if ( 3==polygon_size )
      {
         is_triangle = true;
      }

      if (!is_triangle)
      {
         stream.BeginPolygon();
         stream.BeginPolygonContour();
      }

      for (LtInt32 vertex_index =0; vertex_index<polygon_size; vertex_index++ )
      {
         LtInt32 index = mesh->GetPolygonVertex(polygon_id, vertex_index);

         if (index<0 || index>=mesh_info.GetControlPointsCount())
         {
            continue;
         }

         // add vertex properties, vertex colors, normals, and UVs
         FbxColor color;
         FbxVector2 uv;
         FbxVector4 normal;

         LtInt32 vertex_id = -1;
         vertex_id = mesh_info.GetVertexId(polygon_id, vertex_index);

         if (GetVertexColor(mesh, mesh_info, polygon_id, vertex_index, vertex_id, index, color))
         {
            stream.Color(color.mRed, color.mGreen, color.mGreen, color.mAlpha);
         }

         if (FbxTexture::eUMT_XY==mapping_type||
            FbxTexture::eUMT_XZ==mapping_type||
            FbxTexture::eUMT_YZ==mapping_type)
         {
            LtFloat u = 0;
            LtFloat v = 0;

            bool got_u =false;
            bool got_v = false;
            got_u = uvcreator.GetU(u, index,mapping_type);
            got_v = uvcreator.GetV(v, index,mapping_type);
            if (got_u&&got_v)
            {
               if(!adjust_uv) 
               {
                  stream.TexCoord(u, v);
               }
               else
               {
                  if (swap_uv)
                  {
                     stream.TexCoord(v, 1-u);
                  }
                  else
                  {
                     stream.TexCoord(u, 1-v);
                  }
               }
            }
         }
         else if (GetVertexUV(mesh, mesh_info, polygon_id, vertex_index, vertex_id, index, uv))
         {
            //uv parameters gotten from fbxsdk, set nwc uv for u and 1-v.
            if(!adjust_uv) 
            {
               stream.TexCoord(uv[0], uv[1]);
            }
            else
            {
               if (swap_uv)
               {
                  stream.TexCoord(uv[1], 1-uv[0]);
               }
               else
               {
                  stream.TexCoord(uv[0], 1-uv[1]);
               }
            }
         }

         if (GetVertexNormal(mesh, mesh_info, polygon_id, vertex_index, vertex_id, index, normal))
         {
            stream.Normal(normal[0], normal[1], normal[2]);
         }

         if (is_triangle)
         {
            stream.TriangleVertex( pvertex[index][0], 
               pvertex[index][1],
               pvertex[index][2] );
         }
         else
         {
            stream.PolygonVertex( pvertex[index][0], 
               pvertex[index][1],
               pvertex[index][2] );
         }

      }

      if (!is_triangle)
      {
         stream.EndPolygonContour();
         stream.EndPolygon();
      }
   }
   stream.End();
   geometry.CloseStream(stream);

   return true;
}


void
LcFBXGeometry::GetRemainingPolygons(FbxMesh* mesh,
                                    const LcFBXMeshInfo& mesh_info,
                                    MaterialMap& material_map,
                                    LtFBXListOfPolygonIndices& remaining_polgons)
{
   if (!mesh)
   {
      return;
   }

   LtInt32 count = mesh_info.GetPolygonCount();
   if (count<1)
   {
      return;
   }

   remaining_polgons.clear();

   bool* tag_polygon_set = new bool[count];
   for (LtInt32 i =0; i<count; i++)
   {
      tag_polygon_set[i] = false;
   }

   for (MaterialMap::iterator iter = material_map.begin(); iter!=material_map.end(); iter++)
   {
      for (LtNat32 i=0; i<iter->second.m_polygon_list.size(); i++)
      {
         LtInt32 polygon_id = iter->second.m_polygon_list[i];
         if ( polygon_id>=0 && polygon_id<count )
         {
            tag_polygon_set[polygon_id] = true;
         }
      }
   }

   for (LtInt32 i =0; i<count; i++)
   {
      if (!tag_polygon_set[i])
      {
         remaining_polgons.push_back(i);
      }
   } 
   delete []tag_polygon_set;

}

bool
LcFBXGeometry::HaveSenseMode(FbxLayerElement* lelement)
{
   if (!lelement)
   {
      return false;
   }

   switch (lelement->GetMappingMode())
   {
   case FbxLayerElement::eByControlPoint:
      {
         switch (lelement->GetReferenceMode())
         {
         case FbxLayerElement::eDirect:
         case FbxLayerElement::eIndexToDirect:
            {
               return true;
            }
            break;
         default:
            break; // other reference modes not shown here!
         }
      }
      break;

   case FbxLayerElement::eByPolygonVertex:
      {
         switch (lelement->GetReferenceMode())
         {
         case FbxLayerElement::eDirect:
         case FbxLayerElement::eIndexToDirect:
            {
               return true;
            }
            break;
         default:
            break; // other reference modes not shown here!
         }
      }
      break;

   case FbxLayerElement::eByPolygon: // doesn't make much sense for UVs
   case FbxLayerElement::eAllSame:   // doesn't make much sense for UVs
   case FbxLayerElement::eNone:       // doesn't make much sense for UVs
      break;
   }

   return false;
}


bool
LcFBXGeometry::GetVertexColor(FbxMesh* mesh,
                              const LcFBXMeshInfo& mesh_info,
                              LtInt32 plygon_index,
                              LtInt32 vertex_index,
                              LtInt32 vertexid,
                              LtInt32 control_point_index,
                              FbxColor& color)
{
   if( !IsValidData(mesh, mesh_info, plygon_index, vertex_index, vertexid, control_point_index) )
   {
      return false;
   }

   for (LtInt32 i =0; i<mesh_info.GetLayerCount(); i++)
   {
      if ( !(mesh_info.GetLayer(i)) )
      {
         continue;
      }
      FbxLayerElementVertexColor* levcolor = mesh_info.GetLayer(i)->GetVertexColors();
      if (levcolor)
      {
         switch (levcolor->GetMappingMode())
         {
         case FbxLayerElement::eByControlPoint:
            switch (levcolor->GetReferenceMode())
            {
            case FbxLayerElement::eDirect:
               {
                  color = levcolor->GetDirectArray().GetAt(control_point_index);
                  return true; 
               }
               break;
            case FbxLayerElement::eIndexToDirect:
               {
                  LtInt32 id = levcolor->GetIndexArray().GetAt(control_point_index);
                  if (id>=0)
                  {
                     color = levcolor->GetDirectArray().GetAt(id);  
                     return true;
                  }
               }
               break;
            default:
               break; // other reference modes not shown here!
            }
            break;

         case FbxLayerElement::eByPolygonVertex:
            {

               switch (levcolor->GetReferenceMode())
               {
               case FbxLayerElement::eDirect:
                  {
                     color = levcolor->GetDirectArray().GetAt(vertexid);
                     return true;
                  }   
                  break;
               case FbxLayerElement::eIndexToDirect:
                  {
                     LtInt32 id = levcolor->GetIndexArray().GetAt(vertexid);
                     if (id>=0)
                     {
                        color = levcolor->GetDirectArray().GetAt(id);
                        return true;
                     }
                  }
                  break;
               default:
                  break; // other reference modes not shown here!
               }
            }
            break;

         case FbxLayerElement::eByPolygon: // doesn't make much sense for colors
         case FbxLayerElement::eAllSame:   // doesn't make much sense for colors
         case FbxLayerElement::eNone:       // doesn't make much sense for colors
            break;
         }
      }
   }
   return false;
}

bool
LcFBXGeometry::GetVertexUV(FbxMesh* mesh,
                           const LcFBXMeshInfo& mesh_info,
                           LtInt32 plygon_index,
                           LtInt32 vertex_index,
                           LtInt32 vertexid,
                           LtInt32 control_point_index,
                           FbxVector2& uv)
{
   if( !IsValidData(mesh, mesh_info, plygon_index, vertex_index, vertexid, control_point_index) )
   {
      return false;
   }

   for (LtInt32 i =0; i<mesh_info.GetLayerCount(); i++)
   {
      if ( !(mesh_info.GetLayer(i)) )
      {
         continue;
      }
      FbxLayerElementUV* leuv = mesh_info.GetLayer(i)->GetUVs();
      if (leuv)
      {
         switch (leuv->GetMappingMode())
         {
         case FbxLayerElement::eByControlPoint:
            switch (leuv->GetReferenceMode())
            {
            case FbxLayerElement::eDirect:
               {
                  uv = leuv->GetDirectArray().GetAt(control_point_index);
                  return true;
               }
               break;
            case FbxLayerElement::eIndexToDirect:
               {
                  LtInt32 id = leuv->GetIndexArray().GetAt(control_point_index);
                  uv = leuv->GetDirectArray().GetAt(id);
                  return true;

               }
               break;
            default:
               break; // other reference modes not shown here!
            }
            break;

         case FbxLayerElement::eByPolygonVertex:
            {
               LtInt32 ltexture_uv_index = mesh->GetTextureUVIndex(plygon_index, vertex_index);
               switch (leuv->GetReferenceMode())
               {
               case FbxLayerElement::eDirect:
               case FbxLayerElement::eIndexToDirect:
                  {
                     uv = leuv->GetDirectArray().GetAt(ltexture_uv_index);
                     return true;

                  }
                  break;
               default:
                  break; // other reference modes not shown here!
               }
            }
            break;

         case FbxLayerElement::eByPolygon: // doesn't make much sense for UVs
         case FbxLayerElement::eAllSame:   // doesn't make much sense for UVs
         case FbxLayerElement::eNone:       // doesn't make much sense for UVs
            break;
         }
      }
   }
   return false;
}


bool
LcFBXGeometry::GetVertexNormal(FbxMesh* mesh,
                               const LcFBXMeshInfo& mesh_info,
                               LtInt32 plygon_index,
                               LtInt32 vertex_index,
                               LtInt32 vertexid,
                               LtInt32 control_point_index,
                               FbxVector4& normal)
{
   if( !IsValidData(mesh, mesh_info, plygon_index, vertex_index, vertexid, control_point_index) )
   {
      return false;
   }

   for (LtInt32 i =0; i<mesh_info.GetLayerCount(); i++)
   {
      if ( !(mesh_info.GetLayer(i)) )
      {
         continue;
      }

      FbxLayerElementNormal* lenormal = mesh_info.GetLayer(i)->GetNormals();
      if (lenormal)
      {
         switch (lenormal->GetMappingMode())
         {
         case FbxLayerElement::eByControlPoint:
            switch (lenormal->GetReferenceMode())
            {
            case FbxLayerElement::eDirect:
               {
                  normal = lenormal->GetDirectArray().GetAt(control_point_index);
                  return true; 
               }
               break;
            case FbxLayerElement::eIndexToDirect:
               {
                  LtInt32 id = lenormal->GetIndexArray().GetAt(control_point_index);
                  if (id>=0)
                  {
                     normal = lenormal->GetDirectArray().GetAt(id);
                     return true;
                  }
               }
               break;
            default:
               break; // other reference modes not shown here!
            }
            break;

         case FbxLayerElement::eByPolygonVertex:
            {

               switch (lenormal->GetReferenceMode())
               {
               case FbxLayerElement::eDirect:
                  {
                     normal = lenormal->GetDirectArray().GetAt(vertexid);
                     return true;
                  }   
                  break;
               case FbxLayerElement::eIndexToDirect:
                  {
                     LtInt32 id = lenormal->GetIndexArray().GetAt(vertexid);
                     if (id>=0)
                     {
                        normal = lenormal->GetDirectArray().GetAt(id);
                        return true;
                     }
                  }
                  break;
               default:
                  break; // other reference modes not shown here!
               }
            }
            break;

         case FbxLayerElement::eByPolygon: // doesn't make much sense for normals
         case FbxLayerElement::eAllSame:   // doesn't make much sense for normals
         case FbxLayerElement::eNone:       // doesn't make much sense for normals
            break;
         }
      }
   }
   return false;
}

bool
LcFBXGeometry::IsValidData(FbxMesh* mesh,
                           const LcFBXMeshInfo& mesh_info,
                           LtInt32 plygon_index,
                           LtInt32 vertex_index,
                           LtInt32 vertexid,
                           LtInt32 control_point_index)
{
   if (!mesh)
   {
      return false;
   }

   if (plygon_index<0 || plygon_index>=mesh_info.GetPolygonCount() )
   {
      return false;
   }

   if (vertex_index<0 || vertex_index>=mesh_info.GetPolygonSize(plygon_index))
   {
      return false;
   }

   if (vertexid<0)
   {
      return false;
   }

   if (control_point_index<0 || control_point_index>=mesh_info.GetControlPointsCount())
   {
      return false;
   }

   return true;
}


void
LcFBXGeometry::SetMapData(FbxNode* node,
                          LcFBXScene& fbx_scene,
                          int mat_id,
                          MaterialMap& material_map,
                          LtInt32 polygon_id)
{
   if (polygon_id<0)
   {
      return;
   }

   FbxSurfaceMaterial* surfacemat = node->GetMaterial(mat_id);
   if (surfacemat)
   {
      FbxUInt64 m_id = surfacemat->GetUniqueID();

      // find the material in the dict
      MaterialMap::iterator iter;
      if ( (iter = material_map.find(m_id) )!=material_map.end())
      {                     
         iter->second.m_polygon_list.push_back(polygon_id);
      }
      else
      {
         MaterialMapData map_data;

         map_data.m_polygon_list.push_back(polygon_id);

         AttributeMap::iterator iter =fbx_scene.GetMaterialDict().find(m_id);
         if (iter!=fbx_scene.GetMaterialDict().end())
         {
            map_data.m_nwc_material = iter->second.m_nwc_material;
            map_data.m_adsk_material = iter->second.m_adsk_material;
            map_data.m_uv_swap = iter->second.m_uv_swap;
            map_data.m_mapping_type = iter->second.m_mapping_type;

         }
         else
         {
            LcFBXMaterialFactors material_factors;
            LcNwcMaterial nwc_material = LcFBXMaterial::GetNwcMaterial(surfacemat,material_factors);
            map_data.m_nwc_material = nwc_material;

            bool uv_swap = false;
            FbxTexture::EUnifiedMappingType mapping_type = FbxTexture::eUMT_UV;
            LcNwcAutodeskMaterial nwc_adsk_material = LcFBXAdskMtlUtil::CreateAdskMtl(surfacemat, fbx_scene, material_factors, uv_swap, mapping_type);
            map_data.m_adsk_material = nwc_adsk_material;
            map_data.m_uv_swap = uv_swap;
            map_data.m_mapping_type = mapping_type;
         }
         // the adsk material is converted from fbx material
         map_data.m_is_mtl_from_blob = false; 

         if ( NULL!=map_data.m_nwc_material.GetHandle() || NULL!=map_data.m_adsk_material.GetHandle())
         {
            material_map.insert(MaterialMap::value_type(m_id,map_data));
         }
      }
   }
   else
   {
      // for Consistent Material
      MaterialMap::iterator iter;
      if ( (iter = material_map.find((FbxUInt64)mat_id) )!=material_map.end())
      {
         iter->second.m_polygon_list.push_back(polygon_id);
      }
      else
      {
         MaterialMapData map_data;
         map_data.m_polygon_list.push_back(polygon_id);
         LcNwcAutodeskMaterial nwc_adsk_mtl = LcFBXAdskMtlUtil::GetAdskMtlFromBlob(*m_fbx_scene, node, mat_id);
         map_data.m_adsk_material = nwc_adsk_mtl;
         map_data.m_is_mtl_from_blob = true;
         material_map.insert(MaterialMap::value_type((FbxUInt64)mat_id,map_data));
      }
   }
   return;
}



