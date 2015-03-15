#include "precompile.h"
#include "IsectTriangle.h"
#include "BinaryTree.h"
#include "BSP2D.h"
#include <algorithm>

namespace CSG
{
	using namespace GEOM_FADE2D;
	typedef std::list<ISCutSeg>::iterator ISCutSegItr;
	int FindMaxIndex(Vec3d& vec);

	void ParsingFace(MPMesh* pMesh, MPMesh::FaceHandle faceHandle, const CSGTree* pTree)
	{
		// 树不为空，存在一个on的节点
		ISectTriangle* triangle = pMesh->property(pMesh->SurfacePropHandle, faceHandle);
		assert(triangle);
		for (auto &pair: pTree->Leaves)
		{
			if (pair.first == pMesh->ID) continue;
			triangle->relationTestId.push_back(pair.first);
		}

		// 取得有效的相交网格列表
		std::sort(triangle->relationTestId.begin(), triangle->relationTestId.end());
		Vec3d normal = pMesh->normal(faceHandle);
		triangle->mainIndex = FindMaxIndex(normal);
		unsigned index = triangle->mainIndex;
		unsigned n_test = triangle->relationTestId.size();

		// 记录可能的二次相交点
		std::map<ISCutSegItr, std::list<ISVertexItr>> crossRecord;
		Vec3d crossPoint;
		for (unsigned i = 0; i < n_test; i++)
		{
			for (unsigned j = i; j < n_test; j++)
			{
				auto &segs1 = triangle->segs[i];
				auto &segs2 = triangle->segs[j];

				for (auto segItr1 = segs1.begin(); segItr1 != segs1.end(); segItr1++)
				{
					for (auto segItr2 = segs2.begin(); segItr2 != segs2.end(); segItr2++)
					{
						if (SegInterSectionTest2D(*segItr1, *segItr2, index, &crossPoint))
						{
							auto vItr = InsertPoint(triangle, INNER, crossPoint);
							crossRecord[segItr2].push_back(vItr);
							crossRecord[segItr1].push_back(vItr);
						}
					}
				}
			}
		}

		// 根据相交测试的结果重新划分折痕
		std::list<ISCutSeg> tmpSegList;
		ISCutSeg tmpSeg;
		for (auto& pair: crossRecord)
		{
			SortSegPoint(pair.first, pair.second);
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

			tmpSeg.start = *pair.second.end();
			tmpSeg.start--;
			tmpSeg.end = pair.first->end;
			tmpSegList.push_back(tmpSeg);

			auto &tmpSegs = triangle->segs[pair.first->oppoTriangle->pMesh->ID];
			tmpSegs.erase(pair.first);
			tmpSegs.emplace(tmpSegList.begin(), tmpSegList.end());
			tmpSegList.clear();
		}


		// 3D 转 2D 坐标
		unsigned n_vertices = triangle->vertices.size();
		GEOM_FADE2D::Point2* point = new GEOM_FADE2D::Point2[n_vertices];
		Vec3d* point3d = new Vec3d[n_vertices];

		unsigned count = 0;
		for (auto vertex = triangle->vertices.begin();
			vertex != triangle->vertices.end(); vertex++, count++)
		{
			vertex->Id = count;
			GetLocation(&*vertex, point3d[count]);
			point[count].x =  point3d[count][(index+1)%3];
			point[count].y =  point3d[count][(index+2)%3];
			point[count].setCustomIndex(count);
		}

		// 建立 BSP 树空间
		triangle->bsp = new BSP2D*[n_test];
		for (unsigned i = 0; i < n_test; i++)
			triangle->bsp[i] = BuildBSP2D(triangle, i);

		// 建立约束条件
		std::vector<Segment2> segList;
		Point2 *p0, *p1;
		unsigned testId;
		Relation r0, r1;
		for (unsigned i = 0; i < n_test; i++)
		{
			testId = triangle->relationTestId[i];
			auto& segs = triangle->segs[testId];
			for (auto seg = segs.begin(); seg != segs.end(); seg++)
			{
				p0 = &point[seg->start->Id];
				p1 = &point[seg->end->Id];
				segList.emplace_back(*p0, *p1);
			}
		}
	
		Fade_2D* dt = triangle->dtZone;
		dt = new Fade_2D;
		dt->insert(point[n_vertices-3]);
		dt->insert(point[n_vertices-2]);
		dt->insert(point[n_vertices-1]);

		dt->createConstraint(segList, GEOM_FADE2D::CIS_IGNORE_DELAUNAY);
		dt->applyConstraintsAndZones();

		std::vector<Triangle2*> vAllTriangles;
		dt->getTrianglePointers(vAllTriangles);

		Point2 baryCenter2d;
		CSGTree* tmpTree;
		unsigned i;
		Relation tmpRel;
		for (auto triFrag: vAllTriangles)
		{
			baryCenter2d = triFrag->getBarycenter();
			tmpTree = copy(pTree);
			i = 0;
			for (auto testId: triangle->relationTestId)
			{
				if (tmpTree->Leaves.find(testId) != tmpTree->Leaves.end())
				{
					CompressCSGTree(tmpTree, testId, BSP2DInOutTest(triangle->bsp[i], &baryCenter2d));
					if (tmpTree->Leaves.size() <= 1)
					{
						if (tmpTree->Leaves.find(pMesh->ID) == tmpTree->Leaves.end()) break;
						else AddFaceToResult(triangle, triFrag);
					}
				}
				i++;
			}
			delete tmpTree;
		}
	}

	void GetRelationTable(MPMesh* pMesh, MPMesh::FaceHandle curFace, MPMesh::FaceHandle seedFace, Relation* relationSeed, unsigned nMesh, Relation*& output)
	{
		// 分情况讨论： 子集，超集，混合集
		// TO-DO: 共面的检测
		output = new Relation[nMesh];
		memcpy(output, relationSeed, sizeof(Relation)*nMesh);

		ISectTriangle* triSeed = pMesh->property(pMesh->SurfacePropHandle, seedFace);
		ISectTriangle* triCur = pMesh->property(pMesh->SurfacePropHandle, curFace);
		if (!triSeed)
		{
			assert(triCur);
			for (auto& pair: triCur->segs)
				output[pair.first] = REL_NOT_AVAILABLE;
		}
		else
		{
			if (triCur)
			{
				for (auto& pair: triCur->segs)
					output[pair.first] = REL_NOT_AVAILABLE; // available

				Vec3d *v0, *v1, *v2;
				GetCorners(pMesh, curFace, v0, v1, v2);
				Vec3d baryCenter = (*v0+*v1+*v2)/3.0;
				for (auto& pair: triSeed->segs)
					if (output[pair.first] != REL_NOT_AVAILABLE)
					{
						BSP2DInOutTest
					}
			}
		}
	}

	inline int FindMaxIndex(Vec3d& vec)
	{
		int a = fabs(vec[0]);
		int b = fabs(vec[1]);
		int c = fabs(vec[2]);

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

}
