#pragma once
#include "BinaryTree.h"

namespace CSG
{
    struct CSGMesh;

    struct Blist
    {
        CSGMesh*		pMesh;
		bool			sign;
		int			name;
		int			stamp;
    };
}
