#pragma once
#include <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
#include <typedefs.h>
#include "AABBmp.h"
#include <Fade_2D.h>

namespace GS
{
	class BaseMesh;

} // namespace GS


namespace CSG
{
	using OpenMesh::Vec3d;
	struct ISectTriangle;

	struct MyTraits : OpenMesh::DefaultTraits
	{
		typedef Vec3d Point;
		typedef Vec3d Normal;
	};

	typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MPMeshKernel;

	struct MPMesh:public MPMeshKernel
	{
		MPMesh(GS::BaseMesh* pMesh = nullptr);
        ~MPMesh(void);

		int  ID;
		bool bInverse;
		AABBmp BBox;

        GS::BaseMesh* pOrigin;
		Vec3d *verticesList;

		OpenMesh::FPropHandleT<unsigned> PointInOutTestPropHandle; // �Ƿ�����������б�����
		OpenMesh::FPropHandleT<ISectTriangle*> SurfacePropHandle; // �Ƿ������ཻ������
		OpenMesh::VPropHandleT<MPMesh::VertexHandle> VertexIndexPropHandle; // �ڽ�������е�index
		OpenMesh::FPropHandleT<int> MarkPropHandle;
	};


	MPMesh* ConvertToMPMesh(GS::BaseMesh* pMesh);
	inline GS::double3 Vec3dToDouble3(const Vec3d& vec){return GS::double3(vec[0], vec[1], vec[2]);}
	inline Vec3d Double3ToVec3d(const GS::double3& vec){return Vec3d(vec[0], vec[1], vec[2]);}

	inline void GetCorners(MPMesh* pMesh, MPMesh::FaceHandle fhandle, Vec3d*& v0, Vec3d *&v1, Vec3d *&v2)
	{
		auto fvItr = pMesh->fv_begin(fhandle);
		v0 = &(pMesh->verticesList[fvItr->idx()]);fvItr++;
		v1 = &(pMesh->verticesList[fvItr->idx()]);fvItr++;
		v2 = &(pMesh->verticesList[fvItr->idx()]);
	}

	inline int TestNeighborIndex(MPMesh* pMesh, MPMesh::FaceHandle faceSeed, MPMesh::FaceHandle face)
	{
		auto ffItr = pMesh->ff_begin(faceSeed);
		if (*ffItr == face) return 1;
		if (*(++ffItr) == face) return 2;
		if (*(++ffItr) == face) return 0;
		else return -1;
	}

} // namespace CSG
