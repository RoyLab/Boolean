#include <iostream>
#include "typedefs.h"
#include "VecTest.h"

using namespace std;
void Vec2Test()
{
	//int2 test
	cout<<"/************int2 test***************/"<<endl;
	GS::int2 pt1(10, -10), pt2(-10, 10);
	GS::int2 pt3 = pt1+ pt2;
	if (pt3.x != 0 || pt3.y != 0)
		cout<<"failed in int2 Add test"<<endl;
	else 
		cout<<"int2 Add test sucess"<<endl;
	pt3 = pt1* pt2;
	if (pt3.x != -100 || pt3.y != -100)
        cout<<"failed in int2 Mul test"<<endl;
	else 
		cout<<"int2 Mul test sucess"<<endl;
	pt3 = pt1/pt2;
	if (pt3.x != -1 || pt3.y != -1)
        cout<<"failed in int2 Mul test"<<endl;
	else 
		cout<<"int2 Mul test sucess"<<endl;
	pt3= GS::int2(1,1);
	pt3++;
    if (pt3.x != 2 || pt3.y != 2)
        cout<<"failed in int2 increment test"<<endl;
	else 
		cout<<"int2 increment test sucess"<<endl;
	cout<<"/************int2 test completed******/"<<endl;

    //float2 test
	cout<<"/************float2 test***************/"<<endl;
	GS::float2 pt4(10.0, -10.0), pt5(-10.0, 10.0);
	GS::float2 pt6 = pt4 + pt5;
	if (pt6.x != 0.0 || pt6.y != 0.0)
		cout<<"failed in float2 Add test"<<endl;
	else 
		cout<<"float2 Add test sucess"<<endl;
	pt6 = pt4* pt5;
	if (pt6.x != -100.0 || pt6.y != -100.0)
        cout<<"failed in float2 Mul test"<<endl;
	else 
		cout<<"float2 Mul test sucess"<<endl;
	pt6 = pt4/pt5;
	if (pt6.x != -1.0 || pt6.y != -1.0)
        cout<<"failed in float2 Mul test"<<endl;
	else 
		cout<<"float2 Mul test sucess"<<endl;
	pt6= GS::float2(1.0,1.0);
	pt6++;
    if (pt6.x != 2.0 || pt6.y != 2.0)
        cout<<"failed in float2 increment test"<<endl;
	else 
		cout<<"float2 increment test sucess"<<endl;
	cout<<"/************float2 test completed******/"<<endl;

	    //double2 test
	cout<<"/************double2 test***************/"<<endl;
	GS::double2 pt7(10.0, -10.0), pt8(-10.0, 10.0);
	GS::double2 pt9 = pt7 + pt8;
	if (pt9.x != 0.0 || pt9.y != 0.0)
		cout<<"failed in double2 Add test"<<endl;
	else 
		cout<<"double2 Add test sucess"<<endl;
	pt9 = pt7* pt8;
	if (pt9.x != -100.0 || pt9.y != -100.0)
        cout<<"failed in double2 Mul test"<<endl;
	else 
		cout<<"double2 Mul test sucess"<<endl;
	pt9 = pt7/pt8;
	if (pt9.x != -1.0 || pt9.y != -1.0)
        cout<<"failed in double2 Mul test"<<endl;
	else 
		cout<<"double2 Mul test sucess"<<endl;
	pt9= GS::double2(1.0,1.0);
	pt9++;
    if (pt9.x != 2.0 || pt9.y != 2.0)
        cout<<"failed in double2 increment test"<<endl;
	else 
		cout<<"double2 increment test sucess"<<endl;
	cout<<"/************double2 test completed******/"<<endl;
}

void Vec3Test()
{
	//int3 test
	cout<<"/************int3 test***************/"<<endl;
	GS::int3 pt1(10, -10, 10), pt2(-10, 10, -10);
	GS::int3 pt3 = pt1+ pt2;
	if (pt3.x != 0 || pt3.y != 0 || pt3.z != 0)
		cout<<"failed in int3 Add test"<<endl;
	else 
		cout<<"int3 Add test sucess"<<endl;
	pt3 = pt1* pt2;
	if (pt3.x != -100 || pt3.y != -100 || pt3.z != -100)
        cout<<"failed in int3 Mul test"<<endl;
	else 
		cout<<"int3 Mul test sucess"<<endl;
	pt3 = pt1/pt2;
	if (pt3.x != -1 || pt3.y != -1 || pt3.z != -1)
        cout<<"failed in int3 Mul test"<<endl;
	else 
		cout<<"int3 Mul test sucess"<<endl;
	pt3= GS::int3(1,1, 1);
	pt3++;
    if (pt3.x != 2 || pt3.y != 2 || pt3.z != 2)
        cout<<"failed in int3 increment test"<<endl;
	else 
		cout<<"int3 increment test sucess"<<endl;
	cout<<"/************int3 test completed******/"<<endl;

    //float2 test
	cout<<"/************float2 test***************/"<<endl;
	GS::float3 pt4(10.0, -10.0, 10.0), pt5(-10.0, 10.0, -10.0);
	GS::float3 pt6 = pt4 + pt5;
	if (pt6.x != 0.0 || pt6.y != 0.0 || pt6.z != 0)
		cout<<"failed in float3 Add test"<<endl;
	else 
		cout<<"float3 Add test sucess"<<endl;
	pt6 = pt4* pt5;
	if (pt6.x != -100.0 || pt6.y != -100.0 || pt6.z != -100.0)
        cout<<"failed in float3 Mul test"<<endl;
	else 
		cout<<"float3 Mul test sucess"<<endl;
	pt6 = pt4/pt5;
	if (pt6.x != -1.0 || pt6.y != -1.0 || pt6.z != -1.0)
        cout<<"failed in float3 Mul test"<<endl;
	else 
		cout<<"float3 Mul test sucess"<<endl;
	pt6= GS::float3(1.0,1.0, 1.0);
	pt6++;
    if (pt6.x != 2.0 || pt6.y != 2.0 || pt6.z != 2.0)
        cout<<"failed in float3 increment test"<<endl;
	else 
		cout<<"float3 increment test sucess"<<endl;
	cout<<"/************float3 test completed******/"<<endl;

	    //double3 test
	cout<<"/************double3 test***************/"<<endl;
	GS::double3 pt7(10.0, -10.0, 10.0), pt8(-10.0, 10.0, -10.0);
	GS::double3 pt9 = pt7 + pt8;
	if (pt9.x != 0.0 || pt9.y != 0.0 || pt9.z != 0.0)
		cout<<"failed in double3 Add test"<<endl;
	else 
		cout<<"double3 Add test sucess"<<endl;
	pt9 = pt7* pt8;
	if (pt9.x != -100.0 || pt9.y != -100.0 || pt9.z != -100.0)
        cout<<"failed in double3 Mul test"<<endl;
	else 
		cout<<"double3 Mul test sucess"<<endl;
	pt9 = pt7/pt8;
	if (pt9.x != -1.0 || pt9.y != -1.0 || pt9.z != -1.0)
        cout<<"failed in double3 Mul test"<<endl;
	else 
		cout<<"double3 Mul test sucess"<<endl;
	pt9= GS::double3(1.0,1.0, 1.0);
	pt9++;
    if (pt9.x != 2.0 || pt9.y != 2.0 || pt9.z != 2.0)
        cout<<"failed in double3 increment test"<<endl;
	else 
		cout<<"double3 increment test sucess"<<endl;
	cout<<"/************double3 test completed******/"<<endl;

}

void Vec4Test()
{
		//int4 test
	cout<<"/************int4 test***************/"<<endl;
	GS::int4 pt1(10, -10, 10, -10), pt2(-10, 10, -10, 10);
	GS::int4 pt3 = pt1+ pt2;
	if (pt3.x != 0 || pt3.y != 0 || pt3.z != 0 || pt3.w != 0)
		cout<<"failed in int4 Add test"<<endl;
	else 
		cout<<"int4 Add test sucess"<<endl;
	pt3 = pt1* pt2;
	if (pt3.x != -100 || pt3.y != -100 || pt3.z != -100 || pt3.w != -100)
        cout<<"failed in int4 Mul test"<<endl;
	else 
		cout<<"int4 Mul test sucess"<<endl;
	pt3 = pt1/pt2;
	if (pt3.x != -1 || pt3.y != -1 || pt3.z != -1 || pt3.w != -1)
        cout<<"failed in int4 Mul test"<<endl;
	else 
		cout<<"int4 Mul test sucess"<<endl;
	pt3= GS::int4(1, 1, 1, 1);
	pt3++;
    if (pt3.x != 2 || pt3.y != 2 || pt3.z != 2 || pt3.w != 2)
        cout<<"failed in int4 increment test"<<endl;
	else 
		cout<<"int4 increment test sucess"<<endl;
	cout<<"/************int4 test completed******/"<<endl;

    //float2 test
	cout<<"/************float4 test***************/"<<endl;
	GS::float4 pt4(10.0, -10.0, 10.0, -10.0), pt5(-10.0, 10.0, -10.0, 10.0);
	GS::float4 pt6 = pt4 + pt5;
	if (pt6.x != 0.0 || pt6.y != 0.0 || pt6.z != 0 || pt6.w != 0)
		cout<<"failed in float4 Add test"<<endl;
	else 
		cout<<"float4 Add test sucess"<<endl;
	pt6 = pt4* pt5;
	if (pt6.x != -100.0 || pt6.y != -100.0 || pt6.z != -100.0 || pt6.w != -100.0)
        cout<<"failed in float4 Mul test"<<endl;
	else 
		cout<<"float3 Mul test sucess"<<endl;
	pt6 = pt4/pt5;
	if (pt6.x != -1.0 || pt6.y != -1.0 || pt6.z != -1.0 || pt6.w != -1.0)
        cout<<"failed in float4 Mul test"<<endl;
	else 
		cout<<"float4 Mul test sucess"<<endl;
	pt6= GS::float4(1.0, 1.0, 1.0, 1.0);
	pt6++;
    if (pt6.x != 2.0 || pt6.y != 2.0 || pt6.z != 2.0 || pt6.w != 2.0)
        cout<<"failed in float4 increment test"<<endl;
	else 
		cout<<"float4 increment test sucess"<<endl;
	cout<<"/************float4 test completed******/"<<endl;

	    //double4 test
	cout<<"/************double4 test***************/"<<endl;
	GS::double4 pt7(10.0, -10.0, 10.0, -10.0), pt8(-10.0, 10.0, -10.0, 10.0);
	GS::double4 pt9 = pt7 + pt8;
	if (pt9.x != 0.0 || pt9.y != 0.0 || pt9.z != 0.0 || pt9.w != 0.0)
		cout<<"failed in double4 Add test"<<endl;
	else 
		cout<<"double4 Add test sucess"<<endl;
	pt9 = pt7* pt8;
	if (pt9.x != -100.0 || pt9.y != -100.0 || pt9.z != -100.0 || pt9.w != -100.0)
        cout<<"failed in double4 Mul test"<<endl;
	else 
		cout<<"double4 Mul test sucess"<<endl;
	pt9 = pt7/pt8;
	if (pt9.x != -1.0 || pt9.y != -1.0 || pt9.z != -1.0 || pt9.w != -1.0)
        cout<<"failed in double4 Mul test"<<endl;
	else 
		cout<<"double4 Mul test sucess"<<endl;
	pt9= GS::double4(1.0,1.0, 1.0, 1.0);
	pt9++;
    if (pt9.x != 2.0 || pt9.y != 2.0 || pt9.z != 2.0 || pt9.w != 2.0)
        cout<<"failed in double4 increment test"<<endl;
	else 
		cout<<"double4 increment test sucess"<<endl;
	cout<<"/************double4 test completed******/"<<endl;
}