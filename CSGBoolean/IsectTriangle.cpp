#include "precompile.h"
#include "IsectTriangle.h"
#include "isect.h"

namespace CSG
{
	ISectZone* ZONE;

	ISectTriangle::ISectTriangle(MPMesh* mesh, MPMesh::FaceHandle f):
		mainIndex(-1), pMesh(mesh), face(f),
		dtZone(nullptr)
	{
		auto fvItr = pMesh->fv_begin(face);
		ISVertexInfo info;
		info.pos = *fvItr;
		vertices.push_back(info);
		corner[0] = vertices.end();
		corner[0] --;

		fvItr++;
		info.pos = *fvItr;
		vertices.push_back(info);
		corner[1] = vertices.end();
		corner[1] --;

		fvItr++;
		info.pos = *fvItr;
		vertices.push_back(info);
		corner[2] = vertices.end();
		corner[2] --;

		// add to result mesh.  #WR#
		MPMesh::VertexHandle tmp;
		for (int i = 0; i < 3; i++)
		{
			auto &prop = pMesh->property(pMesh->VertexIndexPropHandle, corner[i]->pos);
			if (!prop.is_valid()) 
				prop = ZONE->mesh.add_vertex(pMesh->verticesList[corner[i]->pos.idx()]);
			corner[i]->pos = prop;
		}
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

	inline bool CompareVertex(ISVertexItr& ref, const Vec3d& vec)
	{
		auto refcpy = ref;
		while (!refcpy->pos.is_valid()) refcpy = refcpy->next;
		return IsEqual(ZONE->mesh.point(refcpy->pos), vec);
	}


	bool IsVertexExisted(ISectTriangle* tri, Vec3d& vec, ISVertexItr& ref)
	{
		// we do not check if it is a corner point #WR#
		// 因为所有的插入点都是在前面的，所以不用检查到最后一个：a, b, c, d, v0, v1, v2.
		const auto end = tri->corner[0];
		for (auto itr = tri->vertices.begin(); itr != end; itr++)
		{
			if (CompareVertex(itr, vec))
			{
				ref = itr;
				return true;
			}
		}
		return false;
	}

	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, Vec3d& vec)
	{
		assert(pos >= EDGE_0 && pos <= INNER);

		ISVertexItr output;
		if (pos <= EDGE_2)
		{
			if (!IsVertexExisted(tri, vec, output))
			{
				ISVertexInfo info;
				info.pos = ZONE->mesh.add_vertex(vec);
				tri->vertices.push_front(info);
				output = tri->vertices.begin();
			}

			auto mesh = tri->pMesh;
			auto ffItr = mesh->ff_begin(tri->face);
			switch (pos)
			{
			case CSG::EDGE_0:
				ffItr ++;
				break;
			case CSG::EDGE_1:
				ffItr ++;
				ffItr ++;
				break;
			case CSG::EDGE_2:
				break;
			default: 
				assert(0);
			}

			ISectTriangle*& other = mesh->property(mesh->SurfacePropHandle, *ffItr);
			if (!other) other = new ISectTriangle(mesh, *ffItr);
			InsertPoint(other, INNER, output);
		}
		else if (pos <= VER_2)
		{
			// get the iterator of vertex #WR#
			switch (pos)
			{
			case CSG::VER_0:
				output = tri->corner[0];
				break;
			case CSG::VER_1:
				output = tri->corner[1];
				break;
			case CSG::VER_2:
				output = tri->corner[2];
				break;
			}
		}
		else 
		{
			assert(pos == INNER);
			ISVertexInfo info;
			info.pos = ZONE->mesh.add_vertex(vec);
			tri->vertices.push_front(info);
			output = tri->vertices.begin();
		}

		while (!output->pos.is_valid()) output = output->next;
		return output;
	}

	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, ISVertexItr ref)
	{
		assert(pos >= EDGE_0 && pos <= INNER);
		//assert(ref->pos.is_valid());
		while (!ref->pos.is_valid()) ref = ref->next;

		Vec3d vec = tri->pMesh->point(ref->pos);
		ISVertexItr output;
		ISVertexInfo info;
		info.next = ref;
		if (pos <= EDGE_2)
		{
			if (!IsVertexExisted(tri, vec, output))
			{
				tri->vertices.push_front(info);
				output = tri->vertices.begin();
			}
			else
			{
				output->pos.reset();
				output->next = ref;
			}

			auto mesh = tri->pMesh;
			auto ffItr = mesh->ff_begin(tri->face);
			switch (pos)
			{
			case CSG::EDGE_0:
				ffItr ++;
				break;
			case CSG::EDGE_1:
				ffItr ++;
				ffItr ++;
				break;
			case CSG::EDGE_2:
				break;
			default: 
				assert(0);
			}

			ISectTriangle*& other = mesh->property(mesh->SurfacePropHandle, *ffItr);
			if (!other) other = new ISectTriangle(mesh, *ffItr);
			InsertPoint(other, INNER, ref);
		}
		else if (pos <= VER_2)
		{
			// get the iterator of vertex #WR#
			switch (pos)
			{
			case CSG::VER_0:
				tri->corner[0]->pos.reset();
				tri->corner[0]->next = ref;
				output = tri->corner[0];
				break;
			case CSG::VER_1:
				tri->corner[1]->pos.reset();
				tri->corner[1]->next = ref;
				output = tri->corner[1];
				break;
			case CSG::VER_2:
				tri->corner[2]->pos.reset();
				tri->corner[2]->next = ref;
				output = tri->corner[2];
				break;
			}
		}
		else 
		{
			assert(pos == INNER);
			tri->vertices.push_front(info);
			output = tri->vertices.begin();
		}
		
		return output;
	}

	void InsertSegment(ISectTriangle* tri, ISVertexItr v0, ISVertexItr v1, ISectTriangle* tri2)
	{
		ISCutSeg seg;
		seg.start = v0;
		seg.end = v1;
		seg.oppoTriangle = tri2;
		tri->segs.push_back(seg);
	}

}

