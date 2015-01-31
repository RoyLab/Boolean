#pragma once
#include <Windows.h>

#ifdef CSG_EXPORTS
#define CSG_API __declspec(dllexport)
#else
#define CSG_API __declspec(dllimport)
#endif

namespace GS
{
    class BaseMesh;
    class CSGExprNode;
}


namespace CSG
{
    extern "C" CSG_API GS::BaseMesh* BooleanOperation(GS::CSGExprNode* input, HANDLE stdoutput);
    extern "C" CSG_API GS::BaseMesh* BooleanOperation_MultiThread(GS::CSGExprNode* input);
}

