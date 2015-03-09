#pragma once
#include <OpenMesh\Core\Geometry\VectorT.hh>
#include "AABBmp.h"

using OpenMesh::Vec3d;

namespace CSG
{

	bool TriangleAABBIntersectTest(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const AABBmp& bbox);
	bool TriTriIntersectTest(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const Vec3d& nv, 
							 const Vec3d& u0, const Vec3d& u1, const Vec3d& u2, const Vec3d& nu,
							 int& startType, int& endType, Vec3d& start, Vec3d& end);

	bool RayTriangleIntersectTest(const Vec3d& o, const Vec3d& d, const Vec3d& v0, const Vec3d& v1,  const Vec3d& v2,
                              double& u, double& v, double& t);
}
