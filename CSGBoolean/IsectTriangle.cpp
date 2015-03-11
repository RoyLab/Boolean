#include "precompile.h"
#include "IsectTriangle.h"

namespace CSG
{
	ISectZone* ZONE;

	ISectTriangle::ISectTriangle(MPMesh* mesh, MPMesh::FaceHandle f):
		mainIndex(-1), pMesh(mesh), face(f),
		dtZone(nullptr)
	{
		Register(this);
	}

	ISectTriangle::~ISectTriangle()	 {if (dtZone) delete dtZone;	}

	void InitZone()
	{
		if (!ZONE) ZONE = new ISectZone;
	}

	void ReleaseZone()
	{
		for (auto itr: ZONE->triangles)
			delete itr;
		SAFE_RELEASE(ZONE);
	}

	void Register(ISectTriangle* tri)
	{
		ZONE->triangles.push_back(tri);
	}

	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, Vec3d& vec)
	{
		return ISVertexItr();
	}

	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, ISVertexItr v)
	{
		return ISVertexItr();
	}

	void InsertSegment(ISectTriangle* tri, ISVertexItr v0, ISVertexItr v1, ISectTriangle* tri2)
	{

	}

}

