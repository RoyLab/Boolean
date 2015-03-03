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
	struct MyTraits : OpenMesh::DefaultTraits
	{
		typedef OpenMesh::Vec3d Point;
		typedef OpenMesh::Vec3d Normal;
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
	};


	MPMesh* ConvertToMPMesh(GS::BaseMesh* pMesh);


} // namespace CSG
