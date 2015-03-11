#pragma once
#include <list>
#include <Fade_2D.h>
#include "MPMesh.h"
#include "BinaryTree.h"
#include <set>

namespace CSG
{
	using OpenMesh::Vec3d;
	struct ISectTriangle;
	struct ISVertexInfo;

	typedef std::list<ISectTriangle>::iterator	ISectTriItr;
	typedef std::list<Vec3d>::iterator			VertexItr;
	typedef std::list<ISVertexInfo>::iterator	ISVertexItr;

	enum VertexPos 
	{
		NONE =   -1, 
		EDGE_0 = 0x01, 
		EDGE_1 = 0x02, 
		EDGE_2 = 0x04, 
		VER_0  = 0x08, 
		VER_1  = 0x10, 
		VER_2  = 0x20, 
		INNER  = 0x00,
	};

	struct ISVertexInfo
	{
		MPMesh::VertexHandle		pos;
		ISVertexItr				next;

		ISVertexInfo(){}
		~ISVertexInfo(){}
	};

	struct ISCutSeg
	{
		ISVertexItr		start, end;
		ISectTriangle*	oppoTriangle;
	};

	struct ISectTriangle
	{
		MPMesh*				pMesh;
		MPMesh::FaceHandle	face;

		int mainIndex;
		std::set<int> relationTestId;

		ISVertexItr				corner[3]; // point to the first three elements in [vertices] #WR#
		std::list<ISVertexInfo> vertices;
		std::list<ISCutSeg>		segs;
		
		GEOM_FADE2D::Fade_2D		*dtZone;

		ISectTriangle(MPMesh* mesh, MPMesh::FaceHandle f);
		~ISectTriangle();
	};
	
	struct ISectZone
	{
		MPMesh mesh;
		std::list<ISectTriangle*> triangles;
	};
	
	void InitZone();
	void ReleaseZone();
	void Register(ISectTriangle*);
	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, Vec3d& vec);
	ISVertexItr InsertPoint(ISectTriangle* tri, VertexPos pos, ISVertexItr ref);
	void InsertSegment(ISectTriangle* tri, ISVertexItr v0, ISVertexItr v1, ISectTriangle* tri2);

} // namespace CSG


