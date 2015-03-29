#include "precompile.h"
#include "COctree.h"
#include "MPMesh.h"
#include "isect.h"
#include "Plane.h"
#include "Box3.h"
#include "topology.h"

namespace CSG
{
	static const int MAX_TRIANGLE_COUNT = 30;
	static const int MAX_LEVEL = 9;

	//extern const char POINT_INOUT_TEST_STRING[16] = "PINOUT";

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
    
            for (unsigned i = 0; i < 8 ; i++)
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
			Vec3d *v0, *v1, *v2;

            for (auto &triTab: root->TriangleTable)
            {
                auto &triangles = triTab.second;
                auto pMesh = pOctree->pMesh[triTab.first];

                const unsigned tn = triangles.size();
				int count;
				MPMesh::FaceHandle fhandle;
				MPMesh::FaceVertexIter fvItr;
                for (unsigned i = 0; i < tn; i++)
                {
                    // intersection test, can be optimized!!!
					fhandle = triangles[i];
                    count = 0;

                    for (unsigned j = 0; j < 8; j++)
                    {
                        count = 0;
						fvItr = pMesh->fv_iter(fhandle);
                        v0 = &pMesh->verticesList[fvItr->idx()];	fvItr++;
                        v1 = &pMesh->verticesList[fvItr->idx()];	fvItr++;
                        v2 = &pMesh->verticesList[fvItr->idx()];
                        auto &aabb = root->Child[j].BoundingBox;

                        if (aabb.IsInBox_LORC(*v0)) count++;
                        if (aabb.IsInBox_LORC(*v1)) count++;
                        if (aabb.IsInBox_LORC(*v2)) count++;
                        if (count > 0)
                        {
                            root->Child[j].TriangleTable[triTab.first].push_back(triangles[i]);
                            root->Child[j].TriangleCount++;
                            if (count == 3) break;
                        }
                        else if(TriangleAABBIntersectTest(*v0, *v1, *v2, aabb))
                        {
                            root->Child[j].TriangleTable[triTab.first].push_back(triangles[i]);
                            root->Child[j].TriangleCount++;
                        }
                    }
                }
            }

            for (unsigned i = 0; i < 8; i++)
            {
                for (auto &itr: root->TriangleTable)
                {
                    auto &childTab = root->Child[i].TriangleTable;
                    if (childTab.find(itr.first) == childTab.end())
                        root->Child[i].DiffMeshIndex.emplace_back(itr.first);
                }

                BuildOctree(root->Child+i, pOctree, level+1);
            }
            root->TriangleTable.clear(); // can it?
        }
    }

    Octree* BuildOctree(MPMesh** meshList, unsigned num)
    {
        if (!num) return NULL;

        Octree* pOctree = new Octree;
        pOctree->pMesh = meshList;
        pOctree->nMesh = num;

        OctreeNode*& root = pOctree->Root;
        root = new OctreeNode;
        
        root->BoundingBox.Clear();
		MPMesh *pMesh;
        for (unsigned i = 0; i < num; i++)
        {
			pMesh = meshList[i];
            root->BoundingBox.IncludeBox(pMesh->BBox);

            for (auto fItr = pMesh->faces_begin(); fItr != pMesh->faces_end(); fItr++)
            {
                root->TriangleCount++;
                root->TriangleTable[i].push_back(*fItr);
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

    static int FindFirstNode(double tx0, double ty0, double tz0, double txm, double tym, double tzm)
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

    struct RayCastInfo
    {
        Vec3d pt;
        Vec3d et;
        Vec3d dir;
        int nCross; // -1: Same , -2 : Opposite // 0: pmrOutside
        GS::Plane<double> splane;
    };

    static inline bool PointWithPlane(const Vec3d& normal, double distance,  const Vec3d& pos) 
    {
        double dist = dot(pos, normal)+ distance; 
        if (dist > EPSF )
            return true;
        return false ; 
    }

    inline bool PointWithPlane(const Vec3d& normal, const Vec3d& v0,  const Vec3d& pos)
    {
        double dist =  dot(normal, (pos-v0)); 
        if (dist > EPSF )
            return true;
        return false; 
    }

	static inline bool PointWithPlane(const GS::double3& normal, double distance,  const GS::double3& pos) 
    {
        double dist = dot(pos, normal)+ distance; 
        if (dist > EPSF )
            return true;
        return false ; 
    }

	static inline bool SignDeterminant(const Vec3d& v0, const Vec3d& v1, const Vec3d& v2, const Vec3d& v3)
	{
		return SignDeterminant(GS::double3(v0[0], v0[1], v0[2]),
			GS::double3(v1[0], v1[1], v1[2]),
			GS::double3(v2[0], v2[1], v2[2]),
			GS::double3(v3[0], v3[1], v3[2]));
	}

	static inline void normalize(Vec3d& vec)
	{
		vec = vec/vec.length();
	}


    static int RayFaceTest(RayCastInfo* rayInfo, MPMesh* pMesh, MPMesh::FaceHandle triHandle)
    {
		Vec3d normal = pMesh->normal(triHandle);
		auto fvItr = pMesh->fv_begin(triHandle);
        Vec3d &v0 = pMesh->verticesList[fvItr->idx()];	fvItr++;
        Vec3d &v1 = pMesh->verticesList[fvItr->idx()];	fvItr++;
        Vec3d &v2 = pMesh->verticesList[fvItr->idx()];

        if (PointWithPlane(normal, v0, rayInfo->pt) == PointWithPlane(normal, v0, rayInfo->et) )
            return 0;
        double t, u, v;
        if (!RayTriangleIntersectTest(rayInfo->pt, rayInfo->dir, v0, v1, v2, u, v, t))
            return 0 ; 
        if (t <= EPSF && t >= -EPSF)
        {
            if (dot((rayInfo->et - rayInfo->pt),  normal) < -EPSF) 
            return -2;
        else return -1;
        }
        if (fabs(u) > EPSF &&  fabs(v) > EPSF && fabs(1-u-v) > EPSF)
        return 1;
        bool A0 = PointWithPlane(Double3ToVec3d(rayInfo->splane.Normal()), rayInfo->splane.Distance(), v0);
        bool A1 = PointWithPlane(Double3ToVec3d(rayInfo->splane.Normal()), rayInfo->splane.Distance(), v1);
        bool A2 = PointWithPlane(Double3ToVec3d(rayInfo->splane.Normal()), rayInfo->splane.Distance(), v2);
        bool B0 = SignDeterminant(rayInfo->pt, rayInfo->et, v0, v1);
        bool B1 = SignDeterminant(rayInfo->pt, rayInfo->et, v1, v2);
        bool B2 = SignDeterminant(rayInfo->pt, rayInfo->et, v2, v0);
        int Inte =!A0 && A1&& B0 || A0&& !A1&& !B0;
        Inte^= (!A1 && A2&& B1 || A1&& !A2&& !B1);
        Inte^= (!A2 && A0&& B2 || A2&& !A0&& !B2);
        return Inte;
    }

    static void RayCastThroughNode(OctreeNode* pNode, MPMesh* pMesh, RayCastInfo* rayInfo, unsigned Id)
    {
        if (pNode == NULL) return;

        auto trianglesItr = pNode->TriangleTable.find(pMesh->ID);
        if (trianglesItr == pNode->TriangleTable.end()) return;

        auto &triangles = trianglesItr->second;
        unsigned n = triangles.size();

		MPMesh::FaceHandle fhandle;
        for (unsigned i = 0 ; i < n; i++)
        {
			fhandle = triangles[i];

			if (pMesh->property(pMesh->PointInOutTestPropHandle, fhandle) == Id) continue;
			else pMesh->property(pMesh->PointInOutTestPropHandle, fhandle) = Id;

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
                               OctreeNode* pNode, int& a, MPMesh* pMesh, RayCastInfo* rayInfo, unsigned Id)
    {
        if (pNode == nullptr) return;

        double txm, tym, tzm;
        int currNode;
        if(tx1 < 0 || ty1 < 0 || tz1 < 0) 
            return; 
        if (pNode->Type != NODE_MIDSIDE)
        {
            RayCastThroughNode(pNode, pMesh, rayInfo, Id);
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
                    ProcessSubNode(tx0,ty0,tz0,txm,tym,tzm, &pNode->Child[a], a, pMesh, rayInfo, Id);
                    currNode = GetNextNode(txm,4,tym,2,tzm,1);
                    break;}
                case 1: {
                    ProcessSubNode(tx0,ty0,tzm,txm,tym,tz1, &pNode->Child[1^a], a, pMesh, rayInfo, Id);
                    currNode = GetNextNode(txm,5,tym,3,tz1,8);
                    break;}
                case 2: {
                    ProcessSubNode(tx0,tym,tz0,txm,ty1,tzm, &pNode->Child[2^a], a, pMesh, rayInfo, Id);
                    currNode = GetNextNode(txm,6,ty1,8,tzm,3);
                    break;}
               case 3: {
                    ProcessSubNode(tx0,tym,tzm,txm,ty1,tz1, &pNode->Child[3^a], a, pMesh, rayInfo, Id);
                    currNode = GetNextNode(txm,7,ty1,8,tz1,8);
                    break;}
               case 4: {
                    ProcessSubNode(txm,ty0,tz0,tx1,tym,tzm, &pNode->Child[4^a], a, pMesh, rayInfo, Id);
                    currNode = GetNextNode(tx1,8,tym,6,tzm,5);
                    break;}
              case 5: {
                    ProcessSubNode(txm,ty0,tzm,tx1,tym,tz1, &pNode->Child[5^a], a, pMesh, rayInfo, Id);
                    currNode = GetNextNode(tx1,8,tym,7,tz1,8);
                    break;}
              case 6: {
                    ProcessSubNode(txm,tym,tz0,tx1,ty1,tzm, &pNode->Child[6^a], a, pMesh, rayInfo, Id);
                    currNode = GetNextNode(tx1,8,ty1,8,tzm,7);
                    break;}
            case 7: {
                    ProcessSubNode(txm,tym,tzm,tx1,ty1,tz1,&pNode->Child[7^a], a, pMesh, rayInfo, Id);
                    currNode = 8;
                    break;}
            }
        } while ( currNode < 8 );

    }

    static void RayTraverse(Octree* pOctree,  MPMesh* pMesh, RayCastInfo* rayInfo)
    {
        assert(pOctree);
        AABBmp &RootBBox = pOctree->Root->BoundingBox;
		static unsigned count = 0; // for every new raytraverse a new Id
		count++;

        int a  =0; 
        if (rayInfo->dir[0] < 0.0)
        {
            rayInfo->pt[0] = RootBBox.Size()[0] - rayInfo->pt[0];
            rayInfo->dir[0] = - rayInfo->dir[0];
            a |= 4;
        }
        if (rayInfo->dir[1] < 0.0)
        {
            rayInfo->pt[1] = RootBBox.Size()[1] - rayInfo->pt[1];
           rayInfo-> dir[1] = - rayInfo->dir[1];
            a |= 2;
        }
        if (rayInfo->dir[2] < 0.0)
        {
            rayInfo->pt[2] = RootBBox.Size()[2] - rayInfo->pt[2];
            rayInfo->dir[2] = - rayInfo->dir[2];
            a |= 1;
        }
        Vec3d t0 = (RootBBox.Min() - rayInfo->pt) /rayInfo->dir;
        Vec3d t1 = (RootBBox.Max() - rayInfo->pt) /rayInfo->dir;
        if (GS::max(t0[0], GS::max(t0[1], t0[2])) < GS::min(t1[0], GS::min(t1[1], t1[2])))
            ProcessSubNode(t0[0], t0[1], t0[2], t1[0], t1[1], t1[2], pOctree->Root, a, pMesh, rayInfo, count);
    }
    
    Relation PolyhedralInclusionTest(Vec3d& point, Octree* pOctree, unsigned meshId, bool IsInverse)
    {
        if (!IsInverse && !pOctree->pMesh[meshId]->BBox.IsInBox(point))
            return REL_OUTSIDE;
        else if (IsInverse && !pOctree->pMesh[meshId]->BBox.IsInBox(point))
            return REL_INSIDE;

        AABBmp &bbox = pOctree->Root->BoundingBox;

        RayCastInfo rayInfo;
        rayInfo.nCross = 0;
        rayInfo.pt = point;
        rayInfo.et = bbox.Max() + Vec3d(1,1,1);
        rayInfo.dir = rayInfo.et-point;
		normalize(rayInfo.dir);

        Vec3d edge = rayInfo.et - Vec3d(0.0,bbox.Size()[1]*0.5,0.0) - point;
        Vec3d norm = cross(rayInfo.dir, edge);
		normalize(norm);
		GS::double3 tmp1 = Vec3dToDouble3(norm);
		GS::double3 tmp2 = Vec3dToDouble3(point);
        rayInfo.splane = GS::Plane<double>(tmp1, tmp2);
        RayTraverse(pOctree, pOctree->pMesh[meshId], &rayInfo);

		if (IsInverse)
		{
			switch (rayInfo.nCross)
			{
			case -1:    assert(0); return REL_OPPOSITE;
			case -2:    assert(0); return REL_SAME;
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
    }

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

