#include "NewOctree.h"
#include "NewCSGMesh.h"
#include <assert.h>
#include "typedefs.h"

#define MAX_TRIANGLE_COUNT 17

namespace CSG
{
    typedef unsigned uint;

    void BuildOctree(OctreeNode* root, Octree* pOctree)
    {
        assert(root && pOctree);

        if (root->TriangleTable.size() < 2)
            root->Type = NODE_SIMPLE;
        else if (root->TriangleCount > MAX_TRIANGLE_COUNT)
        {
            root->Type = NODE_MIDSIDE;
            root->Child = new OctreeNode[8];

            double3 minOffset, maxOffset;
            AABB &bbox = root->BoundingBox;
            double3 step = bbox.Diagonal()*0.5;
    
            for (uint i = 0; i < 8 ; i++)
            {
                auto pChild = &root->Child[i];

                maxOffset.z = i & 1 ?  0 : -step.z; 
                maxOffset.y = i & 2 ?  0 : -step.y;
                maxOffset.x = i & 4 ?  0 : -step.x;
                minOffset.z = i & 1 ?  step.z : 0; 
                minOffset.y = i & 2 ?  step.y : 0;
                minOffset.x = i & 4 ?  step.x : 0;
                pChild->BoundingBox.Set(bbox.Min() + minOffset, 
                    bbox.Max()+ maxOffset);

                pChild->Parent = root;
            }

            for (auto &triTab: root->TriangleTable)
            {
                auto &triangles = triTab.second;
                auto pMesh = pOctree->pMesh[triTab.first->ID];

                const uint tn = triangles.size();
                for (uint i = 0; i < tn; i++)
                {
                    // intersection test, can be optimized!!!
                    auto &tri = pMesh->mTriangle[triangles[i]];
                    int count = 0;

                    for (uint j = 0; j < 8; j++)
                    {
                        count = 0;
                        if (root->Child[j].BoundingBox.IsInBox_LORC(pMesh->mVertex[tri.VertexIndex[0]])) count++;
                        if (root->Child[j].BoundingBox.IsInBox_LORC(pMesh->mVertex[tri.VertexIndex[1]])) count++;
                        if (root->Child[j].BoundingBox.IsInBox_LORC(pMesh->mVertex[tri.VertexIndex[2]])) count++;
                        if (count > 0)
                        {
                            root->Child[j].TriangleTable[pMesh].push_back(triangles[i]);
                            root->Child[j].TriangleCount++;
                            if (count == 3) break;
                        }
                    }
                }
            }

            for (uint i = 0; i < 8; i++)
            {
                for (auto &itr: root->TriangleTable)
                {
                    auto &childTab = root->Child[i].TriangleTable;
                    if (childTab.find(itr.first) == childTab.end())
                        root->Child[i].DiffMeshIndex.emplace_back(itr.first->ID);
                }

                BuildOctree(root->Child+i, pOctree);
            }
        }
        else root->Type = NODE_COMPOUND;
    }


    Octree* BuildOctree(CSGMesh** meshList, unsigned num, int** reltab)
    {
        if (!num) return NULL;

        Octree* pOctree = new Octree;
        pOctree->pMesh = meshList;
        pOctree->nMesh = num;
        pOctree->ppRelationTable = reltab;

        OctreeNode*& root = pOctree->Root;
        root = new OctreeNode;
        
        root->BoundingBox.Clear();
        for (uint i = 0; i < num; i++)
        {
            root->BoundingBox.IncludeBox(meshList[i]->mAABB);
            uint tn = meshList[i]->mTriangle.size();
            for (uint j = 0; j < tn; j++)
            {
                root->TriangleCount++;
                root->TriangleTable[meshList[i]].push_back(j);
            }
        }

        if (!root->TriangleTable.size())
        {
            delete pOctree;
            return NULL;
        }

        // make sure every triangle is inside the box.
        root->BoundingBox.Enlarge(0.1);

        // start recursion
        BuildOctree(root, pOctree);

        return pOctree;
    }

    Relation PolyhedralInclusionTest(GS::double3& point, Octree* pOctree)
    {
        return REL_INSIDE;
    }



} // namespace CSG
