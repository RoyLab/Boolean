#pragma once
#include "NewCSGTree.h"

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
