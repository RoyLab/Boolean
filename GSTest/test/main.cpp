#pragma comment(lib, "GS.lib")
#include <iostream>
#include <vector>
#include "adaptive.h"


int main()
{
	GS::exactinit();
	//GS::double4x4 mat;
	//mat[0] = GS::double4(1, 0, 0, 0);
	//mat[1] = GS::double4(0, 1.1, 1.1, 0);
	//mat[2] = GS::double4(0, 1+pow(0.5, 52), 1+pow(0.5, 52), 0);
	//mat[3] = GS::double4(0, 0, 0, 1);
	//std::cout << GS::adaptiveDet4x4Sign(mat) << std::endl;

	GS::double3x3 mat;
	mat[0] = GS::double3(1, 1-pow(0.5, 52), 0);
	mat[1] = GS::double3(1, 1, 0);
	mat[2] = GS::double3(0, 0, 2);
	std::cout << GS::adaptiveDet3x3Sign(mat) << std::endl;
	system("pause");
	return 0;
}
