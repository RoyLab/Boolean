#pragma comment(lib, "GS.lib")
#include <iostream>
#include <vector>
#include "Box3.h"
#include "configure.h"
#include "topology.h"
#include "Intersect.h"

using namespace GS;

int main()
{
	double3 v0, v1, v2, u0, u1, u2, nv, nu;
	std::vector<Seg3D<double>> isect;

	v0 = double3(0,0,0);
	v1 = double3(2,0,0);
	v2 = double3(2,2,0);
	u0 = double3(1,0,0);
	u1 = double3(3,0,0);
	u2 = double3(3,2,0);

	nv = normalize(cross(v1-v0, v2-v1));
	nu = normalize(cross(u1-u0, u2-u1));

	TriTriIntersectTest(v0, v1, v2, nv, u0, u1, u2, nu, isect);
	system("pause");
	return 0;
}
