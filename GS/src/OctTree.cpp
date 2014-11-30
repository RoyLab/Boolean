#include "arithmetic.h"
#include "OctTree.h"
#include "tbb\tick_count.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/spin_mutex.h"

namespace GS{


int PolygonObj::AddPolygon(BaseMesh& mesh, std::hash_map<IndexPair, bool, IndexPairCompare>& sharedObjMap, OctNodeType level)
{
    bool bAdded = false ;
    if ( mpSharedData.use_count() == 1)
        bAdded = true;
    else if ( mpSharedData.use_count() >= 2  )
    { 
        if (mpSharedData == nullptr || mpSharedData->meObjType <= level)
        {
            IndexPair pair;
            int id[2];
            id[0] = GetID();
            id[1] = mTriId;
            MakeIndex(id, pair);
            if (sharedObjMap.find(pair)  == sharedObjMap.end() )
            {
                sharedObjMap[pair] = true;
                bAdded = true;
            }
        }
    }
 
    if (bAdded)
    {
        const TriInfo& tri = mpMesh->TriangleInfo(mTriId);
        return mesh.Add(mpMesh->Vertex(tri.VertexId[0]), mpMesh->Vertex(tri.VertexId[1]), mpMesh->Vertex(tri.VertexId[2]), tri.Normal); 
    }
    return -1;
    
  

}

int PolygonObj::RayFaceTest(const double3& st, const double3& et, const double3& dir, const Plane<double>& splane)
{
     const TriInfo& tri = mpMesh->TriangleInfo(mTriId);
     Point3D& v0 = mpMesh->Vertex( tri.VertexId[0]).pos;
     Point3D& v1 = mpMesh->Vertex( tri.VertexId[1]).pos;
     Point3D& v2 = mpMesh->Vertex( tri.VertexId[2]).pos;
     if (PointWithPlane(tri.Normal, v0, st) == PointWithPlane(tri.Normal, v0, et) )
         return 0;
     double t, u, v;
     if (!RayTriangleIntersectTest(st, dir, v0, v1, v2, u, v, t))
         return 0 ; 
     if (t <= EPSF && t >= -EPSF)
     {
          if (dot((et -st),  tri.Normal) < -EPSF) 
            return -2;
        else return -1;
     }
     if (fabs(u) > EPSF &&  fabs(v) > EPSF && fabs(1-u-v) > EPSF)
        return 1;
     bool A0 = PointWithPlane(splane.Normal(), splane.Distance(), v0);
     bool A1 = PointWithPlane(splane.Normal(), splane.Distance(), v1);
     bool A2 = PointWithPlane(splane.Normal(), splane.Distance(), v2);
     bool B0 = SignDeterminant(st, et, v0, v1);
     bool B1 = SignDeterminant(st, et, v1, v2);
     bool B2 = SignDeterminant(st, et, v2, v0);
     int Inte =!A0 && A1&& B0 || A0&& !A1&& !B0;
     Inte^= (!A1 && A2&& B1 || A1&& !A2&& !B1);
     Inte^= (!A2 && A0&& B2 || A2&& !A0&& !B2);
     return Inte;
      

}

////////////////////////////////////////////////////////////////////
OctTreeNode::OctTreeNode()
    : type(eNormal)
{
    for (int i =0; i < 8 ; i++)
        child[i] = NULL;

}

OctTreeNode::~OctTreeNode()
{

}


OctLeafNode::~OctLeafNode()
{
    for (int i = 0 ; i < polygons.size(); i++)
        delete polygons[i];
    polygons.clear();
}
/////////////////////////////////////////////////////////////////////

OctTree::OctTree()
    :mpRoot(NULL)
    ,mbCarved(false)
{
}

OctTree::~OctTree()
{
    DeleteChilds(mpRoot);
}


void OctTree::DeleteChilds(OctTreeNode* pNode)
{
    if (pNode != NULL)
    {
        for (int i = 0; i < 8; i++)
        {
            DeleteChilds (pNode->child[i]);
        }
        delete pNode;
    }
    pNode = NULL;
      
}

OctTreeNode* OctTree::CreateLeafNode(const Box3& bbox, OctNodeType  NodeType, std::vector<PolygonObj* >& polygons )
{
     OctLeafNode* pNode = new OctLeafNode;
     pNode->bbox      = bbox;
     pNode->type = NodeType;
     pNode->polygons  = polygons;
 
        for (int i = 0; i < pNode->polygons.size(); i++ )
        {
            if (NodeType & eCritical || NodeType & eIntered)
                pNode->polygons[i]->SetObjTypeFlag(NodeType);
        }
     return pNode;
}


void OctTree::BuildOctTree(std::vector<BaseMesh*>& meshes)
{

    std::vector<PolygonObj* > polygons;
    Box3 bbox;
    int nPolygons = 0; 
    for (int i = 0; i < meshes.size(); i++)
        nPolygons+=meshes[i]->PrimitiveCount();
    polygons.reserve(nPolygons);

    for(int i = 0 ; i < meshes.size(); i++)
    {
        bbox.IncludeBox(meshes[i]->AABB());
        for (int j = 0 ; j < meshes[i]->PrimitiveCount(); j ++)
        {
             PolygonObj* pObj(new PolygonObj(meshes[i], j));
             polygons.emplace_back(pObj);
        }
    }
    bbox.Enlarge(0.1, 0.1, 0.1);
    mMeshOrder.clear();
    for (int i = 0; i < meshes.size(); i++)
    {
        mMeshOrder[meshes[i]->GetID()] = i;
    }
    mpRoot = BuildTree(meshes, polygons, bbox, eIntered);
    polygons.clear();

}

class PolyObjTest {
    std::vector<PolygonObj*>& mPolys;
    const Box3* mChildBoxes;
    std::vector<PolygonObj*>* mXYZSplits;
    OctNodeType* mCellType;
    int* mMeshIds;
public :
    PolyObjTest(std::vector<PolygonObj*>& polys,  const Box3 childBoxes[],std::vector<PolygonObj*> XYZSplits[],  int meshIds[], OctNodeType cellType[]  )
       :mPolys(polys)
       ,mChildBoxes(childBoxes)
       ,mXYZSplits (XYZSplits)
       ,mMeshIds(meshIds)
       ,mCellType(cellType)
       
    {
    }
     void operator() (const tbb::blocked_range<int>& r) const ;
};

tbb::spin_mutex PolyObjTestMutex;

void PolyObjTest::operator() (const tbb::blocked_range<int>& r) const
{
    for (int i = r.begin(); i != r.end(); i++)
    {
        bool bClone =false ;
        bool  bMatched = false ; 
        for (int j = 0 ; j < 8 ; j++)
        {
            int  nIncluded = 0 ;
            if (mPolys[i]->IsInBox(mChildBoxes[j]))
            {
               assert(!bClone);
               nIncluded = 1; 
            }
            else if (mPolys[i]->IntersectWithBox(mChildBoxes[j]))
            {
                nIncluded = 2; 
            }
            if (nIncluded)
            {
                if (mCellType[j]  != eIntered)
                {
                   if (mMeshIds[j] == 0)
                       mMeshIds[j] = mPolys[i]->GetID();
                   else if (mMeshIds[j] != mPolys[i]->GetID())
                       mCellType[j] = eIntered;
                }
                bMatched = true ;
                tbb::spin_mutex::scoped_lock  lock(PolyObjTestMutex);
                {
                    if (!bClone)
                    {
                        mXYZSplits[j].emplace_back(mPolys[i]);
                        bClone = true;
                    }else 
                        mXYZSplits[j].emplace_back(mPolys[i]->Clone());
                }
                if (nIncluded == 1)
                    break;
            }
          
        }
        assert(bMatched);
    }
}

OctTreeNode* OctTree::BuildTree(std::vector<BaseMesh*>& meshes, std::vector< PolygonObj* >& polygons, Box3& bbox, OctNodeType NodeType)
{
    if (polygons.size() == 0)
        return nullptr;
    

    if(polygons.size() <= 17 || NodeType == eNormal || NodeType == eCritical)
    {
        OctLeafNode* pLeafNode = new OctLeafNode;
        pLeafNode->bbox      = bbox;
        pLeafNode->type = NodeType;
        pLeafNode->polygons  = polygons;
        for (int i = 0; i < pLeafNode->polygons.size(); i++ )
        {
            if (NodeType == eCritical || NodeType == eIntered)
                pLeafNode->polygons[i]->SetObjTypeFlag(NodeType);
        }
        return pLeafNode;
    } 

    OctTreeNode* pNode = new OctTreeNode;
    pNode->bbox      = bbox;
    pNode->type = NodeType;
        //split space by XYZ 
    Box3 childBoxes[8];
    SplitSpaceByXYZ(bbox, childBoxes);
    OctNodeType  CellsType[8] = {eNormal, eNormal, eNormal, eNormal, eNormal, eNormal, eNormal, eNormal};
   // DetermineCriticalCell(meshes, childBoxes, CellsType);
    std::vector< PolygonObj* > XYZSplits[8];
    int MeshIds[8];
    memset(MeshIds, 0 , 8*sizeof(int));
    PolyObjTest objTest(polygons, childBoxes,XYZSplits,MeshIds,CellsType);
   // tbb::parallel_for(tbb::blocked_range<int>(0,polygons.size(), 10000), objTest);
    for (int i = 0 ; i < polygons.size(); i ++)
    {
        bool bClone =false ;
        bool  bMatched = false ; 
        for (int j = 0 ; j < 8 ; j++)
        {
            int  nIncluded = 0 ;
            if (polygons[i]->IsInBox(childBoxes[j]))
            {
               assert(!bClone);
               nIncluded = 1; 
            }
            else if (polygons[i]->IntersectWithBox(childBoxes[j]))
            {
                nIncluded = 2; 
            }
            if (nIncluded)
            {
                if (CellsType[j]  != eIntered)
                {
                   if (MeshIds[j] == 0)
                       MeshIds[j] = polygons[i]->GetID();
                   else if (MeshIds[j] != polygons[i]->GetID())
                       CellsType[j] = eIntered;
                }
                bMatched = true ;
                if (!bClone)
                {
                    XYZSplits[j].emplace_back(polygons[i]);
                    bClone = true;
                }else 
                    XYZSplits[j].emplace_back(polygons[i]->Clone());
                if (nIncluded == 1)
                    break;
            }
          
        }
        assert(bMatched);
    }
    for (int i = 0; i < 8 ; i++)
    {
        if (CellsType[i] == eIntered)
            continue;
        for (int j = 0 ; j < meshes.size(); j++)
        {
            if (MeshIds[i] != meshes[j]->GetID())
            {
                if (childBoxes[i].IsWithinBox(meshes[j]->AABB()))
                {
                    CellsType[i] = eCritical;
                    break;
                }
            }
        }
    }
    int k ; 
    for ( k =0 ; k < 8 ; k ++)
    {
        if ( XYZSplits[k].size() < polygons.size())
            break;
    }
    if ( k>= 8)
        return pNode; 

    for (int i = 0; i < 8 ; i++)
    {
      /*  if (XYZSplits[i].size() >= polygons.size())
            pNode->child[i] = CreateLeafNode(bbox, CellsType[i], XYZSplits[i]);
        else */
            pNode->child[i] = BuildTree(meshes, XYZSplits[i], childBoxes[i], CellsType[i]);
        {
            XYZSplits[i].clear();
            std::vector<PolygonObj*>().swap(XYZSplits[i]);
        }
    }
    return pNode;
}

void OctTree::GenMeshesFromCells(std::vector<BaseMesh*>& meshes, int NodeType )
{
    std::vector<OctLeafNode*> leaves;
    GetLeafNodes(mpRoot, leaves, NodeType);
    if (leaves.size() == 0)
        return ; 
    meshes.resize(mMeshOrder.size());
    for (int i = 0 ; i < meshes.size(); i++)
         meshes[i] = NULL;
 
    std::hash_map<IndexPair, bool, IndexPairCompare> sharedObjMap;
    for(int i = 0 ; i < leaves.size(); i++)
    {
         for (int j = 0 ; j < leaves[i]->polygons.size(); j++)
         {
              PolygonObj* polyObj = leaves[i]->polygons[j];
             int index = mMeshOrder[polyObj->GetID()];
             if (meshes[index] == NULL)
             {
                 meshes[index] = new BaseMesh();
                 meshes[index]->GenID();
             }
             
             polyObj->AddPolygon(*meshes[index], sharedObjMap, leaves[i]->type);
           
         }
    }
    leaves.clear();
    sharedObjMap.clear();

}

void OctTree::GetLeafNodes(OctTreeNode* pNode, std::vector<OctLeafNode*>& leaves, int  NodeType)
{
    if (pNode == NULL)
        return; 
    OctNodeType type ;
    switch (pNode->type)
    {
        case eNormal:
            type = OctNodeType(NodeType& eNormal);
            break;
        case eCritical:
            type =  OctNodeType(NodeType& eCritical);
            break;
        case eIntered:
            type = OctNodeType(NodeType& eIntered);
            break;
        default :
            assert(0);
    }
    if (pNode->IsLeaf())
    {
        OctLeafNode* pLeafNode= static_cast<OctLeafNode*>(pNode);
        if (pLeafNode->polygons.size() == 0)
            return ; 
        if (type == pLeafNode->type)
            leaves.push_back(pLeafNode);
        return ;
    }
    if ((pNode->type == eNormal && type != pNode->type)||
        (pNode->type == eCritical) && (type != pNode->type) )
        return ;

    for ( int i = 0; i < 8 ; i++)
    {
        GetLeafNodes(pNode->child[i], leaves, NodeType);
    }
}


void OctTree::SplitSpaceByXYZ(const Box3& bbox,  Box3 childBoxes[])
{
    vec3<float> minOffset, maxOffset; 
    double3 step = double3(bbox.Diagonal().x* 0.5, bbox.Diagonal().y*0.5,  bbox.Diagonal().z*0.5);
    
    for (int i = 0; i < 8 ; i++)
    {
        maxOffset.z = i & 1 ?  0 : -step.z; 
        maxOffset.y = i & 2 ?  0 : -step.y;
        maxOffset.x = i & 4 ?  0 : -step.x;
        minOffset.z = i & 1 ?  step.z : 0; 
        minOffset.y = i & 2 ?  step.y : 0;
        minOffset.x = i & 4 ?  step.x : 0;
        childBoxes[i].Set(bbox.Min() + minOffset, bbox.Max()+ maxOffset);
    }

}



void OctTree::DetermineCriticalCell(std::vector<BaseMesh*>& meshes,const Box3 cells[], OctNodeType CellType[])
{
    
    for (int i = 0 ; i < 8 ; i++)
    {
        CellType[i]= eNormal;
        const Box3& box = cells[i];
        int nInterected = 0; 
        for (int j = 0; j < meshes.size(); j++)
        {
            if (box.Intersects(meshes[j]->AABB()))
                nInterected++; 
            if (nInterected >= 2)
            {
                CellType[i] = eCritical;
                break;
            }
        }
    }
}

void OctTree::CarveCriticalCelles(std::vector<CSGMesh*>& csgmeshes)
{
    std::vector<OctLeafNode*> leaves;
    GetLeafNodes(mpRoot, leaves, eIntered | eCritical);
    if (leaves.size() == 0)
        return ; 
    csgmeshes.resize(mMeshOrder.size());
    for (int i = 0 ; i < csgmeshes.size(); i++)
         csgmeshes[i] = NULL;
    std::map<int, std::vector<int> > surfaceMap;
    std::hash_map<IndexPair, bool, IndexPairCompare> sharedObjMap;
    for (int i = 0 ; i < leaves.size(); i++)
    {
        if (leaves[i]->polygons.size() == 0 )
                continue;
        if (leaves[i]->type == eCritical)
        {
            int start = -1; 
            int end ; 
            int index = mMeshOrder[leaves[i]->polygons[0]->GetID()];
            if (csgmeshes[index] == NULL)
            {
                    csgmeshes[index] = new CSGMesh;
                    csgmeshes[index]->GenID();
            }

            for (int k = 0 ; k < leaves[i]->polygons.size(); k++)
            {
                PolygonObj* polyObj = leaves[i]->polygons[k];
                int idx = polyObj->AddPolygon(*csgmeshes[index],sharedObjMap, leaves[i]->type);
                if (idx >= 0)
                {
                    if (start == -1 )
                        start = idx ;
                    end = idx;
                }
            }
            if (start >= 0)
                csgmeshes[index]->AddTriangesGroup(start, end);
        }else {
            for (int j = 0 ; j < leaves[i]->polygons.size(); j++)
            {
                 PolygonObj* polyObj = leaves[i]->polygons[j];
                 int index = mMeshOrder[polyObj->GetID()];
                 if (csgmeshes[index] == NULL)
                 {
                     csgmeshes[index] = new CSGMesh;
                     csgmeshes[index]->GenID();
                 }
              
                 int  meshId = polyObj->GetID();
                 int surfaceId  = csgmeshes[index]->AddSurface (polyObj->mpMesh, polyObj->mTriId);
                 if (surfaceId >=0)
                        surfaceMap[meshId].push_back( surfaceId);
  
            }
        }
        if (surfaceMap.size() >= 2)
        {
            std::vector<int> meshIdList(surfaceMap.size());
            std::map<int, std::vector<int> >::iterator it = surfaceMap.begin();
            for (int i = 0; it != surfaceMap.end(); i++,it++)
                meshIdList[i] = it->first;

            typedef std::pair<int, int> MeshIdPair;
            MeshIdPair testPair;
            std::map<MeshIdPair, bool> InterCarveMap;
            for(int k = 0 ; k < meshIdList.size()-1; k++)
            {
                for (int m = k+1; m < meshIdList.size(); m++)
                {
                    testPair.first = meshIdList[k];
                    testPair.second = meshIdList[m];
                    if (InterCarveMap.find(testPair) == InterCarveMap.end())
                        InterCarveMap[testPair] = true;
                }
            }
            meshIdList.clear();
            std::vector<int>().swap(meshIdList);
            std::map<MeshIdPair, bool>::iterator pairIter = InterCarveMap.begin();
            for (; pairIter !=InterCarveMap.end(); pairIter++ )
            {
                int meshId = pairIter->first.first;
                int othermeshId= pairIter->first.second;
                csgmeshes[mMeshOrder[meshId]]->InterCarve(surfaceMap[meshId],  *csgmeshes[mMeshOrder[othermeshId]],surfaceMap[othermeshId] ); 

            }
            InterCarveMap.clear();
        }
        surfaceMap.clear();
    }
    sharedObjMap.clear();
    leaves.clear();
}

void OctTree::RayCastingTest::operator() (OctLeafNode* pNode)
{
    if (pNode == NULL)
        return ;
    for (int i = 0 ; i < pNode->polygons.size(); i++)
    {
        PolygonObj* pObj = pNode->polygons[i];
        if (pObj->GetID() != nMeshId)
            continue;
        if (pObj->mpSharedData.use_count()> 1)
        {
            if (mTriMap.find(pObj->mTriId) != mTriMap.end())
                continue;
            mTriMap[pObj->mTriId] = true;
        }

        if (!pObj->AABB().IsInBox(pt, et))
            continue;
        int ret = pObj->RayFaceTest(pt, et, dir, splane);
        //Point on the face
        if (ret< 0 )
        {
            nCross = ret;
            return;  
        }
        else nCross ^= ret;
    }

}

PointMeshRelation OctTree::PointInPolyhedron(double3 pt, BaseMesh* pMesh)
{
    if (mpRoot == nullptr || !pMesh->AABB().IsInBox(pt))
        return pmrOutside;
    double3 r0 = mpRoot->bbox.Max()+double3(1.0, 1.0, 1.0);
    double3 dir = normalize(r0 - pt);
    double3 edge = r0 - double3(0.0, mpRoot->bbox.Size().y*0.5 , 0.0) - pt;
    double3 norm = normalize(cross(dir, edge));
    Plane<double> splane(norm, pt);
    RayCastingTest rc(pt, r0, dir, splane,  pMesh->GetID());
    RayTraverse(pt, dir, rc);
    //check return value 
    if (rc.nCross == -1 )
        return pmrSame;
    else if (rc.nCross == -2)
        return pmrOpposite;
    else if (rc.nCross == 1)
        return pmrInside;
    else return pmrOutside;

}

void OctTree::RayTraverse(double3 pt, double3 dir, RayCastingTest& func)
{
    if (mpRoot == nullptr)
         return ; 
    int a  =0; 
    if (dir.x < 0.0)
    {
        pt.x = mpRoot->bbox.Size().x - pt.x;
        dir.x = - dir.x;
        a |= 4;
    }
    if (dir.y < 0.0)
    {
        pt.y = mpRoot->bbox.Size().y - pt.y;
        dir.y = - dir.y;
        a |= 2;
    }
    if (dir.z < 0.0)
    {
        pt.z = mpRoot->bbox.Size().z - pt.z;
        dir.z = - dir.z;
        a |= 1;
    }
    double3 t0 = (mpRoot->bbox.Min() - pt) /dir;
    double3 t1 = (mpRoot->bbox.Max() - pt) /dir;
    if (max(t0.x, max(t0.y, t0.z)) < min(t1.x, min(t1.y, t1.z)))
        ProcessSubNode(t0.x, t0.y, t0.z, t1.x, t1.y, t1.z, mpRoot, a, func);
}

// Ray-Octree intersection based on
// 
// An Efficient Parametric Algorithm for Octree Traversal (2000)
// by J. Revelles , C. Ureña , M. Lastra
//
// Implementation by Jeroen Baert - www.forceflow.be - jeroen.baert@cs.kuleuven.be
//
// Licensed under a Creative Commons Attribute-NonCommercial Sharealike 3.0 Unported license
// which can be found at: http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// TL:DR? You must attribute me, cannot use this for commercial purposes, 
// and if you share this, it should be under the same or a similar license!
//
// Thanks,
//
// Jeroen
//

int OctTree::FindFirstNode(double tx0, double ty0, double tz0, double txm, double tym, double tzm)
{
    unsigned char answer = 0; // initialize to 00000000
    // select the entry plane and set bits
    if(tx0 > ty0){
        if(tx0 > tz0){ // PLANE YZ
            if(tym < tx0) answer|=2; // set bit at position 1
            if(tzm < tx0) answer|=1; // set bit at position 0 			
            return (int) answer; 
        } 
    } 
    else if(ty0 > tz0){ // PLANE XZ
        if(txm < ty0) answer|=4; // set bit at position 2
        if(tzm < ty0) answer|=1; // set bit at position 0
            return (int) answer;
       
    }
    // PLANE XY
    if(txm < tz0) answer|=4;   // set bit at position 2
    if(tym < tz0) answer|=2;   // set bit at position 1
    return (int) answer;
}
 
int OctTree::GetNextNode(double txm, int x, double tym, int y, double tzm, int z)
{
    if(txm < tym){
        if(txm < tzm){return x;}  // YZ plane
    }
    else{
        if(tym < tzm){return y;} // XZ plane
    }
    return z; // XY plane;
}

void OctTree::ProcessSubNode ( double tx0, double ty0, double tz0, double tx1, double ty1, double tz1, 
                               OctTreeNode* pNode, int& a, RayCastingTest& func)
{
    if (pNode == nullptr)
        return ; 
    float txm, tym, tzm;
    int currNode;
    if(tx1 < 0 || ty1 < 0 || tz1 < 0) 
        return; 
    if (pNode->IsLeaf())
    {
        OctLeafNode* pLeafNode = static_cast<OctLeafNode*>(pNode);
        func(pLeafNode);
        return ; 
    }
    txm = 0.5*(tx0 + tx1); 
    tym = 0.5*(ty0 + ty1); 
    tzm = 0.5*(tz0 + tz1); 
    currNode = FindFirstNode(tx0,ty0,tz0,txm,tym,tzm); 
    do{ 
        if (func.Terminal())
            return ; 
        switch (currNode) 
        { 
            case 0: {
                ProcessSubNode(tx0,ty0,tz0,txm,tym,tzm, pNode->child[a], a, func);
                currNode = GetNextNode(txm,4,tym,2,tzm,1);
                break;}
            case 1: {
                ProcessSubNode(tx0,ty0,tzm,txm,tym,tz1, pNode->child[1^a], a, func);
                currNode = GetNextNode(txm,5,tym,3,tz1,8);
                break;}
            case 2: {
                ProcessSubNode(tx0,tym,tz0,txm,ty1,tzm, pNode->child[2^a], a, func);
                currNode = GetNextNode(txm,6,ty1,8,tzm,3);
                break;}
           case 3: {
                ProcessSubNode(tx0,tym,tzm,txm,ty1,tz1, pNode->child[3^a], a, func);
                currNode = GetNextNode(txm,7,ty1,8,tz1,8);
                break;}
           case 4: {
                ProcessSubNode(txm,ty0,tz0,tx1,tym,tzm, pNode->child[4^a], a, func);
                currNode = GetNextNode(tx1,8,tym,6,tzm,5);
                break;}
          case 5: {
                ProcessSubNode(txm,ty0,tzm,tx1,tym,tz1, pNode->child[5^a], a, func);
                currNode = GetNextNode(tx1,8,tym,7,tz1,8);
                break;}
          case 6: {
                ProcessSubNode(txm,tym,tz0,tx1,ty1,tzm, pNode->child[6^a], a, func);
                currNode = GetNextNode(tx1,8,ty1,8,tzm,7);
                break;}
        case 7: {
                ProcessSubNode(txm,tym,tzm,tx1,ty1,tz1,pNode->child[7^a], a, func);
                currNode = 8;
                break;}
        }
    } while ( currNode < 8 );

}



void OctTree::CreateTreeGeometry(LineSet& segSet, const float4& color)
{
    if (mpRoot == nullptr)
        return ; 

    // draw current bounding-box
    float3 min = mpRoot->bbox.Min();
    float3 max = mpRoot->bbox.Max();
    // back face 
    segSet.AddLine(double3(min.x, min.y,min.z), double3(max.x, min.y, min.z), color);
    segSet.AddLine(double3(min.x, min.y,min.z), double3(min.x, min.y, max.z), color);
    segSet.AddLine(double3(min.x, min.y,max.z), double3(max.x, min.y, max.z), color);
    segSet.AddLine(double3(max.x, min.y, max.z), double3(max.x, min.y, min.z), color);
    // left face 
    segSet.AddLine(double3(min.x, min.y,min.z),  double3(min.x, max.y, min.z), color);
    segSet.AddLine(double3(min.x, max.y, min.z), double3(min.x, max.y, max.z), color);
    segSet.AddLine(double3(min.x, max.y, max.z),  double3(min.x, min.y, max.z), color);
  

    //front face 
    segSet.AddLine(double3(min.x, max.y,min.z), double3(max.x, max.y, min.z), color);
    segSet.AddLine(double3(max.x, max.y,min.z), double3(max.x, max.y, max.z), color);
    segSet.AddLine(double3(max.x, max.y,max.z), double3(min.x, max.y, max.z), color);
    // right face
    segSet.AddLine(double3(max.x, max.y,min.z), double3(max.x, min.y, min.z), color);
    segSet.AddLine(double3(max.x, max.y,max.z), double3(max.x, min.y, max.z), color);
    //get the bounding of current bounding box 
    FillSplittingLine(mpRoot, segSet, color);
     
}

void OctTree::FillSplittingLine(OctTreeNode* pNode, LineSet& segSet, const float4& color)
{
    if (pNode == nullptr)
        return ; 
    if (pNode->IsLeaf())
        return ;
    float3 min = pNode->bbox.Min();
    float3 max = pNode->bbox.Max();
    // draw x-axis seperate  line
    float y = (min.y + max.y)*0.5;
    segSet.AddLine(double3(min.x, y,min.z), double3(max.x, y, min.z), color);
    segSet.AddLine(double3(max.x, y,min.z), double3(max.x, y, max.z), color);
    segSet.AddLine(double3(max.x, y, max.z), double3(min.x, y, max.z), color);
    segSet.AddLine(double3(min.x, y, max.z), double3(min.x, y,min.z), color);

    // draw y-axis seperate line 
    float x = (min.x + max.x)*0.5;
    segSet.AddLine(double3(x, max.y,min.z), double3(x, min.y, min.z), color);
    segSet.AddLine(double3(x, min.y, min.z), double3(x, min.y, max.z), color);
    segSet.AddLine(double3(x, min.y,max.z), double3(x, max.y, max.z), color);
    segSet.AddLine(double3(x, max.y, max.z), double3(x, max.y, min.z), color);

    // draw z-axis seperate line 
    float z = (min.z+ max.z)*0.5;
    segSet.AddLine(double3(min.x, max.y, z), double3(max.x, max.y, z), color);
    segSet.AddLine(double3(max.x, max.y, z), double3(max.x, min.y, z), color);
    segSet.AddLine(double3(max.x, min.y, z), double3(min.x, min.y, z), color);
    segSet.AddLine(double3(min.x, min.y, z), double3(min.x, max.y, z), color);

    for (int i = 0 ; i < 8 ; i++)
        FillSplittingLine(pNode->child[i], segSet, color);
}

}