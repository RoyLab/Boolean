//#pragma comment(lib, "GS.lib")
#include <iostream>
#include <vector>
#include "Box3.h"
#include "configure.h"
#include "topology.h"
#define _USE_MATH_DEFINES
#include <OpenMesh\Core\Geometry\VectorT.hh>
#include "IsectTriangle.h"

using OpenMesh::Vec3d;
using namespace CSG;

int main()
{
	Vec3d v0, v1, v2, u0, u1, u2, nv, nu, s, t;
	v0 = Vec3d(0,0,0);
	v1 = Vec3d(2,0,0);
	v2 = Vec3d(2,2,0);
	u0 = Vec3d(0,0,0);
	u1 = Vec3d(3,0,-1);
	u2 = Vec3d(3,2,1);

	nv = cross(v1-v0, v2-v1);
	nv = nv/nv.length();
	nu = cross(u1-u0, u2-u1);
	nu = nu/nu.length();


	enum fuc{A, B, C};

	fuc a = fuc(10);
	int b = int(a);

	int c = sizeof(std::list<int>);
	std::cout << c <<std::endl;
	system("pause");
	return 0;
}
