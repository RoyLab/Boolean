#include "NewCSGTree.h"
#include "CSGExprNode.h"
#include "NewCSGMesh.h"
#include <vector>
#include <assert.h>

namespace CSG
{
    void ConvertCSGTreeNode(GS::CSGExprNode* input, CSGTreeNode** pRoot, std::vector<CSGMesh*>& meshList)
    {
        if (!input) return;

        CSGTreeNode*& root = *pRoot;
        root = new CSGTreeNode;

        GS::BaseMesh* pBaseMesh = input->GetMesh();
        if (pBaseMesh)
        {
            root->pMesh = new CSGMesh(pBaseMesh);
            root->pMesh->ID = meshList.size();
            meshList.emplace_back(root->pMesh);
        }
        
        switch (input->BoolOperation())
        {
        case GS::CSGUnion:
            root->Operation = OP_UNION;
            break;
        case GS::CSGIntersect:
            root->Operation = OP_INTERSECT;
            break;
        case GS::CSGDiff:
            root->Operation = OP_DIFF;
            break;
        default:
            root->Operation = OP_UNKNOWN;
            break;
        }

        ConvertCSGTreeNode(input->LeftNode(), &root->pLeft, meshList);
        ConvertCSGTreeNode(input->RightNode(), &root->pRight, meshList);
    }


    CSGTree* ConvertCSGTree(GS::CSGExprNode* root, CSGMesh*** arrMesh, int *nMesh) // convert nodes.
    {
        if (!root) return NULL;

        CSGTree* pRes = new CSGTree;
        pRes->pRoot = NULL;
        std::vector<CSGMesh*> meshList;

        ConvertCSGTreeNode(root, &pRes->pRoot, meshList);

        CSGMesh**& pMesh = *arrMesh;
        *nMesh = meshList.size();
        pMesh = new CSGMesh*[*nMesh];
        memcpy(pMesh, meshList.data(), sizeof(CSGMesh*)*(*nMesh));
        return pRes;
    }


}  // namespace CSG

