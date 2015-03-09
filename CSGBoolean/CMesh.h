#pragma once
#include "AABB.h"
#include <vector>
#include "configure.h"

namespace GS
{
    class BaseMesh;
}


namespace CSG
{
    struct TriangleInfo
    {
        unsigned VertexIndex[3];
        GS::double3 Normal;
        TriangleInfo(const int* index, const double3& normal)
        {
            VertexIndex[0] = index[0];
            VertexIndex[1] = index[1];
            VertexIndex[2] = index[2];
            Normal = normal;
        }
    };

    struct CSGMesh
    {
        CSGMesh(GS::BaseMesh* pMesh);
        ~CSGMesh(void);

        const std::vector<double3>& Vertex() const {return mVertex;}
        const std::vector<TriangleInfo>& Triangle() const {return mTriangle;}

        std::vector<double3> mVertex;
        std::vector<TriangleInfo> mTriangle;
        AABB mAABB;
        int ID;  // from Zero to N
        GS::BaseMesh* pOrigin;
		bool bInverse;
    };

    GS::BaseMesh* ConverteToBaseMesh(GS::ListOfvertices &vertex);
}


