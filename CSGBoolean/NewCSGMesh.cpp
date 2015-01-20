#include "NewCSGMesh.h"
#pragma comment(lib, "GS")

namespace CSG
{
    typedef unsigned uint;

    CSGMesh::CSGMesh(GS::BaseMesh* pMesh):
        SimpleMesh(pMesh), ID(-1)
    {
        uint vn = mVertex.size();
        mAABB.Clear();
        for (uint i = 0; i < vn; i++)
            mAABB.IncludePoint(mVertex[i]);
    }

    CSGMesh::~CSGMesh(void)
    {
    }

    GS::BaseMesh* ConverteToBaseMesh(CSGMesh* input)
    {
        return NULL;
    }

}
