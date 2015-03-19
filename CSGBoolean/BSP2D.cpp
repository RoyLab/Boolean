#include "precompile.h"
#include "BSP2D.h"
#include <Fade_2D.h>
#include "IsectTriangle.h"
#include "isect.h"

namespace CSG
{
#define CALC_DISTANCE(ds, de, splitCoef, start, end)\
			(ds) = (splitCoef)[0]*(start).x()+(splitCoef)[1]*(start).y()+(splitCoef)[2];\
			(de) = (splitCoef)[0]*(end).x()+(splitCoef)[1]*(end).y()+(splitCoef)[2]

	// 通过向量的方向来划分空间
	// 向量逆时针转90度为法向方向
	inline void CalcLineCoef(ISCutSeg& seg, std::vector<TMP_VInfo>& infos)
	{
		auto &p0 = infos[seg.start->Id].p2;
		auto &p1 = infos[seg.end->Id].p2;
		
		assert(!IsEqual(infos[seg.start->Id].p3, infos[seg.end->Id].p3));

		auto dir = p1-p0;
		double d = dir.x()*p0.y()-dir.y()*p0.x();
		seg.lineCoef[0] = -dir.y();
		seg.lineCoef[1] = dir.x();
		seg.lineCoef[2] = d;
	}

	void SplitSegments(BSPSeg& splitSeg, std::vector<BSPSeg>::iterator cur,  std::vector<BSPSeg>::iterator end, 
		std::vector<BSPSeg>& left, std::vector<BSPSeg>& right)
	{
		auto &splitCoef = splitSeg.lineCoef;
		double ds,de, nx, ny;
		Point2 newPoint;
		BSPSeg tmpSeg;
		for (;cur != end; cur++)
		{
			CALC_DISTANCE(ds, de, splitCoef, cur->start, cur->end);
			if (ds*de < 0.0)
			{
				nx = cur->start.x()*de/(de-ds)+cur->end.x()*ds/(ds-de);
				ny = cur->start.y()*de/(de-ds)+cur->end.y()*ds/(ds-de);
				newPoint.set(nx, ny);
				if (ds > 0.0)
				{
					tmpSeg.lineCoef = cur->lineCoef;

					tmpSeg.start = cur->start;
					tmpSeg.end = newPoint;
					left.push_back(tmpSeg);

					tmpSeg.start = newPoint;
					tmpSeg.end = cur->end;
					right.push_back(tmpSeg);
				}
				else
				{
					tmpSeg.lineCoef = cur->lineCoef;

					tmpSeg.start = cur->start;
					tmpSeg.end = newPoint;
					right.push_back(tmpSeg);

					tmpSeg.start = newPoint;
					tmpSeg.end = cur->end;
					left.push_back(tmpSeg);
				}
			}
			else
			{
				if (ds > 0)	left.push_back(*cur);
				else if (ds < 0) right.push_back(*cur);
				else
				{
					if (de > 0)	left.push_back(*cur);
					else if (de < 0) right.push_back(*cur);
				}
			}
		}
	}


	// left == in, right == out
	BSP2D* BuildBSP2DNode(std::vector<BSPSeg>& segments)
	{
		std::vector<BSPSeg> leftSegs, rigtSegs;
		SplitSegments(segments[0], segments.begin()+1, segments.end(), leftSegs, rigtSegs);
		BSP2D* root = new BSP2D;
		root->line = segments[0].lineCoef;
		
		if (leftSegs.size()) root->leftTree = BuildBSP2DNode(leftSegs);
		else root->leftTree = new BSP2D(BSP2D::IN);

		if (rigtSegs.size()) root->rightTree = BuildBSP2DNode(rigtSegs);
		else root->rightTree = new BSP2D(BSP2D::OUT);
	}

	BSP2D* BuildBSP2D(ISectTriangle* tri, unsigned index, std::vector<TMP_VInfo>& infos)
	{
		// 生成line
		BSPSeg tmpSeg;
		std::vector<BSPSeg> segments;
		auto &segs = tri->segs[index];
		for (auto &seg: segs.segs)
		{
			CalcLineCoef(seg, infos);
			tmpSeg.lineCoef = seg.lineCoef;
			tmpSeg.start = infos[seg.start->Id].p2;
			tmpSeg.end = infos[seg.end->Id].p2;
			segments.push_back(tmpSeg);
		}

		BSP2D* root = BuildBSP2DNode(segments);
	}

	bool SegIsectTest2D(ISCutSeg& seg1, ISCutSeg& seg2, std::vector<TMP_VInfo>& infos, Vec3d& output)
	{
		double ds1, ds2, de1, de2;
		Point2 *ps1, *ps2, *pe1, *pe2;

		ps1 = &(infos[seg1.start->Id].p2);
		pe1 = &(infos[seg1.end->Id].p2);
		CALC_DISTANCE(ds1, de1, seg2.lineCoef, *ps1, *pe1);
		if (ds1*de1 >= 0.0) return false;

		ps2 = &(infos[seg2.start->Id].p2);
		pe2 = &(infos[seg2.end->Id].p2);
		CALC_DISTANCE(ds2, de2, seg1.lineCoef, *ps2, *pe2);
		if (ds2*de2 >= 0.0) return false;

		output = infos[seg1.start->Id].p3*de1/(de1-ds1)+infos[seg1.end->Id].p3*ds1/(ds1-de1);
		return true;
	}
}
