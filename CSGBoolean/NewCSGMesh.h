#pragma once
#include "SimpleMesh.h"
#include "AABB.h"

namespace CSG
{
    struct CSGMesh :
        public GS::SimpleMesh
    {
        CSGMesh(GS::BaseMesh* pMesh);
        ~CSGMesh(void);

        AABB mAABB;
        int ID;
    };

    GS::BaseMesh* ConverteToBaseMesh(CSGMesh* input);
}


