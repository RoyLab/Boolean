#pragma once
#include <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>
#include <typedefs.h>
#include "AABBmp.h"

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
		MPMesh(GS::BaseMesh* pMesh);
        ~MPMesh(void);

		int  ID;
		bool bInverse;
		AABBmp BBox;

        GS::BaseMesh* pOrigin;

		OpenMesh::FPropHandleT<unsigned> PointInOutTestPropHandle; // 是否在内外测试中被检测过
		OpenMesh::FPropHandleT<ISectTriangle*> SurfacePropHandle; // 是否属于相交三角形
		OpenMesh::VPropHandleT<MPMesh::VertexHandle> VertexIndexPropHandle; // 在结果网格中的index
	};


	MPMesh* ConvertToMPMesh(GS::BaseMesh* pMesh);
	inline GS::double3 Vec3dToDouble3(const Vec3d& vec){return GS::double3(vec[0], vec[1], vec[2]);}
	inline Vec3d Double3ToVec3d(const GS::double3& vec){return Vec3d(vec[0], vec[1], vec[2]);}

} // namespace CSG
