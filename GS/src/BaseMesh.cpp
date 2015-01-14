#define _USE_MATH_DEFINES 
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <assert.h>
#include <hash_map>
#include "BaseMesh.h"
#include "topology.h"
#include "adaptive.h"
#include "tbb\parallel_for.h"
#include "tbb\tick_count.h"


namespace GS{

static inline double3 normalize(double3 &v, float3 &center, float3 &scale)
{


	return (v-center)/scale*2.0;
}

static inline double3 denormalize(double3 &v, float3 &center, float3 &scale)
{
	return v/2.0*scale+center;
}

BaseMeshImp::BaseMeshImp(bool bUseUniqueVertex)
	:mID(0)
	,mbUniqueVertex(bUseUniqueVertex)
	,mPrimitiveCapacity(INIT_PRIMITIVE_CAPACITY)
	,mVertexCapacity(INIT_VERTEX_CAPACITY)
	,mpVB(NULL)
	,mpIB(NULL)
    ,mRefCount(1)
    ,m_bCoordNormalized(false)
{
	mVertex.reserve(mVertexCapacity);
	mNormal.reserve(mVertexCapacity);
	mColorIdx.reserve(mVertexCapacity);
	mTriangle.reserve(mPrimitiveCapacity);
	mTriAABBList.reserve(mPrimitiveCapacity);
}

BaseMeshImp::BaseMeshImp(const BaseMeshImp& mesh)
	:mID(mesh.mID)
	,mbUniqueVertex(mesh.mbUniqueVertex)
	,mPrimitiveCapacity(mesh.mPrimitiveCapacity)
	,mVertexCapacity(mesh.mVertexCapacity)
    ,mAABB(mesh.mAABB)
	,mNormal(mesh.mNormal)
	,mColorIdx(mesh.mColorIdx)
    ,mColorTable(mesh.mColorTable)
	,mVertex(mesh.mVertex)
	,mTriangle(mesh.mTriangle)
	,mTriAABBList(mesh.mTriAABBList)
	,mVertexDict(mesh.mVertexDict)
	,mpVB(NULL)
	,mpIB(NULL)
    ,mRefCount(1)
    ,m_bCoordNormalized(false)
{
    
	
   
}


BaseMeshImp::~BaseMeshImp()
{
	mCenterOfGravity.clear();
	mVertex.clear();
	mNormal.clear();
	mColorIdx.clear();
    mColorTable.clear();
	mVertexDict.clear();
    mTriAABBList.clear();
    mTriangle.clear();
	if (mpVB)
	{
		delete mpVB;
		mpVB = NULL;
	}
	if (mpIB)
	{
		delete mpIB;
		mpIB = NULL;
	}

}

inline
void  BaseMesh::ClearSurfaces()
{
    for(int i = 0 ; i < mSurfaceList.size(); i++)
    {
        delete mSurfaceList[i];
    }
    mSurfaceList.clear();
}

void BaseMeshImp::GenCenterOfGravity()
{
    mCenterOfGravity.clear();
    mCenterOfGravity.reserve(PrimitiveCount());
    for (int i = 0 ; i< PrimitiveCount(); i++)
    {
        TriInfo& triInfo = mTriangle[i];
        mCenterOfGravity.push_back(GS::CenterOfGravity(mVertex[triInfo.VertexId[0]], 
                                mVertex[triInfo.VertexId[1]], mVertex[triInfo.VertexId[2]]));
    }
}



VertexInfo BaseMeshImp::Vertex(int idx) const
{
	assert(idx < mVertex.size());
	VertexInfo vx;
	vx.pos = mVertex[idx];
	vx.normal = mNormal[idx];
	vx.color = mColorTable[mColorIdx[idx]];
	return vx;
}

//void BaseMesh::Add(const Point3D& p1, const Point3D& p2, const Point3D& p3)
//{
//	Vec3D n= GS::NormalOf(p1, p2, p3);
//	int vId1 = AddVertex(p1);
//	int vId2 = AddVertex(p2);
//	int vId3 = AddVertex(p3);
//	AddTriangle(vId1, vId2, vId3, n);
//}

int  BaseMeshImp::Add(const VertexInfo& v1, const VertexInfo& v2, const VertexInfo& v3)
{
    Vec3D n= GS::NormalOf(v1.pos, v2.pos, v3.pos);
    int vId1 = AddVertex(v1);
    int vId2 = AddVertex(v2);
    int vId3 = AddVertex(v3);
    return AddTriangle(vId1, vId2, vId3, n);
 
}

int	BaseMeshImp:: AddVertex(const VertexInfo& v)
{
	int vertexIdx = -1; 
	std::hash_map<Point3D, int,Vec3HashCompare<double> >::iterator it  = mVertexDict.find (v.pos);
    if (mbUniqueVertex && it != mVertexDict.end())
		vertexIdx = mVertexDict[v.pos];
	if(!mbUniqueVertex || vertexIdx == -1)
	{
		vertexIdx = mVertex.size();
		CheckVertexCapacity();
		mVertex.push_back(v.pos);
		mNormal.push_back(v.normal);
        int idx = -1; 
        for (int i = 0; i < mColorTable.size(); i++)
        {
            if (vec4_equal(mColorTable[i], v.color))
            {
                idx = i; 
                break;
            }
        }
        if (idx == -1)
        {
		    mColorTable.push_back(v.color);
            idx= mColorTable.size() -1;
        }
        mColorIdx.push_back(idx);
		if (mbUniqueVertex)
			mVertexDict[v.pos] = vertexIdx;
	}
	return vertexIdx;
}

//int	BaseMesh:: AddVertex(const Point3D& point)
//{
//	int vertexIdx = -1; 
//	std::map<Point3D, int, Vec3LessThan<double>>::iterator it  = mVertexDict.find (point);
//    if (mbUniqueVertex && it != mVertexDict.end())
//		vertexIdx = mVertexDict[point];
//	if(!mbUniqueVertex || vertexIdx == -1)
//	{
//		vertexIdx = mVertex.size();
//		CheckVertexCapacity();
//		mVertex.push_back(point);
//		if (mbUniqueVertex)
//			mVertexDict[point] = vertexIdx;
//	}
//	return vertexIdx;
//}
	  

int  BaseMeshImp::AddTriangle(int vId1, int vId2, int vId3, const Vec3D& normal)
{
	if (vId1 == vId2|| vId1 == vId3 || vId2 == vId3)
		return -1 ;
	assert(vId1 >=0 && vId2 >=0 && vId3 >=0);
    CheckPrimitiveCapacity();
	TriInfo triInfo;
	triInfo.Normal = normal;
	triInfo.VertexId[0] = vId1;
	triInfo.VertexId[1] = vId2;
	triInfo.VertexId[2] = vId3;
	mTriangle.push_back(triInfo);
    Box3 TriAABB; 
	TriAABB.IncludePoint(mVertex[vId1]);
	TriAABB.IncludePoint(mVertex[vId2]);
	TriAABB.IncludePoint(mVertex[vId3]);
    mTriAABBList.push_back(TriAABB);
    return mTriangle.size()-1;
}

int  BaseMeshImp::AddTriangle(const BaseMeshImp* pMeshImp, int TriId)
{
	assert(TriId < pMeshImp->PrimitiveCount());
    CheckPrimitiveCapacity();
	int vId1 = AddVertex(pMeshImp->Vertex(pMeshImp->mTriangle[TriId].VertexId[0]));
	int vId2 = AddVertex(pMeshImp->Vertex(pMeshImp->mTriangle[TriId].VertexId[1]));
	int vId3 = AddVertex(pMeshImp->Vertex(pMeshImp->mTriangle[TriId].VertexId[2]));
	return AddTriangle(vId1, vId2, vId3, pMeshImp->mTriangle[TriId].Normal);

}

void BaseMeshImp::CheckPrimitiveCapacity()
{
	if (PrimitiveCount() >=  mPrimitiveCapacity)
	{
		mPrimitiveCapacity*=2;
		mTriangle.reserve(mPrimitiveCapacity);
        mTriAABBList.reserve(mPrimitiveCapacity);
	}
}

void BaseMeshImp::CheckVertexCapacity()
{
 
	if ( mVertex.size() >= mVertexCapacity)
	{
		mVertexCapacity *=2;
		mVertex.reserve(mVertexCapacity);
		mNormal.reserve(mVertexCapacity);
		mColorIdx.reserve(mVertexCapacity);
	}
}


Point3D  BaseMeshImp::CenterOfGravity (int idx) const
{
	assert(idx < mCenterOfGravity.size());
	return mCenterOfGravity[idx];
}

void BaseMeshImp::GetMesh(ListOf3DPoints& Triangles)
{
	Triangles.reserve(mTriangle.size()*3);
	for(int i = 0 ; i< mTriangle.size(); i++)
	{
		Triangles.push_back(mVertex[mTriangle[i].VertexId[0]]);
    	Triangles.push_back(mVertex[mTriangle[i].VertexId[1]]);
		Triangles.push_back(mVertex[mTriangle[i].VertexId[2]]);
	}
}

void  BaseMeshImp::GenAABB(bool bNeedTriAABB)
{
	//if (bNeedTriAABB)
	//{
	//	mTriAABBList.clear();
 //      // tbb::tick_count t0 = tbb::tick_count::now();
 //       mTriAABBList.reserve(mTriangle.size());
	//	for (int i = 0; i< mTriangle.size(); i++ )
	//	{
	//		Box3 TriAABB; 
	//		TriAABB.IncludePoint(mVertex[mTriangle[i].VertexId[0]]);
	//		TriAABB.IncludePoint(mVertex[mTriangle[i].VertexId[1]]);
	//		TriAABB.IncludePoint(mVertex[mTriangle[i].VertexId[2]]);
 //           mTriAABBList.push_back(TriAABB);
	//	}
 // 
	//}
	mAABB.Clear();
	for (int i = 0; i< mVertex.size(); i++)  
		mAABB.IncludePoint(mVertex[i]);
}


void  BaseMesh::GenSurface()
{
	  
	std::hash_map<Plane<double>, std::vector<int>, PlaneHashCompare<double> > Partition;
	for (int i = 0 ; i < PrimitiveCount(); i++)
	{ 
        const TriInfo& triInfo = TriangleInfo(i);
		/*Plane<double> plane;
		plane.pos = mpMeshImp->mVertex[triInfo.VertexId[0]];
		plane.normal = triInfo.Normal;*/
        Plane<double> plane(triInfo.Normal, mpMeshImp->mVertex[triInfo.VertexId[0]]);
		Partition[plane].push_back(i);
	}
	ClearSurfaces();

	std::hash_map<Plane<double>, std::vector<int>, PlaneHashCompare<double> >::iterator iter ;
    mSurfaceList.reserve(Partition.size());
    
	for (iter = Partition.begin(); iter != Partition.end(); iter++)
	{
		
		SurfacePtr pSurface = new Surface<double>(this, iter->second);
		if (pSurface->GenerateOuterLineByTri())
			mSurfaceList.push_back(pSurface);
		else 
	       	delete pSurface;

	}
    Partition.clear();

	for (int i = 0 ; i<mSurfaceList.size(); i++)
		mSurfaceList[i]->GenerateAABB();

  
	
}
		

 
const TriInfo&  BaseMeshImp::TriangleInfo(int idx) const
{
	assert(idx < mTriangle.size());
	return mTriangle[idx];
}

//void BaseMeshImp::PointRelationTest::operator()(const tbb::blocked_range<int>&r)
//{
//    for (int i = r.begin() ; i !=r.end(); i++)
//    {
//        Point3D& p0 = mpMesh->mVertex[mpMesh->mTriangle[i].VertexId[0]];
//		Point3D& p1 = mpMesh->mVertex[mpMesh->mTriangle[i].VertexId[1]];
//		Point3D& p2 = mpMesh->mVertex[mpMesh->mTriangle[i].VertexId[2]];
//		double PointToPlaneDist = dot(mpMesh->mTriangle[i].Normal, (mPos-p0));
//		if (!IsEqual(PointToPlaneDist, 0. ,(double)EPSF ))
//		{
//			double Tri_Area = 0;
//			Vec3D v0 = p0 - mPos;
//            Vec3D v1 = p1 - mPos;
//            Vec3D v2 = p2 - mPos;
//            Vec3D n01 = normalize(cross(v0, v1));
//			Vec3D n12 = normalize(cross(v1, v2));
//			Vec3D n20 = normalize(cross(v2, v0));
//			assert(! vec3_equal(n01, 0)&&
//					! vec3_equal(n12, 0)&&
//					! vec3_equal(n20, 0));
// 
//			Tri_Area  = M_PI*2 - std::acos(dot(n01, n12)) - std::acos(dot(n12, n20)) - std::acos(dot(n20, n01));
//			if (PointToPlaneDist < 0 )
//				Tri_Area = -Tri_Area;
//			 mArea += Tri_Area;
//
//		}
//    }
//
//}

//PointMeshRelation BaseMeshImp::RelationWith(const Point3D& pos, const Vec3D& dir )
//{
//	if(!mAABB.IsInBox(pos))
//		return pmrOutside;
//	PointMeshRelation pr= CoplanarRelation(pos, dir);
//	if(pr != pmrUnknown)
//		return pr; 
//	return RelationWithPoint(pos, dir);
//
//}
//
//PointMeshRelation BaseMeshImp::CoplanarRelation(const Point3D& pos, const Vec3D& dir )
//{
//	for (int i =0 ; i< mTriangle.size(); i++)
//	{
//		Point3D& p0 = mVertex[mTriangle[i].VertexId[0]];
//		Point3D& p1 = mVertex[mTriangle[i].VertexId[1]];
//		Point3D& p2 = mVertex[mTriangle[i].VertexId[2]];
//		double PointToPlaneDist = dot(mTriangle[i].Normal, (pos-p0));
//		if (IsEqual(PointToPlaneDist, 0. ,(double)EPSF ))
//		{
//			  if (PointInTriangle(pos, p0, p1, p2, mTriangle[i].Normal))
//			  {
//					if (dot(dir, mTriangle[i].Normal) < -EPSF) 
//						return pmrOpposite;
//					else return pmrSame;
//			  }
//		}
//	}
//	return pmrUnknown;
//}
//
//PointMeshRelation BaseMeshImp::RelationWithPoint(const Point3D& pos, const Vec3D& dir )
//{
//	/*double Area = 0.;
//	for (int i =0 ; i< mTriangle.size(); i++)
//	{
//		Point3D& p0 = mVertex[mTriangle[i].VertexId[0]];
//		Point3D& p1 = mVertex[mTriangle[i].VertexId[1]];
//		Point3D& p2 = mVertex[mTriangle[i].VertexId[2]];
//		double PointToPlaneDist = dot(mTriangle[i].Normal, (pos-p0));
//		if (!IsEqual(PointToPlaneDist, 0. ,(double)EPSF ))
//		{
//			double Tri_Area = 0;
//			Vec3D v0 = p0 - pos;
//            Vec3D v1 = p1 - pos;
//            Vec3D v2 = p2 - pos;
//            Vec3D n01 = normalize(cross(v0, v1));
//			Vec3D n12 = normalize(cross(v1, v2));
//			Vec3D n20 = normalize(cross(v2, v0));
//			assert(! vec3_equal(n01, 0)&&
//					! vec3_equal(n12, 0)&&
//					! vec3_equal(n20, 0));
// 
//			Tri_Area  = M_PI*2 - std::acos(dot(n01, n12)) - std::acos(dot(n12, n20)) - std::acos(dot(n20, n01));
//			if (PointToPlaneDist < 0 )
//				Tri_Area = -Tri_Area;
//			 Area += Tri_Area;
//
//		}
//	}*/
//    PointRelationTest pr(this, pos);
//    tbb::parallel_reduce(tbb::blocked_range<int>(0, mTriangle.size(), 32), pr );
//    if (fabs(pr.mArea) >2*M_PI)
//		return pmrInside;
//    else
//        return  pmrOutside;
//}

PointMeshRelation BaseMeshImp::RelationWith(const Point3D& pos, const Vec3D& dir )
{
	if(!mAABB.IsInBox(pos))
		return pmrOutside;
	double Area = 0;
	for (int i =0 ; i< mTriangle.size(); i++)
	{
		Point3D& p0 = mVertex[mTriangle[i].VertexId[0]];
		Point3D& p1 = mVertex[mTriangle[i].VertexId[1]];
		Point3D& p2 = mVertex[mTriangle[i].VertexId[2]];
		double PointToPlaneDist = dot(mTriangle[i].Normal, (pos-p0));
        double Tri_Area = 0;
		if (IsEqual(PointToPlaneDist, 0. ,(double)EPSF ))
		{
			  if (PointInTriangle(pos, p0, p1, p2, mTriangle[i].Normal))
			  {
					if (dot(dir, mTriangle[i].Normal) < -EPSF) 
						return pmrOpposite;
					else return pmrSame;
			  }
		}
		else {
                Vec3D v0 = p0 - pos;
                Vec3D v1 = p1 - pos;
                Vec3D v2 = p2 - pos;
                Vec3D n01 = normalize(cross(v0, v1));
				Vec3D n12 = normalize(cross(v1, v2));
				Vec3D n20 = normalize(cross(v2, v0));
				assert(! vec3_equal(n01, 0)&&
					   ! vec3_equal(n12, 0)&&
					   ! vec3_equal(n20, 0));
 
				Tri_Area  = M_PI*2 - std::acos(dot(n01, n12)) - std::acos(dot(n12, n20)) - std::acos(dot(n20, n01));
				if (PointToPlaneDist < 0 )
					Tri_Area = -Tri_Area;
		 }
	     Area += Tri_Area;

	}
	 //有向面积为0在外部，否则（理论值为+-4PI）在内部。实际取2PI相当于加大了一些误差范围

    if (fabs(Area) >2*M_PI)
		return pmrInside;
    else
        return  pmrOutside;
}

bool  BaseMeshImp::TrianglesIntersectTest(int TriId, const BaseMeshImp& meshB, int MeshBTriId, 
			                          std::vector<Seg3D<double> >& intersects)
{
	if (!mTriAABBList[TriId].Intersects(meshB.mTriAABBList[MeshBTriId]))
		return false ;
	const Point3D& p0 = mVertex[mTriangle[TriId].VertexId[0]];
	const Point3D& p1 = mVertex[mTriangle[TriId].VertexId[1]];
	const Point3D& p2 = mVertex[mTriangle[TriId].VertexId[2]];
    const Point3D& v0 = meshB.mVertex[meshB.mTriangle[MeshBTriId].VertexId[0]];
	const Point3D& v1 = meshB.mVertex[meshB.mTriangle[MeshBTriId].VertexId[1]];
	const Point3D& v2 = meshB.mVertex[meshB.mTriangle[MeshBTriId].VertexId[2]];
	return TriangleInterTriangle(p0, p1, p2, mTriangle[TriId].Normal, 
		                         v0, v1, v2, meshB.mTriangle[MeshBTriId].Normal,   
								 intersects);
}

int BaseMeshImp::GetVertexStride() const 
{
	return sizeof(float3)+ sizeof(float4)+sizeof(float3);
	
}

 std::hash_map<int, bool> BaseMesh::mMeshIDSet;
void BaseMesh::GenID()
{
	static bool bFirstTime = false;
	if (!bFirstTime)
	{
		srand( (unsigned)time( NULL ) );
		bFirstTime = true;
	}
    for(;;)
    {
        int id = rand()+1;
        if (mMeshIDSet.find(id )!= mMeshIDSet.end())
            continue;

	    mpMeshImp->mID =  id;
        mMeshIDSet[id] = true;
        break;
    }
}



void BaseMeshImp::FillD3DBuffer(ID3D11DeviceContext* dc,D3D11Buffer* mpVB, int VBOffset, 
			                  D3D11Buffer* mpIB, int IBOffset)
{

      struct VertexType{
		float3 vertex;
		float4 color;
		float3 normal;
	};
	int VBSize = mVertex.size()*GetVertexStride();
	unsigned char * pVBStart = (unsigned char *)mpVB->MapDeviceBuffer(dc, GS::WriteDiscard, VBOffset, VBSize);
	assert(pVBStart!= NULL);
	VertexType* pVBData =(VertexType*)(pVBStart+ VBOffset);

	int n = PrimitiveCount();
	for(int i = 0 ; i< n; i++)
	{
		int Id = mTriangle[i].VertexId[0];
		pVBData->vertex = mVertex[Id];
		pVBData->color =  mColorTable[mColorIdx[Id]];
		pVBData->normal = mTriangle[i].Normal;
		pVBData++;
		
		Id = mTriangle[i].VertexId[1];
		pVBData->vertex = mVertex[Id];
		pVBData->color =  mColorTable[mColorIdx[Id]];
		pVBData->normal = mTriangle[i].Normal;
		pVBData++;

		Id = mTriangle[i].VertexId[2];
		pVBData->vertex = mVertex[Id];
		pVBData->color =  mColorTable[mColorIdx[Id]];
		pVBData->normal = mTriangle[i].Normal;
		pVBData++;
	}
	struct IndexType {
		unsigned int v[3];

	};
	int IBSize = PrimitiveCount()*3* sizeof (unsigned int);
	unsigned char * pIBStart = (unsigned char *)mpIB->MapDeviceBuffer(dc, GS::WriteDiscard, IBOffset, IBSize);
		assert(pIBStart!= NULL);
    IndexType* pIBData =(IndexType*)(pIBStart + IBOffset);
	for (int i = 0 ; i < mTriangle.size(); i++)
	{
		pIBData->v[0] = 3*i;
		pIBData->v[1] = 3*i + 1;
		pIBData->v[2] = 3*i + 2;
		pIBData++;
	}
	mpVB->UnMapDeviceBuffer(dc);
	mpIB->UnMapDeviceBuffer(dc);
}


void BaseMeshImp::Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData)
{
   if (PrimitiveCount() ==0)
		return ; 
	if (!mpVB)
		mpVB = new D3D11Buffer(GS::Dynamic, GS::Vertex);
	if (!mpIB)
		mpIB = new D3D11Buffer(GS::Dynamic, GS::Index);
	if (bForceFillData)
	{
		mpVB->ReleaseDeviceBuffer();
		mpIB->ReleaseDeviceBuffer();
	}

	if (!mpVB->IsBufferAvailable())
	{
		mpVB->CreateDeviceBuffer(device, NULL, PrimitiveCount()*3* GetVertexStride() );
		if (!mpIB->IsBufferAvailable())
			mpIB->CreateDeviceBuffer(device, NULL, PrimitiveCount()*3* sizeof (unsigned int));
		FillD3DBuffer(dc,mpVB, 0, mpIB, 0);
	}
	unsigned int stride;
	unsigned int offset;
	ID3D11Buffer* pVB = mpVB->GetDeviceBuffer();

	// Set vertex buffer stride and offset.
	stride = GetVertexStride(); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	dc->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	dc->IASetIndexBuffer(mpIB->GetDeviceBuffer(), DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Render the triangle.
	dc->DrawIndexed(PrimitiveCount()*3, 0, 0);

}

BaseMesh::BaseMesh(bool bUseUniqueVertex)
{
    mpMeshImp = new BaseMeshImp(bUseUniqueVertex);
}


BaseMesh::BaseMesh(const BaseMesh& mesh)
{
   // tbb::tick_count t0 = tbb::tick_count::now(); 
    mpMeshImp = mesh.mpMeshImp;
    mpMeshImp->mRefCount++;
    mSurfaceList.resize(mesh.mSurfaceList.size());
    for (int i = 0; i < mesh.mSurfaceList.size(); i++)
    {
        mSurfaceList[i]= new Surface<double>(this, *mesh.mSurfaceList[i]);
    }
 
}

BaseMesh::~BaseMesh()
{

    for(int i = 0 ; i<mSurfaceList.size(); ++ i)
		delete mSurfaceList[i];
    {
	    mSurfaceList.clear();
        ListOfSurfaces().swap(mSurfaceList);
    }
    if (mpMeshImp->mRefCount > 1 )
        mpMeshImp->mRefCount--;
    else 
        delete mpMeshImp;
    mpMeshImp = NULL;
}

BaseMesh*  BaseMesh::Clone() 
{
    return new BaseMesh(*this);
}

 PlaneMesh*  BaseMesh::ToPlaneMesh() const 
 {
	PlaneMesh* result = new PlaneMesh(AABB());

	auto &tri = mpMeshImp->mTriangle;
	auto &polygons = result->Ploygons();
	
	PlanePolygon poly;
	poly.color = Vertex(0).color;
	int n = PrimitiveCount();
	for (int i = 0; i < n; i++)
	{
		poly.bplanes.clear();
		auto &v1 = Vertex(tri[i].VertexId[0]).pos;
		auto &v2 = Vertex(tri[i].VertexId[1]).pos;
		auto &v3 = Vertex(tri[i].VertexId[2]).pos;

		auto edge1 = v3 - v2;
		auto edge2 = v2 - v1;
		auto edge3 = v1 - v3;
		double3 norm = normalize(cross(edge1, edge2));
		Plane<double> splane(norm, v1);
		poly.splane = splane;

		double3 bnormal;
		bnormal = normalize(cross(edge3, norm));
		poly.bplanes.push_back(Plane<double>(bnormal, v1));
		bnormal = normalize(cross(edge2, norm));
		poly.bplanes.push_back(Plane<double>(bnormal, v2));
		bnormal = normalize(cross(edge1, norm));
		poly.bplanes.push_back(Plane<double>(bnormal, v3));

		polygons.push_back(poly);
	}

	return result;
    //PlaneMesh* pPlaneMesh = new PlaneMesh(AABB());
    //for (int i = 0; i<mSurfaceList.size(); i++)
    //{
    //    bool bIsCCW = mSurfaceList[i]->IsCCW();
    //    Plane<double> splane(mSurfaceList[i]->N(), mSurfaceList[i]->Origin());
    //    PlanePolygon planePoly;
    //    planePoly.color = mSurfaceList[i]->SurfaceColor();
    //    planePoly.splane = splane;
    //    std::vector<Point3D> outline;
    //    mSurfaceList[i]->GetLine(outline);
    //    int nPoints = outline.size();

    //    for (int j = 0; j< nPoints; j++)
    //    {
    //        vec3<double> edge = outline[(j+1)%nPoints] - outline[j];
    //        vec3<double> normal = normalize( cross(mSurfaceList[i]->N(), edge));
    //        if (!bIsCCW) 
    //        {
    //            normal.x = - normal.x;
    //            normal.y = - normal.y;
    //            normal.z = - normal.z;
    //        }
    //        Plane<double> bplane( normal, outline[j]);
    //        planePoly.bplanes.push_back(bplane);
    //    }
    //    pPlaneMesh->AddPolygon(planePoly);
    //}
    // return pPlaneMesh; 
}


void BaseMeshImp::NormalizeCoord(const Box3* bbox)
{
    assert(!m_bCoordNormalized);

    m_bCoordNormalized = true;
    m_OriginalBBox = mAABB;

    if (bbox) m_TransformBBox = *bbox;
    else m_TransformBBox = mAABB;

    mAABB.Set(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
	auto center = m_TransformBBox.Center();
	auto scale = m_TransformBBox.Diagonal();

	for (auto &v: mVertex)
		v = normalize(v, center, scale);
}
	
void BaseMeshImp::DenormalizeCoord(const Box3* bbox)
{
    assert(m_bCoordNormalized);

    if (bbox) m_TransformBBox = *bbox;

    m_bCoordNormalized = false;
    mAABB = m_OriginalBBox;

	auto center = m_TransformBBox.Center();
	auto scale = m_TransformBBox.Diagonal();

	for (auto &v: mVertex)
		v = denormalize(v, center, scale);
}

}