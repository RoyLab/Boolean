#pragma once
#include <list>
#include <Fade_2D.h>
#include "MPMesh.h"
#include "BinaryTree.h"
#include <map>

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
		MPMesh::VertexHandle	 pos;
		ISVertexItr	next;
		int Id;

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
		BSP2D** bsp;

		int mainIndex;
		std::vector<int> relationTestId; // 有哪些折痕需要内外测试，对应ISVertexInfo中的relation

		ISVertexItr				corner[3]; // point to the last three elements in [vertices] #WR#
		std::list<ISVertexInfo> vertices;
		std::map<int, std::list<ISCutSeg>> segs;
		
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
	void GetLocation(ISVertexInfo* info, Vec3d& vec);

	void ParsingFace(MPMesh*, MPMesh::FaceHandle, const CSGTree*);
	void GetRelationTable(MPMesh* pMesh, MPMesh::FaceHandle curFace, MPMesh::FaceHandle seedFace, Relation* relationSeed, unsigned nMesh, Relation*& output);
	bool CompareRelationSpace();

	extern ISectZone* ZONE;

} // namespace CSG


