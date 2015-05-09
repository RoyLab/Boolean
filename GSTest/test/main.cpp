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
	v0 = Vec3d(0,2,0);
	v1 = Vec3d(-1,1,0);
	v2 = Vec3d(1,1,0);
	u0 = Vec3d(0,0,0);
	u1 = Vec3d(1,2,0);
	u2 = Vec3d(-1,2,0);

	nv = cross(v1-v0, v2-v1);
	nv = nv/nv.length();
	nu = cross(u1-u0, u2-u1);
	nu = nu/nu.length();

	OpenMesh::TriMesh_ArrayKernelT<MyTraits> myMesh;
	auto a0 = myMesh.add_vertex(u0);
	auto a1 = myMesh.add_vertex(v1);
	auto a2 = myMesh.add_vertex(v2);
	auto b0 = myMesh.add_vertex(v0);
	auto b1 = myMesh.add_vertex(u1);
	auto b2 = myMesh.add_vertex(u2);
	auto face = myMesh.add_face(a0,a1,a2);
	myMesh.add_face(b0,a2,a1);
	myMesh.add_face(a0,a2,b1);
	myMesh.add_face(a0,b2,a1);

	auto fvItr = myMesh.fv_begin(face);
	auto k0 = *fvItr; fvItr++;
	auto k1 = *fvItr; fvItr++;
	auto k2 = *fvItr;

	auto ffItr = myMesh.ff_begin(face);
	auto f0 = *ffItr; ffItr++;
	auto f1 = *ffItr; ffItr++;
	auto f2 = *ffItr;

	auto feItr = myMesh.fe_begin(face);
	auto e0 = myMesh.edge(*feItr);

	enum fuc{A, B, C};

	fuc a = fuc(10);
	int b = int(a);

	int c = sizeof(std::list<int>);
	std::cout << c <<std::endl;
	//system("pause");
	return 0;
}
