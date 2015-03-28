#include "precompile.h"
#include "IsectTriangle.h"
#include "isect.h"

namespace CSG
{
	ISectZone* ZONE;

	void GetLocation(ISVertexInfo* info, Vec3d& vec)
	{
		while (!info->pos.is_valid()) info = &(*info->next);
		vec = ZONE->mesh.point(info->pos);
	}

	ISectTriangle::ISectTriangle(MPMesh* mesh, MPMesh::FaceHandle f):
		xi(-1), yi(-1), pMesh(mesh), face(f),
		dtZone(nullptr)
	{
		// 三个角点在vertices的最后三个
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

	ISectTriangle::~ISectTriangle()	 {SAFE_RELEASE(dtZone);	}

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
		// 但是，如果这个ref本身就在这个里面，那么问题就来了
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

	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, Vec3d& vec, bool isMulti)
	{
		ISVertexItr output;
		if (pos == INNER)
		{
			if (!IsVertexExisted(tri, vec, output))
			{
				ISVertexInfo info;
				info.pos = ZONE->mesh.add_vertex(vec);
				tri->vertices.push_front(info);
				output = tri->vertices.begin();
			}
		}
		else if (pos <= EDGE_2)
		{
			auto mesh = tri->pMesh;
			auto ffItr = mesh->ff_begin(tri->face);
			switch (pos)
			{
			case CSG::EDGE_0:
				ffItr ++;
				ffItr ++;
				break;
			case CSG::EDGE_1:
				break;
			case CSG::EDGE_2:
				ffItr ++;
				break;
			default: 
				assert(0);
			}

			ISectTriangle*& other = mesh->property(mesh->SurfacePropHandle, *ffItr);
#ifdef _DEBUG_
			auto fvItr = mesh->fv_begin(tri->face);
			if (pos == EDGE_1) fvItr++;
			if (pos == EDGE_2) {fvItr++; fvItr++;}
			
			auto f0 = *fvItr;
			fvItr = mesh->fv_begin(*ffItr);
			assert(*fvItr != f0); fvItr++;
			assert(*fvItr != f0); fvItr++;
			assert(*fvItr != f0);
#endif
			if (!other) other = new ISectTriangle(mesh, *ffItr);
			output = InsertPoint(other, INNER, vec);
			return InsertPoint(tri, INNER, output);
		}
		else
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
		return output;
	}

	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, ISVertexItr ref, bool isMulti)
	{
		// 追溯到最源头的迭代器
		while (!ref->pos.is_valid()) ref = ref->next;

		Vec3d vec = ZONE->mesh.point(ref->pos);
		ISVertexItr output = ref;
		ISVertexInfo info;
		info.next = ref;
		if (pos == INNER)
		{
			if (!IsVertexExisted(tri, vec, output))
			{
				tri->vertices.push_front(info);
				output = tri->vertices.begin();
			}
			else
			{
				if (!output->pos.is_valid() || output->pos != ref->pos)
				{
					output->pos.reset();
					output->next = ref;
				}
			}
		}
		else if (pos <= EDGE_2)
		{
			auto mesh = tri->pMesh;
			auto ffItr = mesh->ff_begin(tri->face);
			switch (pos)
			{
			case CSG::EDGE_0:
				ffItr ++;
				ffItr ++;
				break;
			case CSG::EDGE_1:
				break;
			case CSG::EDGE_2:
				ffItr ++;
				break;
			default:
				assert(0);
			}

			ISectTriangle*& other = mesh->property(mesh->SurfacePropHandle, *ffItr);
			if (!other) other = new ISectTriangle(mesh, *ffItr);
			InsertPoint(other, INNER, ref);
			return InsertPoint(tri, INNER, ref);
		}
		else
		{
			// get the iterator of vertex #WR#
			switch (pos)
			{
			case CSG::VER_0:
				if (tri->corner[0]->pos == ref->pos) break;
				tri->corner[0]->pos.reset();
				tri->corner[0]->next = ref;
				output = tri->corner[0];
				break;
			case CSG::VER_1:
				if (tri->corner[1]->pos == ref->pos) break;
				tri->corner[1]->pos.reset();
				tri->corner[1]->next = ref;
				output = tri->corner[1];
				break;
			case CSG::VER_2:
				if (tri->corner[2]->pos == ref->pos) break;
				tri->corner[2]->pos.reset();
				tri->corner[2]->next = ref;
				output = tri->corner[2];
				break;
			}
		}
		
		return output;
	}

	void InsertSegment(ISectTriangle* tri, ISVertexItr v0, ISVertexItr v1, ISectTriangle* tri2)
	{
		ISCutSeg seg;
		seg.start = v0;
		seg.end = v1;
		seg.oppoTriangle = tri2;
		tri->segs[tri2->pMesh->ID].segs.push_back(seg);
	}

}

