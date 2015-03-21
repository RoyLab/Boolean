#pragma once
#include "BinaryTree.h"
#include "MPMesh.h"
#include "IsectTriangle.h"

namespace GEOM_FADE2D
{
	class Point2;
}

namespace CSG
{
	struct ISectTriangle;
	struct ISCutSeg;
	typedef Vec3d Line2D;
	using GEOM_FADE2D::Point2;

	struct BSPSeg
	{
		Point2 start, end;
		Line2D lineCoef;
	};

	struct BSP2D
	{
		enum {NA, IN, OUT};

		BSP2D *leftTree, *rightTree;
		Line2D line;
		int type;

		BSP2D(int t = NA):leftTree(0), rightTree(0), type(t){}
		~BSP2D(){SAFE_RELEASE(leftTree); SAFE_RELEASE(rightTree);}
	};

	Relation BSP2DInOutTest(const BSP2D*, GEOM_FADE2D::Point2*);
	BSP2D* BuildBSP2D(ISectTriangle* tri, unsigned index, std::vector<TMP_VInfo>& infos);
	bool SegIsectTest2D(ISCutSeg& seg1, ISCutSeg& seg2, std::vector<TMP_VInfo>& infos, Vec3d& output);
}

