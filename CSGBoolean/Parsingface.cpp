#include "precompile.h"
#include "IsectTriangle.h"
#include "BinaryTree.h"

namespace CSG
{
	void ParsingFace(MPMesh* pMesh, MPMesh::FaceHandle faceHandle, ISectTriangle* triangle, const CSGTree* pTree)
	{
		// 树不为空，存在一个on的节点
		std::vector<int> validMesh;
		for (auto &pair: pTree->Leaves)
		{
			if (pair.first == pMesh->ID) continue;
			validMesh.push_back(pair.first);
		}
	}

}
