#include "NewCSGMesh.h"
#include "BaseMesh.h"
#pragma comment(lib, "GS")

namespace CSG
{
    CSGMesh::CSGMesh(GS::BaseMesh* pMesh):ID(-1)
    {
        int n = (int)pMesh->VertexCount();
        for (int i = 0; i < n; i++)
            mVertex.push_back(pMesh->Vertex(i).pos);

        n = (int)pMesh->PrimitiveCount();
        for (int i = 0; i < n; i++)
        {
            auto &index = pMesh->TriangleInfo(i);
            mTriangle.emplace_back(index.VertexId, index.Normal);
        }

        uint vn = mVertex.size();
        mAABB.Clear();
        for (uint i = 0; i < vn; i++)
            mAABB.IncludePoint(mVertex[i]);
    }

    CSGMesh::~CSGMesh(void)
    {
    }

    GS::BaseMesh* ConverteToBaseMesh(GS::double3* input, int count)
    {
        return NULL;
    }

}
