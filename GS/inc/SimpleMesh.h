#pragma once
#include <vector>
#include "typedefs.h"
#include "BaseMesh.h"

namespace GS
{
    class BaseMesh;
    typedef vec3<unsigned> uint3;

    struct SimpleMesh
    {
        enum {PARA_NEGATE = 1};
        SimpleMesh(const BaseMesh* mesh, int para = 0)
        {
            int n = (int)mesh->VertexCount();
            for (int i = 0; i < n; i++)
                mVertex.push_back(mesh->Vertex(i).pos);

            int a, b, c;
            switch (para)
            {
            case PARA_NEGATE:
                a = 2; b = 1; c = 0;
                break;
            default:
                a = 0; b = 1; c = 2;
                break;
            }

            n = (int)mesh->PrimitiveCount();
            for (int i = 0; i < n; i++)
            {
                auto &index = mesh->mpMeshImp->mTriangle[i].VertexId;
                mTriangle.emplace_back(index[a], index[b], index[c]);
            }
        }

        virtual ~SimpleMesh(){}

        const std::vector<double3>& Vertex() const {return mVertex;}
        const std::vector<uint3>& Triangle() const {return mTriangle;}

        std::vector<double3> mVertex;
        std::vector<uint3> mTriangle;
    };


}