#include "precompile.h"
#include "COctree.h"
#include "MPMesh.h"
#include "isect.h"


namespace CSG
{
	static const int MAX_TRIANGLE_COUNT = 25;
	static const int MAX_LEVEL = 8;

    static void BuildOctree(OctreeNode* root, Octree* pOctree, int level)
    {
        assert(root && pOctree);

        if (root->TriangleCount <= MAX_TRIANGLE_COUNT || level > MAX_LEVEL)
		{
			if (root->TriangleTable.size() > 1)
				root->Type = NODE_COMPOUND;
			else root->Type = NODE_SIMPLE;
		}
        else
        {
			if (root->TriangleTable.size() > 1)
				root->Type = NODE_MIDSIDE;
			else root->Type = NODE_SIMPLE;

            root->Child = new OctreeNode[8];

            Vec3d minOffset, maxOffset;
            AABBmp &bbox = root->BoundingBox;
            Vec3d step = bbox.Diagonal()*0.5;
    
            for (uint i = 0; i < 8 ; i++)
            {
                auto pChild = &root->Child[i];

                maxOffset[2] = i & 1 ?  0 : -step[2]; 
                maxOffset[1] = i & 2 ?  0 : -step[1];
                maxOffset[0] = i & 4 ?  0 : -step[0];
                minOffset[2] = i & 1 ?  step[2] : 0; 
                minOffset[1] = i & 2 ?  step[1] : 0;
                minOffset[0] = i & 4 ?  step[0] : 0;
                pChild->BoundingBox.Set(bbox.Min() + minOffset, 
                    bbox.Max()+ maxOffset);

                pChild->Parent = root;
            }

            for (auto &triTab: root->TriangleTable)
            {
                auto &triangles = triTab.second;
                auto pMesh = pOctree->pMesh[triTab.first];

                const uint tn = triangles.size();
				int count;
				MPMesh::FaceHandle fhandle;
				MPMesh::FaceVertexIter fvItr;
                for (uint i = 0; i < tn; i++)
                {
                    // intersection test, can be optimized!!!
					fhandle = pMesh->face_handle(triangles[i]);
                    count = 0;

                    for (uint j = 0; j < 8; j++)
                    {
                        count = 0;
						fvItr = pMesh->fv_iter(fhandle);
                        auto &v0 = pMesh->point(*fvItr);	fvItr++;
                        auto &v1 = pMesh->point(*fvItr);	fvItr++;
                        auto &v2 = pMesh->point(*fvItr);
                        auto &aabb = root->Child[j].BoundingBox;

                        if (aabb.IsInBox_LORC(v0)) count++;
                        if (aabb.IsInBox_LORC(v1)) count++;
                        if (aabb.IsInBox_LORC(v2)) count++;
                        if (count > 0)
                        {
                            root->Child[j].TriangleTable[triTab.first].push_back(triangles[i]);
                            root->Child[j].TriangleCount++;
                            if (count == 3) break;
                        }
                        else if(TriangleAABBIntersectTest(v0, v1, v2, aabb))
                        {
                            root->Child[j].TriangleTable[triTab.first].push_back(triangles[i]);
                            root->Child[j].TriangleCount++;
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
                        root->Child[i].DiffMeshIndex.emplace_back(itr.first);
                }

                BuildOctree(root->Child+i, pOctree, level+1);
            }
        }
    }


    Octree* BuildOctree(MPMesh** meshList, uint num)
    {
        if (!num) return NULL;

        Octree* pOctree = new Octree;
        pOctree->pMesh = meshList;
        pOctree->nMesh = num;

        OctreeNode*& root = pOctree->Root;
        root = new OctreeNode;
        
        root->BoundingBox.Clear();
		MPMesh *pMesh;
        for (uint i = 0; i < num; i++)
        {
			pMesh = meshList[i];
            root->BoundingBox.IncludeBox(pMesh->BBox);

            for (auto fItr = pMesh->faces_begin(); fItr != pMesh->faces_end(); fItr++)
            {
                root->TriangleCount++;
                root->TriangleTable[i].push_back(fItr->idx());
            }
        }

        if (!root->TriangleTable.size())
        {
            delete pOctree;
            return NULL;
        }

        // make sure every triangle is inside the box.
        root->BoundingBox.Enlarge(0.001);

        // start recursion
        BuildOctree(root, pOctree, 0);

        return pOctree;
    }


    /*static int FindFirstNode(double tx0, double ty0, double tz0, double txm, double tym, double tzm)
    {
        unsigned char answer = 0; // initialize to 00000000
        // select the entry plane and set bits
        if(tx0 > ty0){
            if(tx0 > tz0){ // PLANE YZ
                if(tym < tx0) answer|=2; // set bit at position 1
                if(tzm < tx0) answer|=1; // set bit at position 0 			
                return (int) answer; 
            } 
        } 
        else if(ty0 > tz0){ // PLANE XZ
            if(txm < ty0) answer|=4; // set bit at position 2
            if(tzm < ty0) answer|=1; // set bit at position 0
                return (int) answer;
       
        }
        // PLANE XY
        if(txm < tz0) answer|=4;   // set bit at position 2
        if(tym < tz0) answer|=2;   // set bit at position 1
        return (int) answer;
    }

    static int GetNextNode(double txm, int x, double tym, int y, double tzm, int z)
    {
        if(txm < tym){
            if(txm < tzm){return x;}  // YZ plane
        }
        else{
            if(tym < tzm){return y;} // XZ plane
        }
        return z; // XY plane;
    }


    static inline bool PointWithPlane(const double3& normal, double distance,  const double3& pos) 
    {
        double dist = dot(pos, normal)+ distance; 
        if (dist > EPSF )
            return true;
        return false ; 
    }

    inline bool PointWithPlane(const double3& normal, const double3& v0,  const double3& pos)
    {
        double dist =  dot(normal, (pos-v0)); 
        if (dist > EPSF )
            return true;
        return false; 
    }

    struct RayCastInfo
    {
        double3 pt;
        double3 et;
        double3 dir;
        int nCross; // -1: Same , -2 : Opposite // 0: pmrOutside
        Plane<double> splane;
    };

    static int RayFaceTest(RayCastInfo* rayInfo, CSGMesh* pMesh, uint triId)
    {
        auto &triangle = pMesh->mTriangle[triId];
        double3& v0 = pMesh->mVertex[triangle.VertexIndex[0]];
        double3& v1 = pMesh->mVertex[triangle.VertexIndex[1]];
        double3& v2 = pMesh->mVertex[triangle.VertexIndex[2]];
        if (PointWithPlane(triangle.Normal, v0, rayInfo->pt) == PointWithPlane(triangle.Normal, v0, rayInfo->et) )
            return 0;
        double t, u, v;
        if (!RayTriangleIntersectTest(rayInfo->pt, rayInfo->dir, v0, v1, v2, u, v, t))
            return 0 ; 
        if (t <= EPSF && t >= -EPSF)
        {
            if (dot((rayInfo->et - rayInfo->pt),  triangle.Normal) < -EPSF) 
            return -2;
        else return -1;
        }
        if (fabs(u) > EPSF &&  fabs(v) > EPSF && fabs(1-u-v) > EPSF)
        return 1;
        bool A0 = PointWithPlane(rayInfo->splane.Normal(), rayInfo->splane.Distance(), v0);
        bool A1 = PointWithPlane(rayInfo->splane.Normal(), rayInfo->splane.Distance(), v1);
        bool A2 = PointWithPlane(rayInfo->splane.Normal(), rayInfo->splane.Distance(), v2);
        bool B0 = SignDeterminant(rayInfo->pt, rayInfo->et, v0, v1);
        bool B1 = SignDeterminant(rayInfo->pt, rayInfo->et, v1, v2);
        bool B2 = SignDeterminant(rayInfo->pt, rayInfo->et, v2, v0);
        int Inte =!A0 && A1&& B0 || A0&& !A1&& !B0;
        Inte^= (!A1 && A2&& B1 || A1&& !A2&& !B1);
        Inte^= (!A2 && A0&& B2 || A2&& !A0&& !B2);
        return Inte;
    }


    static void RayCastThroughNode(OctreeNode* pNode, CSGMesh* pMesh, RayCastInfo* rayInfo, std::set<uint>& triSet)
    {
        if (pNode == NULL) return;

        auto trianglesItr = pNode->TriangleTable.find(pMesh->ID);
        if (trianglesItr == pNode->TriangleTable.end()) return;

        auto &triangles = trianglesItr->second;
        uint n = triangles.size();

        for (uint i = 0 ; i < n; i++)
        {
            if (!triSet.insert(triangles[i]).second) continue;
            int ret = RayFaceTest(rayInfo, pMesh, triangles[i]);
            //Point on the face
            if (ret< 0 )
            {
                rayInfo->nCross = ret;
                return;  
            }
            else rayInfo->nCross ^= ret;
        }

    }

    static void ProcessSubNode(double tx0, double ty0, double tz0, double tx1, double ty1, double tz1, 
                               OctreeNode* pNode, int& a, CSGMesh* pMesh, RayCastInfo* rayInfo, std::set<uint>& triSet)
    {
        if (pNode == nullptr) return;

        double txm, tym, tzm;
        int currNode;
        if(tx1 < 0 || ty1 < 0 || tz1 < 0) 
            return; 
        if (pNode->Type != NODE_MIDSIDE)
        {
            RayCastThroughNode(pNode, pMesh, rayInfo, triSet);
            return ; 
        }

        txm = 0.5*(tx0 + tx1); 
        tym = 0.5*(ty0 + ty1); 
        tzm = 0.5*(tz0 + tz1); 
        currNode = FindFirstNode(tx0,ty0,tz0,txm,tym,tzm); 
        do{ 
            if (rayInfo->nCross < 0) return;

            switch (currNode) 
            { 
                case 0: {
                    ProcessSubNode(tx0,ty0,tz0,txm,tym,tzm, &pNode->Child[a], a, pMesh, rayInfo, triSet);
                    currNode = GetNextNode(txm,4,tym,2,tzm,1);
                    break;}
                case 1: {
                    ProcessSubNode(tx0,ty0,tzm,txm,tym,tz1, &pNode->Child[1^a], a, pMesh, rayInfo, triSet);
                    currNode = GetNextNode(txm,5,tym,3,tz1,8);
                    break;}
                case 2: {
                    ProcessSubNode(tx0,tym,tz0,txm,ty1,tzm, &pNode->Child[2^a], a, pMesh, rayInfo, triSet);
                    currNode = GetNextNode(txm,6,ty1,8,tzm,3);
                    break;}
               case 3: {
                    ProcessSubNode(tx0,tym,tzm,txm,ty1,tz1, &pNode->Child[3^a], a, pMesh, rayInfo, triSet);
                    currNode = GetNextNode(txm,7,ty1,8,tz1,8);
                    break;}
               case 4: {
                    ProcessSubNode(txm,ty0,tz0,tx1,tym,tzm, &pNode->Child[4^a], a, pMesh, rayInfo, triSet);
                    currNode = GetNextNode(tx1,8,tym,6,tzm,5);
                    break;}
              case 5: {
                    ProcessSubNode(txm,ty0,tzm,tx1,tym,tz1, &pNode->Child[5^a], a, pMesh, rayInfo, triSet);
                    currNode = GetNextNode(tx1,8,tym,7,tz1,8);
                    break;}
              case 6: {
                    ProcessSubNode(txm,tym,tz0,tx1,ty1,tzm, &pNode->Child[6^a], a, pMesh, rayInfo, triSet);
                    currNode = GetNextNode(tx1,8,ty1,8,tzm,7);
                    break;}
            case 7: {
                    ProcessSubNode(txm,tym,tzm,tx1,ty1,tz1,&pNode->Child[7^a], a, pMesh, rayInfo, triSet);
                    currNode = 8;
                    break;}
            }
        } while ( currNode < 8 );

    }

    static void RayTraverse(Octree* pOctree,  CSGMesh* pMesh, RayCastInfo* rayInfo)
    {
        assert(pOctree);
        AABB &RootBBox = pOctree->Root->BoundingBox;
        std::set<uint> triSet;

        int a  =0; 
        if (rayInfo->dir.x < 0.0)
        {
            rayInfo->pt.x = RootBBox.Size().x - rayInfo->pt.x;
            rayInfo->dir.x = - rayInfo->dir.x;
            a |= 4;
        }
        if (rayInfo->dir.y < 0.0)
        {
            rayInfo->pt.y = RootBBox.Size().y - rayInfo->pt.y;
           rayInfo-> dir.y = - rayInfo->dir.y;
            a |= 2;
        }
        if (rayInfo->dir.z < 0.0)
        {
            rayInfo->pt.z = RootBBox.Size().z - rayInfo->pt.z;
            rayInfo->dir.z = - rayInfo->dir.z;
            a |= 1;
        }
        double3 t0 = (RootBBox.Min() - rayInfo->pt) /rayInfo->dir;
        double3 t1 = (RootBBox.Max() - rayInfo->pt) /rayInfo->dir;
        if (max(t0.x, max(t0.y, t0.z)) < min(t1.x, min(t1.y, t1.z)))
            ProcessSubNode(t0.x, t0.y, t0.z, t1.x, t1.y, t1.z, pOctree->Root, a, pMesh, rayInfo, triSet);
    }
    
    Relation PolyhedralInclusionTest(GS::double3& point, Octree* pOctree, uint meshId, bool IsInverse)
    {
        if (!pOctree->pMesh[meshId]->mAABB.IsInBox(point))
            return REL_OUTSIDE;

        AABB &bbox = pOctree->Root->BoundingBox;

        RayCastInfo rayInfo;
        rayInfo.nCross = 0;
        rayInfo.pt = point;
        rayInfo.et = bbox.Max() + double3(1,1,1);
        rayInfo.dir = GS::normalize(rayInfo.et-point);

        double3 edge = rayInfo.et - double3(0.0,bbox.Size().y*0.5,0.0) - point;
        double3 norm = GS::normalize(GS::cross(rayInfo.dir, edge));
        rayInfo.splane = Plane<double>(norm, point);
        RayTraverse(pOctree, pOctree->pMesh[meshId], &rayInfo);

		if (IsInverse)
		{
			switch (rayInfo.nCross)
			{
			case -1:    return REL_OPPOSITE;
			case -2:    return REL_SAME;
			case 1:     return REL_OUTSIDE;
			default:    return REL_INSIDE;
			}
		}

        switch (rayInfo.nCross)
        {
        case -1:    return REL_SAME;
        case -2:    return REL_OPPOSITE;
        case 1:     return REL_INSIDE;
        default:    return REL_OUTSIDE;
        }
    }*/

	OctreeNode::OctreeNode():
		Child(0), Parent(0), TriangleCount(0)
		, pRelationData(nullptr)
	{
	}

	OctreeNode::~OctreeNode()
	{
		if (pRelationData)
		{
			//if (Type == NODE_SIMPLE)
			//	delete (SimpleData*)pRelationData;
			//else delete (ComplexData*)pRelationData;
		}

		if (Child)
		{
			delete [] Child;
			Child = nullptr;
		}

	}

	Octree::Octree():
		Root(0), pMesh(0), nMesh(0)
	{

	}

	Octree::~Octree()
	{
		SAFE_RELEASE(Root);
	}


} // namespace CSG

