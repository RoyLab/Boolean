#include "precompile.h"
#include "MPMesh.h"
#include <limits>
#include "BaseMesh.h"
#include "COctree.h"

namespace CSG
{
	static const unsigned VERTEX_ID_INIT = UINT_MAX;

	inline OpenMesh::Vec3d convert_double3(GS::double3& vec)
	{
		return OpenMesh::Vec3d(vec.x, vec.y, vec.z);
	}

	MPMesh::MPMesh(GS::BaseMesh* pMesh):
        ID(-1), pOrigin(pMesh), bInverse(false)
    {
		request_face_normals();
		add_property(PointInOutTestPropHandle);
		add_property(SurfacePropHandle);
		add_property(VertexIndexPropHandle);
    }

    MPMesh::~MPMesh(void)
    {
		release_face_normals();
		remove_property(PointInOutTestPropHandle);
		remove_property(SurfacePropHandle);
		remove_property(VertexIndexPropHandle);
    }

	MPMesh* ConvertToMPMesh(GS::BaseMesh* pMesh)
	{
		MPMesh *res = new MPMesh(pMesh);

        int n = (int)pMesh->VertexCount();
        res->BBox.Clear();
		MPMesh::VertexHandle vhandle;
        for (int i = 0; i < n; i++)
		{
            vhandle = res->add_vertex(convert_double3(pMesh->Vertex(i).pos));
			//res->property(res->PointInOutTestPropHandle, vhandle) = 0;
			res->BBox.IncludePoint(res->point(vhandle));
		}

		std::vector<MPMesh::VertexHandle>  face_vhandles(3);
        n = (int)pMesh->PrimitiveCount();
        for (int i = 0; i < n; i++)
        {
            auto &info = pMesh->TriangleInfo(i);
			face_vhandles[0] = res->vertex_handle(info.VertexId[0]);
			face_vhandles[1] = res->vertex_handle(info.VertexId[1]);
			face_vhandles[2] = res->vertex_handle(info.VertexId[2]);
            res->add_face(face_vhandles);
        }

		res->update_normals();

		return res;
		return NULL;
	}

} // namespace CSG 
