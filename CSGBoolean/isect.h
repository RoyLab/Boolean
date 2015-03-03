#pragma once
#include <OpenMesh\Core\Geometry\VectorT.hh>
#include "AABBmp.h"

using OpenMesh::Vec3d;

namespace CSG
{

   bool TriangleAABBIntersectTest(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const AABBmp& bbox);

}
