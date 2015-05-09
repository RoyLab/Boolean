#include "precompile.h"
#include "IsectTriangle.h"
#include "BinaryTree.h"
#include "BSP2D.h"
#include "BaseMesh.h"
#include "COctree.h"
#include "isect.h"
#include <algorithm>

namespace CSG
{
    extern unsigned long long mark;

	using namespace GEOM_FADE2D;
	typedef std::list<ISCutSeg>::iterator ISCutSegItr;
	int FindMaxIndex(Vec3d& vec);
	void SortSegPoint(std::pair<const ISCutSegItr, std::list<ISVertexItr>>& pair, std::vector<TMP_VInfo>& infos);
	void AddFaceToResult(GS::BaseMesh*, GEOM_FADE2D::Triangle2*, std::vector<TMP_VInfo>&);

	inline bool operator<(const ISCutSegItr& v1, const ISCutSegItr& v2)
	{
		return &*v1 < &*v2;
	}

	inline double cross(const OpenMesh::Vec2d& v0, const OpenMesh::Vec2d& v1)
	{
		return v0[0]*v1[1]-v0[1]*v1[0];
	}

	inline bool IsPointInTriangle(const OpenMesh::Vec2d &bc, OpenMesh::Vec2d* v, Relation& rel)
	{
		double d = cross(v[1]-v[0], bc-v[0]);
		if (cross(v[2]-v[1], bc-v[1])*d < 0) return false;
		if (cross(v[0]-v[2], bc-v[2])*d < 0) return false;
		return true;
	}

	bool IsInsideTriangle(const ISectTriangle* triangle, MPMesh* pMesh, MPMesh::FaceHandle coTri, const Point2 &bc, Relation &rel)
	{
		Vec3d *v0, *v1, *v2;
		GetCorners(pMesh, coTri, v0, v1, v2);
		
		OpenMesh::Vec2d v[3];
		v[0][0] = (*v0)[triangle->xi]; v[0][1] = (*v0)[triangle->yi];
		v[1][0] = (*v1)[triangle->xi]; v[1][1] = (*v1)[triangle->yi];
		v[2][0] = (*v2)[triangle->xi]; v[2][1] = (*v2)[triangle->yi];

		if (IsPointInTriangle(OpenMesh::Vec2d(bc.x(), bc.y()), v, rel))
		{
			double d = OpenMesh::dot(triangle->pMesh->normal(triangle->face), pMesh->normal(coTri));
			if (pMesh->bInverse ^ triangle->pMesh->bInverse) d = -d;
			if (d > 0.0) rel = REL_SAME;
			else rel = REL_OPPOSITE;
			return true;
		}
		else return false;
	}

	void ParsingFace1(MPMesh* pMesh, MPMesh::FaceHandle faceHandle, MPMesh** meshList, std::vector<TMP_VInfo>& points)
    {
		// ����Ϊ�գ�����һ��on�Ľڵ�
		ISectTriangle* triangle = pMesh->property(pMesh->SurfacePropHandle, faceHandle);
		assert(triangle);

		Vec3d normal = pMesh->normal(faceHandle);
		if (pMesh->bInverse) normal = -normal;
		int mainAxis = FindMaxIndex(normal);
		if (normal[mainAxis] > 0.0)
		{
			triangle->xi = (mainAxis+1)%3;
			triangle->yi = (mainAxis+2)%3;
		}
		else
		{
			triangle->yi = (mainAxis+1)%3;
			triangle->xi = (mainAxis+2)%3;
		}

		unsigned n_vertices = triangle->vertices.size();

		// 3D ת 2D ����
		points.reserve(n_vertices+5);
		points.resize(n_vertices);
		size_t count = 0;
		for (auto vertex = triangle->vertices.begin();
			vertex != triangle->vertices.end(); vertex++, count++)
		{
			vertex->Id = count;
			GetLocation(&*vertex, points[count].p3);
			points[count].p2.set(points[count].p3[triangle->xi], points[count].p3[triangle->yi]);
			points[count].p2.setCustomIndex(count);
			points[count].ptr = vertex;
		}

		// ���� BSP ���ռ�
		for (auto &pair: triangle->segs)
			pair.second.bsp = BuildBSP2D(triangle, pair.first,points);
    }

	void ParsingFace(MPMesh* pMesh, MPMesh::FaceHandle faceHandle, const TestTree* pTree,
        Relation testRelation, MPMesh** meshList, std::vector<TMP_VInfo>& points, Octree* pOctree, GS::BaseMesh* pResult)
	{
        if (testRelation == REL_INSIDE) return;

		// ����Ϊ�գ�����һ��on�Ľڵ�
		ISectTriangle* triangle = pMesh->property(pMesh->SurfacePropHandle, faceHandle);
		assert(triangle);
        triangle->relationTestId.clear();
		for (auto &itr: *pTree)
            GetLeafList(itr.testTree, triangle->relationTestId);

		// ȡ����Ч���ཻ�����б�
		std::sort(triangle->relationTestId.begin(), triangle->relationTestId.end());

		// ��¼�����ཻ��
		std::map<ISCutSegItr, std::list<ISVertexItr>> crossRecord;
		unsigned n_test = triangle->relationTestId.size();
		Vec3d crossPoint;
		for (unsigned i = 0; i < n_test; i++)
		{
			auto &res1 = triangle->segs.find(triangle->relationTestId[i]);
            if (res1 == triangle->segs.end()) continue;
            auto &segs1 = res1->second;

			for (unsigned j = i+1; j < n_test; j++)
			{
				auto &res2 = triangle->segs.find(triangle->relationTestId[j]);
                if (res2 == triangle->segs.end()) continue;
                auto &segs2 = res2->second;

				for (auto segItr1 = segs1.segs.begin(); segItr1 != segs1.segs.end(); segItr1++)
				{
					for (auto segItr2 = segs2.segs.begin(); segItr2 != segs2.segs.end(); segItr2++)
					{
						if (SegIsectTest2D(*segItr1, *segItr2, points, crossPoint))
						{
							auto vItr = InsertPoint(triangle, INNER, crossPoint);
							vItr->Id = points.size();
							points.emplace_back();
							points.back().p3 = crossPoint;
							points.back().p2 = Point2(crossPoint[triangle->xi], crossPoint[triangle->yi]);
                            points.back().p2.setCustomIndex(vItr->Id);
							points.back().ptr = vItr;
							crossRecord[segItr2].push_back(vItr);
							crossRecord[segItr1].push_back(vItr);
						}
					}
				}
			}
		}

		// �����ཻ���ԵĽ�����»����ۺ�
		std::list<ISCutSeg> tmpSegList;
		ISCutSeg tmpSeg;
		for (auto& pair: crossRecord)
		{
            if (pair.second.size() > 1) SortSegPoint(pair, points);
			tmpSeg.start = pair.first->start;
			tmpSeg.end = *pair.second.begin();
			tmpSegList.push_back(tmpSeg);

			std::list<ISVertexItr>::iterator vItr = pair.second.begin(), vItr2 = vItr;
			vItr2++;
			for (; vItr2 != pair.second.end(); vItr++, vItr2++)
			{
				tmpSeg.start = *vItr;
				tmpSeg.end = *vItr2;
				tmpSegList.push_back(tmpSeg);
			}

			tmpSeg.start = *(--pair.second.end());
			tmpSeg.end = pair.first->end;
			tmpSegList.push_back(tmpSeg);

			auto &tmpSegs = triangle->segs[pair.first->oppoTriangle->pMesh->ID];
			tmpSegs.segs.erase(pair.first);
			tmpSegs.segs.insert(tmpSegs.segs.end(), tmpSegList.begin(), tmpSegList.end());
			tmpSegList.clear();
		}

		// ����Լ������
		std::vector<Segment2> segList;
		Point2 *p0, *p1;
		unsigned testId;
		for (unsigned i = 0; i < n_test; i++)
		{
			testId = triangle->relationTestId[i];
			auto& res1 = triangle->segs.find(testId);
            if (res1 == triangle->segs.end()) continue;
			auto& segs = res1->second;
			for (auto seg = segs.segs.begin(); seg != segs.segs.end(); seg++)
			{
				p0 = &points[seg->start->Id].p2;
				p1 = &points[seg->end->Id].p2;
				segList.emplace_back(*p0, *p1);
			}
		}
	
		Fade_2D*& dt = triangle->dtZone;
        dt = new Fade_2D;
		dt->insert(points[triangle->corner[0]->Id].p2);
		dt->insert(points[triangle->corner[1]->Id].p2);
		dt->insert(points[triangle->corner[2]->Id].p2);

		dt->createConstraint(segList, GEOM_FADE2D::CIS_IGNORE_DELAUNAY);
		dt->applyConstraintsAndZones();

		std::vector<Triangle2*> vAllTriangles;
		dt->getTrianglePointers(vAllTriangles);

		Point2 baryCenter2d;
		CSGTreeNode* curNode;
        Relation curRelation(REL_UNKNOWN), outRelation(REL_UNKNOWN);
        bool pass;

		for (auto triFrag: vAllTriangles)
		{
			baryCenter2d = triFrag->getBarycenter();
			GS::double3 v[3];
			v[0] = Vec3dToDouble3(points[triFrag->getCorner(0)->getCustomIndex()].p3);
			v[1] = Vec3dToDouble3(points[triFrag->getCorner(1)->getCustomIndex()].p3);
			v[2] = Vec3dToDouble3(points[triFrag->getCorner(2)->getCustomIndex()].p3);

			// ȥ����Щ��С��������
			GS::double3x3 mat(GS::double3(1,1,1), v[2]-v[1], v[2]-v[0]);
			//if (fabs(GS::determinant(mat)) < EPSF) continue;

            pass = true;
			for (auto &test: *pTree)
			{
                curNode = GetFirstNode(test.testTree);
                while (curNode)
                {
                    curRelation = REL_UNKNOWN;
                    testId = curNode->pMesh->ID;
                    auto cop = triangle->coplanarTris.find(testId);
                    if (cop != triangle->coplanarTris.end())
                    {
						for (auto &coTri: cop->second)
						{
							if (IsInsideTriangle(triangle, meshList[testId], coTri, baryCenter2d, curRelation))
								break;
						}
                    }
                    if (curRelation == REL_UNKNOWN)
                    {
                        if (triangle->segs.find(testId) != triangle->segs.end())
                            curRelation = BSP2DInOutTest(triangle->segs[testId].bsp, &baryCenter2d);
                        else
                        {
                            Vec3d bc((Double3ToVec3d(v[0])+Double3ToVec3d(v[1])+Double3ToVec3d(v[2]))/3.0);
                            curRelation = PolyhedralInclusionTest(bc, pOctree, testId, pOctree->pMesh[testId]->bInverse);
                        }
                    }
                    curNode = GetNextNode(curNode, curRelation, outRelation);
                }
                if (!(test.targetRelation & outRelation))
                {
                    pass = false;
                    break;
                }
            }
            mark += 2;

            if (pass)
            {
#ifdef _DEBUG
				countd2 ++;
#endif
				pResult->AddTriangle(v);
            }
        }
        SAFE_RELEASE(dt);
	}

	void GetRelationTable(MPMesh* pMesh, MPMesh::FaceHandle curFace, MPMesh::FaceHandle seedFace, 
		Relation* relationSeed, unsigned nMesh, Octree* pOctree, Relation*& output)
	{
		// �������ڵ����������Σ���δ�һ�������ε�����һ�������εĹ�ϵ�������
		output = new Relation[nMesh];
		memcpy(output, relationSeed, sizeof(Relation)*nMesh);

		if (seedFace == curFace) return;

		ISectTriangle* triSeed = pMesh->property(pMesh->SurfacePropHandle, seedFace);
		ISectTriangle* triCur = pMesh->property(pMesh->SurfacePropHandle, curFace);
		if (!triSeed || (!triSeed->coplanarTris.size() && !triSeed->segs.size()))
		{
			assert(triCur);
			MarkNARelation(triCur, output);
		}
		//else if (!triCur || (!triCur->coplanarTris.size() && !triCur->segs.size()))
  //      {
  //          Vec3d *g[3];
  //          GetCorners(pMesh, curFace, g[0], g[1], g[2]);
  //          auto bc = (*g[0]+*g[1]+*g[2])/3.0;
		//	for (auto& pair: triSeed->segs)
		//		output[pair.first] = PolyhedralInclusionTest(bc, pOctree, pair.first, pOctree->pMesh[pair.first]->bInverse);

  //          for (auto& pair2: triSeed->coplanarTris)
  //          {
  //              if (output[pair2.first] == REL_NOT_AVAILABLE)
  //                  output[pair2.first] = PolyhedralInclusionTest(bc, pOctree, pair2.first, pOctree->pMesh[pair2.first]->bInverse);;
  //          }
  //      }
        else
		{
			int index = TestNeighborIndex(pMesh, seedFace, curFace);
			assert(index != -1);
			Vec3d* v[3];
			unsigned nv = triSeed->vertices.size();
			GetCorners(pMesh, seedFace, v[0], v[1], v[2]); 
			int a = triSeed->xi;
			int b = triSeed->yi;
#ifdef _DEBUG
			Vec3d* u[3];
			GetCorners(pMesh, curFace, u[0], u[1], u[2]); 
#endif
			Point2 p[3];
			p[0].set((*v[0])[a], (*v[0])[b]);
			p[1].set((*v[1])[a], (*v[1])[b]);
			p[2].set((*v[2])[a], (*v[2])[b]);

			a = (index+1)%3;
			b = (index+2)%3;

			Point2 testPoint;
			testPoint.set(p[a].x()+p[b].x()-p[index].x(), p[a].y()+p[b].y()-p[index].y());

			for (auto& pair: triSeed->segs)
				output[pair.first] = BSP2DInOutTest(pair.second.bsp, &testPoint);

            for (auto& pair2: triSeed->coplanarTris)
            {
                if (output[pair2.first] == REL_NOT_AVAILABLE)
                    output[pair2.first] = REL_UNKNOWN;
            }

			if (triCur) MarkNARelation(triCur, output);

            std::vector<unsigned> errorList;
            for (auto& pair2: triSeed->coplanarTris)
            {
                if (output[pair2.first] == REL_UNKNOWN)
                    errorList.push_back(pair2.first);
            }

            if (errorList.size())
            {
                Vec3d *g[3];
                GetCorners(pMesh, curFace, g[0], g[1], g[2]);
                auto bc = (*g[0]+*g[1]+*g[2])/3.0;
                for (auto index: errorList)
                    output[index] = PolyhedralInclusionTest(bc, pOctree, index, pOctree->pMesh[index]->bInverse);
            }
		}
	}

	inline int FindMaxIndex(Vec3d& vec)
	{
		double a = fabs(vec[0]);
		double b = fabs(vec[1]);
		double c = fabs(vec[2]);

		if (a >= b)
		{
			if (a >= c) return 0;
			else return 2;
		}
		else
		{
			if (b >= c) return 1;
			else return 2;
		}
	}

	void SortSegPoint(std::pair<const ISCutSegItr, std::list<ISVertexItr>>& pair, std::vector<TMP_VInfo>& infos)
	{
		// ȷ�����򷽷�
		auto dir = infos[pair.first->end->Id].p3 - infos[pair.first->start->Id].p3;
		int maxIndex = FindMaxIndex(dir);
        assert(dir[maxIndex] != 0.0);
		if (dir[maxIndex] > 0.0)
		{
			// from small to big
			for (auto itri = pair.second.begin(); itri != pair.second.end(); itri++)
			{
				for (auto itrj = itri;; itrj++)
				{
					auto itrnext = itrj; itrnext++;
					if (itrnext == pair.second.end()) break;

					if (infos[(*itrj)->Id].p3[maxIndex] > infos[(*itrnext)->Id].p3[maxIndex])
					{
						auto tmp = *itrj;
						*itrj = *itrnext;
						*itrnext = tmp;
					}

				}
			}
		}
		else
		{
			for (auto itri = pair.second.begin(); itri != pair.second.end(); itri++)
			{
				for (auto itrj = itri;; itrj++)
				{
					auto itrnext = itrj; itrnext++;
					if (itrnext == pair.second.end()) break;

					if (infos[(*itrj)->Id].p3[maxIndex] < infos[(*itrnext)->Id].p3[maxIndex])
					{
						auto tmp = *itrj;
						*itrj = *itrnext;
						*itrnext = tmp;
					}

				}
			}
		}

	}

	void AddFaceToResult(GS::BaseMesh* result, GEOM_FADE2D::Triangle2* tri, std::vector<TMP_VInfo>& infos)
	{
		//countd2 ++;
		GS::double3 v[3];
		v[0] = Vec3dToDouble3(infos[tri->getCorner(0)->getCustomIndex()].p3);
		v[1] = Vec3dToDouble3(infos[tri->getCorner(1)->getCustomIndex()].p3);
		v[2] = Vec3dToDouble3(infos[tri->getCorner(2)->getCustomIndex()].p3);

		// ȥ����Щ��С��������
		GS::double3x3 mat(v[0], v[1], v[2]);
		if (fabs(GS::determinant(mat)) < EPSF) return;

		result->AddTriangle(v);
	}

	ISCutSegData::ISCutSegData():bsp(0){}
	ISCutSegData::~ISCutSegData(){SAFE_RELEASE(bsp);}
}
