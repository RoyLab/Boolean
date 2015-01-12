#ifndef _BASE_MESH_H_
#define _BASE_MESH_H_
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <cmath>
#include "configure.h"
#include "Box3.h"
#include "topology.h"

#include "Geometry.h"
#include "Surface.h"
#include "Plane.h"
#include "PlaneMesh.h"
#include "D3D11Buffer.h"
#include "FixedPlaneMesh.h"

//#include "tbb\blocked_range.h"
//#include "tbb\parallel_reduce.h"
//#include "tbb/spin_mutex.h"

namespace GS{



enum  PointMeshRelation 
{
	pmrUnknown, 
	pmrInside, 
	pmrOutside, 
	pmrSame, 
	pmrOpposite
};

enum  TriMeshRelation 
{
	tmrUnknown, 
	tmrInside, 
	tmrOutside, 
	tmrSame, 
	tmrOpposite
};

enum ManifordType {
	mtRegular, 
	mtPlane
};

 struct RelationRec {
    int             MeshID;
    TriMeshRelation Relation;
 };

 

template<typename P= float> struct TriangleInfo{
	vec3<P>    Normal;
	int        VertexId[3];

};


typedef Surface<double>*                SurfacePtr; 
typedef TriangleInfo<double>            TriInfo;
typedef TVertexInfo<double>             VertexInfo;
typedef std::vector<TriInfo>            ListOfTriInfos;
typedef std::vector<SurfacePtr>         ListOfSurfaces;
typedef std::vector<RelationRec>        ListOfRelationRec;


struct BaseMeshImp{
public:
        int                 mID;
        bool                mbUniqueVertex;
        Box3                mAABB;
        ListOf3DPoints      mCenterOfGravity;
        ListOfTriInfos      mTriangle;
        ListOf3DPoints      mVertex;
        ListOf3DPoints      mNormal;
        std::vector<int>    mColorIdx;
        std::vector<float4> mColorTable;
        std::vector<Box3>   mTriAABBList;
        int                 mPrimitiveCapacity;
        int                 mVertexCapacity;
        std::hash_map<Point3D, int, Vec3HashCompare<double> > mVertexDict;
        ManifordType        meManifordType;
        D3D11Buffer       * mpVB;
        D3D11Buffer       * mpIB;
        // Device related ; 
        int                 mRefCount; // reference count 
        
        // WR
        bool                m_bCoordNormalized;
        Box3                m_OriginalBBox;
        Box3                m_TransformBBox;

    public :
         BaseMeshImp(bool bUseUniqueVertex = true);      
         BaseMeshImp(const BaseMeshImp&);
         ~BaseMeshImp();
         int            PrimitiveCount () const { return mTriangle.size(); }
         void           GenCenterOfGravity();
         VertexInfo     Vertex(int idx) const;
         int            Add(const VertexInfo& v1, const VertexInfo& v2, const VertexInfo& v3); 
         int            AddTriangle(const BaseMeshImp* pMesh, int TriId);
         int            AddVertex(const VertexInfo& v);
         int            AddTriangle(int vId1, int vId2, int vId3, const Vec3D& normal);
        Point3D         CenterOfGravity (int idx) const;
        void            GetMesh(ListOf3DPoints& Triangles);
        void            GenAABB(bool bNeedTriAABB);
        const TriInfo&  TriangleInfo(int idx) const;
        PointMeshRelation RelationWith(const Point3D& pos, const Vec3D& dir );
        bool            TrianglesIntersectTest(int TriId, const BaseMeshImp& meshB, int MeshBTriId, 
                        std::vector<Seg3D<double> >&);
        void            Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData);
        void            FillD3DBuffer(ID3D11DeviceContext* dc,D3D11Buffer* mpVB, int VBOffset, 
                                        D3D11Buffer* mpIB, int IBOffet);

        //WR
        void            NormalizeCoord(const Box3* = nullptr);
        void            DenormalizeCoord(const Box3*);

protected:

        PointMeshRelation  CoplanarRelation(const Point3D& pos, const Vec3D& dir );
		PointMeshRelation  RelationWithPoint(const Point3D& pos, const Vec3D& dir );
        void        CheckPrimitiveCapacity();
        void        CheckVertexCapacity();
         int        GetVertexStride() const ;
private: 
  
        BaseMeshImp& operator = (const BaseMeshImp& );


    };

class BaseMesh: public Geometry {
        friend class SimpleMesh;
public:
        BaseMesh(bool bUseUniqueVertex = true);
         BaseMesh(const BaseMesh&);
		virtual ~BaseMesh();

        /// return  the number of facets in the mesh.
		inline int                 PrimitiveCount () const { return mpMeshImp->PrimitiveCount(); }
		inline int                 VertexCount    () const { return mpMeshImp->mVertex.size();}
		virtual BaseMesh*          Clone() ;
		//virtual BaseMesh*          Add(BaseMesh* mesh);
	/*	void                       Add(const Point3D& p1, const Point3D& p2, const Point3D& p3);*/
        int                        Add(const VertexInfo& v1, const VertexInfo& v2, const VertexInfo& v3) ;
        int                        Add(const VertexInfo& v1, const VertexInfo& v2, 
                                        const VertexInfo& v3,  const Vec3D& normal);
        int                        AddTriangle(const BaseMesh* pMesh, int TriId);
        void                       AddSurface(Surface<double>* pSurface);
		inline const Box3&                AABB () const {return mpMeshImp->mAABB;}
        inline const Box3&                PrimitiveAABB(int idx ) const {return mpMeshImp->mTriAABBList[idx];} 
        ListOfSurfaces&            Surfaces()  {return mSurfaceList;}
		inline int                        GetID() const {return mpMeshImp->mID;}
        Point3D                    CenterOfGravity (int idx) const {return mpMeshImp->CenterOfGravity(idx);}
        void                       GenCenterOfGravity();
        void                       GenID();
		void                       GenSurface();
		void                       GenAABB(bool bNeedTriAABB );
        void                       GetMesh(ListOf3DPoints& Triangles) const ;
		const   TriInfo&           TriangleInfo(int idx) const;
        inline VertexInfo           Vertex(int idx) const {return mpMeshImp->Vertex(idx);}
		virtual PointMeshRelation  RelationWith(const Point3D& pos, const Vec3D& dir );
		ManifordType               GetManifordType() const {return mpMeshImp->meManifordType;}
		void                       SetManifordType(ManifordType eManiford) {mpMeshImp->meManifordType =eManiford;}
        PlaneMesh*                 ToPlaneMesh() const ; 
		 FixedPlaneMesh*           ToFixedPlaneMesh() const ;
		virtual void               Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData);
	
		//WR
        void							NormalizeCoord(const Box3* bbox = nullptr) {mpMeshImp->NormalizeCoord(bbox);}
        void							DenormalizeCoord(const Box3* bbox= nullptr) {mpMeshImp->DenormalizeCoord(bbox);}
        void                        SetTransformedAABB(const Box3& box) {mpMeshImp->m_TransformBBox = box; mpMeshImp->m_bCoordNormalized = true;}
        int                        AddTriangle(const double3*);
protected:

        void                       SetID(int nID) {  mpMeshImp->mID = nID; }
        void                       SetAABB(const Box3& bound) {mpMeshImp->mAABB = bound;}
        void                       ClearSurfaces();
        bool                       TrianglesIntersectTest(int TriId, const BaseMesh& meshB, int MeshBTriId, 
                                           std::vector<Seg3D<double> >&);
      // virtual void                FillD3DBuffer(ID3D11DeviceContext* dc,D3D11Buffer* mpVB, int VBOffset, 
     //                                           D3D11Buffer* mpIB, int IBOffet);
        static   int                GenMeshID();
        static   std::hash_map<int, bool> mMeshIDSet;

protected :
        BaseMeshImp*   mpMeshImp;
        ListOfSurfaces      mSurfaceList;
  
};

inline 
void BaseMesh::GenCenterOfGravity()
{
    mpMeshImp->GenCenterOfGravity();
}

inline 
int BaseMesh::Add(const VertexInfo& v1, const VertexInfo& v2, const VertexInfo& v3) 
{
    BaseMeshImp* pImp = mpMeshImp;
    if (mpMeshImp->mRefCount > 1)
    {
        mpMeshImp->mRefCount--;
        mpMeshImp = new BaseMeshImp(*pImp);
    }
    return mpMeshImp->Add(v1, v2, v3);
}

inline
int  BaseMesh::Add(const VertexInfo& v1, const VertexInfo& v2, 
                      const VertexInfo& v3,  const Vec3D& normal)
{
     BaseMeshImp* pImp = mpMeshImp;
    if (mpMeshImp->mRefCount > 1)
    {
        mpMeshImp->mRefCount--;
        mpMeshImp = new BaseMeshImp(*pImp);
    }
   int vid1 = mpMeshImp->AddVertex(v1);
   int vid2 = mpMeshImp->AddVertex(v2);
   int vid3 = mpMeshImp->AddVertex(v3);
   return mpMeshImp->AddTriangle(vid1, vid2, vid3, normal);
 
}

inline 
void  BaseMesh::AddSurface(Surface<double>* pSurface)
{
    assert(pSurface && pSurface->Triangulated());
    BaseMeshImp* pImp = mpMeshImp;
    if (mpMeshImp->mRefCount > 1)
    {
        mpMeshImp->mRefCount--;
        mpMeshImp = new BaseMeshImp(*pImp);
    }
    for (int j = 0; j < pSurface->TriangleCount(); j++)
    {
       int TriId = pSurface->GetTriangleId(j);
       AddTriangle(pSurface->GetParent(), TriId);
	}
}

inline 
int  BaseMesh::AddTriangle(const BaseMesh* pMesh, int TriId)
{
    BaseMeshImp* pImp = mpMeshImp;
    if (mpMeshImp->mRefCount > 1)
    {
        mpMeshImp->mRefCount--;
        mpMeshImp = new BaseMeshImp(*pImp);
    }
    return mpMeshImp->AddTriangle(pMesh->mpMeshImp, TriId);
    
}

inline 
int  BaseMesh::AddTriangle(const double3* pos)
{
    VertexInfo info[3];
    info[0].pos = pos[0];
    info[1].pos = pos[1];
    info[2].pos = pos[2];
    return mpMeshImp->Add(info[0], info[1], info[2]);
    
}

inline
void BaseMesh::GetMesh(ListOf3DPoints& Triangles)  const 
{
     mpMeshImp->GetMesh(Triangles);
}

inline 
void  BaseMesh::GenAABB(bool bNeedTriAABB )
{
    return mpMeshImp->GenAABB(bNeedTriAABB);
}

inline
const   TriInfo&  BaseMesh::TriangleInfo(int idx) const
{
    return mpMeshImp->TriangleInfo(idx);
}

inline 
PointMeshRelation  BaseMesh::RelationWith(const Point3D& pos, const Vec3D& dir )
{
    return mpMeshImp->RelationWith(pos, dir);
}

inline 
bool  BaseMesh::TrianglesIntersectTest(int TriId, const BaseMesh& meshB, int MeshBTriId, 
                             std::vector<Seg3D<double> >& intersects)
{
    return mpMeshImp->TrianglesIntersectTest(TriId, *meshB.mpMeshImp, MeshBTriId, intersects);
}

inline 
void  BaseMesh::Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData)
{
    mpMeshImp->Render(device, dc, bForceFillData);
}


}




#endif 