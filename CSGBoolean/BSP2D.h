#pragma once
#include "BinaryTree.h"

namespace GEOM_FADE2D
{
	struct Point2;
}

namespace CSG
{
	struct BSP2D;

	Relation BSP2DInOutTest(const BSP2D*, GEOM_FADE2D::Point2*);
}
