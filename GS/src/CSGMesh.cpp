#include "CSGMesh.h"
#include <assert.h>

#include "tbb\tick_count.h"
namespace GS{


CSGMesh::CSGMesh(bool bUseUniqueVertex )
	:BaseMesh(bUseUniqueVertex)
{
    mTriGroupRec.clear();
    mSurfacesMap.clear();
}


CSGMesh::CSGMesh(BaseMesh* pMesh)
	:BaseMesh(*pMesh)
{
	CSGMesh* pCSGMesh = dynamic_cast<CSGMesh*>(pMesh);
	if (pCSGMesh)
	{
		mInterCarveMeshID = pCSGMesh->mInterCarveMeshID;
		mTriMeshRelation = pCSGMesh->mTriMeshRelation;
        mTriGroupRec= pCSGMesh->mTriGroupRec;
        mSurfacesMap = pCSGMesh->mSurfacesMap;
	}

}

CSGMesh::~CSGMesh()
{
	mInterCarveMeshID.clear();
	mTriMeshRelation.clear();
    mSurfacesMap.clear();
    mTriGroupRec.clear();
}

int CSGMesh::AddSurface(const BaseMesh* pMesh, int triId)
{
    IndexPair pair;
    int ids[2];
    ids[0] = pMesh->GetID();
    ids[1] = triId;
    MakeIndex(ids, pair);
    if (mSurfacesMap.find(pair) != mSurfacesMap.end())
        return mSurfacesMap[pair];
    int NewTriId = AddTriangle(pMesh, triId);  
    assert(NewTriId >=0 );
    std::vector<int> triIds(1);
    triIds[0]=NewTriId;
    Surface<double>* pSurface = new Surface<double>(this, triIds);
    pSurface->GenerateOuterLineByTri();
    pSurface->GenerateAABB();
    mSurfaceList.push_back(pSurface);
    triIds.clear();
    mSurfacesMap[pair] = mSurfaceList.size()-1;
    return mSurfaceList.size()-1;

}

int  CSGMesh::AddSurface(const Polygon& poly)
{
    ListOfvertices results; 
    poly.Trianglate(results);
    std::vector<int> triIds(results.size()/3);
    
    for (int k = 0 ; k < results.size(); k+=3)
    {   
        int triId = Add(results[k], results[k+1], results[k+2],  poly.normal);
        if (triId == -1)
            continue;
        triIds.push_back(triId);
    }
    results.clear();
    ListOfvertices().swap(results);
    if (triIds.size() == 0)
        return -1 ; 
    Surface<double>* pSurface = new Surface<double>(this, triIds);
    pSurface->GenerateOuterLineByTri();
    pSurface->GenerateAABB();
    mSurfaceList.push_back(pSurface);
    triIds.clear();
    std::vector<int>().swap(triIds);
    return mSurfaceList.size()-1;
}


bool CSGMesh::InterCarve(CSGMesh& mesh)
{
    if (!AABB().Intersects(mesh.AABB()))
		return false ;
	bool bResult = false ;
	for (int i = 0 ; i < mSurfaceList.size(); i++)
	{
		for (int j = 0 ; j < mesh.mSurfaceList.size(); j++)
		{
			if(!mSurfaceList[i]->AABB().Intersects(mesh.mSurfaceList[j]->AABB()))
				continue;
			for (int m = 0 ; m < mSurfaceList[i]->TriangleCount(); m++)
			{
				int  TriAId = mSurfaceList[i]->GetTriangleId(m);
				for (int k = 0 ; k < mesh.mSurfaceList[j]->TriangleCount(); k++)
				{
					std::vector<Seg3D<double> > intersects;
					int TriBId = mesh.mSurfaceList[j]->GetTriangleId(k);
 					bool bCarved = TrianglesIntersectTest(TriAId, mesh,TriBId, intersects);
					if (bCarved)
					{
						for ( int s = 0 ; s < intersects.size(); s++)
						{
							mSurfaceList[i]->AddConstraint(intersects[s].start, intersects[s].end, mesh.GetID());
							AddCarveMeshID(mesh.GetID());
							mesh.mSurfaceList[j]->AddConstraint(intersects[s].start, intersects[s].end, GetID());
							mesh.AddCarveMeshID(GetID());
							bResult = true;
						}
					}
					intersects.clear();
				}
			}

		}
	}
	return bResult;


}


bool CSGMesh::InterCarve(const std::vector<int>& surfaceIds, CSGMesh& mesh, const std::vector<int>& otherSurfaceIds)
{
    bool bResult = false ;
    for (int i = 0 ; i < surfaceIds.size(); i++)
	{
        Surface<double>* pSurface = mSurfaceList[surfaceIds[i]];
		for (int j = 0 ; j < otherSurfaceIds.size(); j++)
		{
            Surface<double>* pOtherSurface= mesh.mSurfaceList[otherSurfaceIds[j]];
            if(!pSurface->AABB().Intersects(pOtherSurface->AABB()))
				continue;
            for (int m = 0 ; m < pSurface->TriangleCount(); m++)
			{
				int  TriAId = pSurface->GetTriangleId(m);
				for (int k = 0 ; k < pOtherSurface->TriangleCount(); k++)
				{
					std::vector<Seg3D<double> > intersects;
					int TriBId = pOtherSurface->GetTriangleId(k);
 					bool bCarved = TrianglesIntersectTest(TriAId, mesh,TriBId, intersects);
					if (bCarved)
					{
						for ( int s = 0 ; s < intersects.size(); s++)
						{
							pSurface->AddConstraint(intersects[s].start, intersects[s].end, mesh.GetID());
							AddCarveMeshID(mesh.GetID());
							pOtherSurface->AddConstraint(intersects[s].start, intersects[s].end, GetID());
							mesh.AddCarveMeshID(GetID());
							bResult = true;
						}
					}
					intersects.clear();
				}
			}
        }


    }
    return bResult;
}

void CSGMesh::Uncarve(int MeshId)
{

}

CSGMesh* CSGMesh::TriangulateByCarve(bool UseUniqueVertex)
{

  //TODO: 这里用不用唯一定点约束，值得考量
	CSGMesh* TriMesh = new CSGMesh(UseUniqueVertex) ;
    TriMesh->SetID(GetID());
    TriMesh->SetAABB(AABB());
	ListOfvertices vertices;
    int start = -1; 
    int end; 


	for (int i = 0; i < mSurfaceList.size(); i++)
	{
		if (!mSurfaceList[i]->Triangulated())
		{
			vertices.clear();
			mSurfaceList[i]->Triangulate(vertices);
			for (int k = 0 ; k < vertices.size(); k+=3)
			{   
			    int idx  = TriMesh->Add(vertices[k], vertices[k+1], vertices[k+2], mSurfaceList[i]->N());
                if (idx >=0)
                {
                    if (start == -1)
                        start = idx;
                    end = idx;
                }
			}
		}else {

			for(int j = 0; j< mSurfaceList[i]->TriangleCount(); j++)
			{
				int TriId = mSurfaceList[i]->GetTriangleId(j);
				int idx = TriMesh->AddTriangle(this, TriId);
                if (idx >=0)
                {
                    if (start == -1)
                        start = idx;
                    end = idx;
                }
			}

		}
	}
     if (start >=0)
            TriMesh->AddTriangesGroup(start, end, false );
    for (int i = 0 ; i < mTriGroupRec.size(); i++)
    {
        GroupRec& group = mTriGroupRec[i];
        start = -1; 
        end; 
        for (int j = group.start; j <= group.end; j++)
        {
           int triId =  TriMesh->AddTriangle(this, j);
           if (triId >=0)
           {
               if (start == -1)
                start = triId;
               end = triId;
           }
        }
        if (start >=0)
            TriMesh->AddTriangesGroup(start, end, group.bUnified );
       
    }
    return TriMesh; 
}

inline
PointMeshRelation CSGMesh::PointRelatedWithPlane(const Point3D& TriPos,const Vec3D& TriN,
		                                         const Point3D& p, const Point3D& n)
{
	double DotDist = dot((TriPos - p), n);
	if (IsEqual(fabs(DotDist), 0. , (double)EPSF /*(double)PLANE_THICK_EPS*/))
	{
		if (dot(TriN, n) > 0) 
			return pmrSame;
		else
			return pmrOpposite;
	}else if (DotDist > 0 )    
		return pmrInside;
	return pmrOutside;
}

TriMeshRelation CSGMesh::RelatedWith(int TriId, BaseMesh* mesh )
{
    if (!AABB().Intersects(mesh->AABB()))
		return tmrOutside;

	PointMeshRelation Relation;
	if (mesh->GetManifordType() == mtRegular)//正则流形
			Relation = mesh->RelationWith(CenterOfGravity(TriId),TriangleInfo(TriId).Normal);
	else //平面，上部为inside, 下部为outside
	{
		const Vec3D& n = mesh->TriangleInfo(0).Normal;
		Vec3D p = mesh->Vertex(mesh->TriangleInfo(0).VertexId[0]).pos;
		Relation = PointRelatedWithPlane(CenterOfGravity(TriId),TriangleInfo(TriId).Normal, p, n);
	}
	switch( Relation)
	{
			case pmrOutside: return tmrOutside;
			case pmrInside:   return tmrInside;
			case pmrSame:     return tmrSame;
			case pmrOpposite: return tmrOpposite;
			default:
			{
				assert(0);
				return tmrUnknown;
			}
	}
 
}


//NOTE: 注意这里的判定方法是依据重心坐标，这有可能在稳定性上有所欠缺
void CSGMesh::RelatedWith(BaseMesh* mesh)
{
	if (mTriMeshRelation.size() ==0)
        mTriMeshRelation.resize(PrimitiveCount());
    if (!AABB().Intersects(mesh->AABB()))
		return ;
	const Vec3D& n = mesh->TriangleInfo(0).Normal;
	Vec3D p = mesh->Vertex(mesh->TriangleInfo(0).VertexId[0]).pos;
	for (int i = 0 ; i< PrimitiveCount(); i++)
	{
		PointMeshRelation Relation;
		if (mesh->GetManifordType() == mtRegular)//正则流形
			Relation = mesh->RelationWith(CenterOfGravity(i),TriangleInfo(i).Normal);
		else //平面，上部为inside, 下部为outside
			Relation = PointRelatedWithPlane(CenterOfGravity(i),TriangleInfo(i).Normal, p, n);
		TriMeshRelation TriRelation;
		switch( Relation)
		{
			case  pmrInside: 
				TriRelation = tmrInside;
				break;
			case pmrSame: 
				TriRelation = tmrSame;
				break;
            case pmrOpposite:
				TriRelation = tmrOpposite;
				break;
			default:
				continue;
		}
		AddRelation(i, mesh->GetID(), TriRelation);
	}	
}




void CSGMesh::AddCarveMeshID(int ID)
{
	if (std::find(mInterCarveMeshID.begin(), mInterCarveMeshID.end(), ID) == mInterCarveMeshID.end())
		mInterCarveMeshID.push_back(ID);
}

void CSGMesh::AddRelation(int TriId, int RelatedMeshID, TriMeshRelation Relation)
{
	ListOfRelationRec& RecList = mTriMeshRelation[TriId];
	for (int i = 0; i < RecList.size(); i++)
	{
		if (RecList[i].MeshID == RelatedMeshID)
		{
			assert(Relation == RecList[i].Relation);
			return ;
		}
	}
    RelationRec Rec;
    Rec.MeshID = RelatedMeshID;
    Rec.Relation = Relation;
	RecList.push_back(Rec);
}

CSGMesh* CSGMesh::ForceTriangulate()
{
	std::vector<bool> Constraint(mSurfaceList.size());
	for (int i = 0; i< mSurfaceList.size(); i++)
	{
		Constraint.push_back(mSurfaceList[i]->HasConstraint());
		mSurfaceList[i]->SetConstraint(true);
		mSurfaceList[i]->SetTriangulateMethod(tmOld);
	}
	CSGMesh* pMesh =TriangulateByCarve(true); 
	for (int i = 0; i< mSurfaceList.size(); i++)
	{
		
		mSurfaceList[i]->SetConstraint(Constraint[i]);
		mSurfaceList[i]->SetTriangulateMethod(tmNew);
	}
	Constraint.clear();
	return pMesh;
}

 bool    CSGMesh::AddTriangesGroup(int start, int end, bool unified)
 {
     assert (start <= end);
     if (end >= mpMeshImp->mTriangle.size())
         return false ; 
     GroupRec group;
     group.bUnified = unified;
     group.start = start;
     group.end = end;
     mTriGroupRec.push_back(group);
     return true;

 }


}
