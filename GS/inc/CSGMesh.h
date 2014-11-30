#ifndef _CSG_MESH_H
#define _CSG_MESH_H
#include "BaseMesh.h"
#include "Polygon.h"
#include "tbb\tick_count.h"
namespace GS{

struct GroupRec{
    bool bUnified ; 
    int start;
    int end;
    GroupRec()
        :bUnified(false)
        , start(-1)
        , end(-1)
    {
    }
};

class CSGMesh:public BaseMesh{

public:
    CSGMesh (bool bUseUniqueVertex = true);
    CSGMesh(BaseMesh* pMesh);
    virtual ~CSGMesh();

    int AddSurface(const BaseMesh* pMesh, int triId);
    int AddSurface(const Polygon& poly);
    //������aMesh�໥�и�Ŀ̺�
    bool InterCarve(CSGMesh& mesh);
    bool InterCarve(const std::vector<int>& surfaceIds, CSGMesh& mesh, const std::vector<int>& otherSurfaceIds);
    //ȥ��MeshID�����ܴ������Ŀ̺���
    void Uncarve(int MeshId);
    //���ݿ̺��������ǻ������µ�Mesh
    CSGMesh* TriangulateByCarve(bool UseUniqueVertex = true);
    //��������ÿ��������Ƭ�����aMesh��λ�ù�ϵ
    //NOTE: Ϊ��ʡ�������������ͨ����ԭʼMesh,������������
    virtual void RelatedWith(BaseMesh* mesh); 
    TriMeshRelation RelatedWith(int TriId, BaseMesh* mesh );
    //�����ǻ�������������
    CSGMesh* ForceTriangulate();
    bool    AddTriangesGroup(int start, int end, bool unified = true);
    int     GetTriangesGroupCount() const { return mTriGroupRec.size();}
    const GroupRec& GetTriangleGroupRec(int id) const {return mTriGroupRec[id];}
protected:
        void AddCarveMeshID(int ID);
        void AddRelation(int TriId, int RelatedMeshID, TriMeshRelation Relation);
private:

    PointMeshRelation PointRelatedWithPlane(const Point3D& TriPos,const Vec3D& TriN,
                                           const Point3D& p, const Point3D& n);
private:
    std::vector<int>                   mInterCarveMeshID;
    std::vector<ListOfRelationRec>     mTriMeshRelation;
    std::hash_map<IndexPair, int, IndexPairCompare> mSurfacesMap;
    std::vector<GroupRec >            mTriGroupRec;
   

};



struct CSGRecMesh {
    BaseMesh* mpBaseMesh;
    BaseMesh* mpNoneCriticalMesh;
	CSGMesh*  mpCarveMesh;
	CSGMesh*  mpTriMesh;
	 
    CSGRecMesh(BaseMesh* pBasesh)
		:mpBaseMesh(pBasesh)
        ,mpNoneCriticalMesh(NULL)
		,mpCarveMesh(NULL)
		,mpTriMesh (NULL)
	{
		//mpCarveMesh = new CSGMesh(mpBaseMesh);
	}
	~CSGRecMesh()
	{

        if (mpNoneCriticalMesh)
            delete mpNoneCriticalMesh;
		if (mpCarveMesh)
			delete mpCarveMesh;
		if (mpTriMesh)
			delete mpTriMesh;
	}
	void Triangulate()
	{
		if (mpTriMesh == NULL)
			mpTriMesh = mpCarveMesh->TriangulateByCarve();
		mpTriMesh->GenCenterOfGravity();
   
	}

	void ClearCarveMesh()
	{
		if (mpCarveMesh)
			delete mpCarveMesh;
		mpCarveMesh = NULL;
	}

    void ClearNoCriticalMesh()
    {
       if (mpNoneCriticalMesh)
            delete mpNoneCriticalMesh;
       mpNoneCriticalMesh = NULL; 
    }


    bool  InterCarve( CSGRecMesh& mesh)
	{
        if (!mpCarveMesh)
            mpCarveMesh= new CSGMesh(mpBaseMesh);
        if (!mesh.mpCarveMesh)
            mesh.mpCarveMesh = new CSGMesh(mesh.mpBaseMesh);
		return mpCarveMesh->InterCarve(*mesh.mpCarveMesh);
	}

    void ComputeInterRelation(const CSGRecMesh& mesh)
	{
		  mpTriMesh->RelatedWith(mesh.mpBaseMesh);
		  mesh.mpTriMesh->RelatedWith(mpBaseMesh);
	}
};

typedef std::pair<CSGRecMesh*, CSGRecMesh*> CSGRecMeshPair;

}
#endif 
