#define _USE_MATH_DEFINES 
#include <cmath>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_conformer_2.h>
#include "hashlookup.h"
#include "Surface.h"
#include "BaseMesh.h"
#include "CSGMesh.h"
#include "topology.h"
#include "tbb\tick_count.h"
#include "Fade_2D.h"



namespace GS{


template<typename Precision> 
Surface<Precision>::Surface(BaseMesh* pOwner, std::vector<int>& TriIdList)
 :mpOwner(pOwner)
 ,mTriIdList(TriIdList)
 ,mbHasConstraint(false)
 ,mbConcave(true)
 ,mbTriangulated(true)
 ,mVertexCapacity(INIT_VERTEX_CAPACITY)
 ,mSegCapacity(INIT_SEG_CAPACITY)
{
    mVertex.reserve(mVertexCapacity);
    mSeg.reserve(mSegCapacity);
}

/*

procedure TSurface.AssignFrom(aSurface: TSurface);
var
  i: integer;
  Pair: TPair<TPoint2D, integer>;
  Index: TIndexPair;
begin
  ClearAll;

  for i in aSurface.FTriId do
    FTriId.Add(i);

  for i := 0 to aSurface.FSeg.Count - 1 do
    FSeg.Add(TSeg.Create(aSurface.FSeg[i]));

  for i := 0 to aSurface.FOuterLine.Count - 1 do
    FOuterLine.Add(TSeg.Create(aSurface.FOuterLine[i]));

  for i := 0 to aSurface.FVertex.Count - 1 do
    FVertex.Add(TSegVertex.Create(aSurface.FVertex[i]));

  for Pair in aSurface.FVertexDict do
    FVertexDict.AddOrSet(Pair.Key, Pair.Value);

  for Index in aSurface.FSegIndexHash.Keys do
    FSegIndexHash.AddOrSetValue(Index, True);

  for i := 0 to FSeg.Count - 1 do
  begin
    if aSurface.FSeg[i].MeshID = aSurface.FOwner.FID then
      FSeg[i].MeshID := FOwner.ID;
  end;

  eU := aSurface.eU;
  eV := aSurface.eV;
  eN := aSurface.eN;
  Origin := aSurface.Origin;

  FAABB.AssignFrom(aSurface.FAABB);
  FAABB2D := aSurface.FAABB2D;
  FHasConstraint := aSurface.FHasConstraint;
  FIsConcave := aSurface.FIsConcave;
  FTag := aSurface.FTag;
end;
*/

template<typename Precision>
Surface<Precision>::Surface(BaseMesh* pOwner, const Surface<Precision>& surface)
	:mpOwner(pOwner)
	,mSeg(surface.mSeg)
	,mSegIndexHash(surface.mSegIndexHash)
	,mOutline(surface.mOutline)
	,mVertex(surface.mVertex)
	,mVertexDict(surface.mVertexDict)
	,mTriIdList(surface.mTriIdList)
	,mAABB3D(surface.mAABB3D)
	,mAABB2D(surface.mAABB2D)
	,mbHasConstraint(surface.mbHasConstraint)
	,mbConcave(surface.mbConcave)
	,mbTriangulated(surface.mbTriangulated)
	,mColor(surface.mColor)
    ,mVertexCapacity(surface.mVertexCapacity)
    ,mSegCapacity(surface.mSegCapacity)
{

    for (int i = 0; i< mSeg.size(); i++)
	{
		if(surface.mSeg[i].MeshID == surface.mpOwner->GetID())
			mSeg[i].MeshID = mpOwner->GetID();
	}
	mU      = surface.mU;
	mV      = surface.mV;
    mN      = surface.mN;
    mOrigin = surface.mOrigin;
	mNormal = surface.mNormal;
	mnTag   = surface.mnTag;
}


template<typename Precision> 
Surface<Precision>::~Surface()
{
	Clear();
}

template<typename Precision> 
void  Surface<Precision>::CheckVertexCapacity()
{
 
	if ( mVertex.size() >= mVertexCapacity)
	{
		mVertexCapacity *=2;
		mVertex.reserve(mVertexCapacity);
	}
}

template<typename Precision> 
void  Surface<Precision>::CheckSegCapacity()
{
 
	if ( mSeg.size() >= mSegCapacity)
	{
		mSegCapacity *=2;
		mSeg.reserve(mSegCapacity);
	}
}


template<typename Precision> 
void Surface<Precision>::GenerateAABB()
{
	mAABB3D.Clear();
	for (int i = 0 ; i < mTriIdList.size(); i++)
	{
		const TriInfo& info = mpOwner->TriangleInfo(mTriIdList[i]);
		mAABB3D.IncludePoint(mpOwner->Vertex(info.VertexId[0]).pos);
		mAABB3D.IncludePoint(mpOwner->Vertex(info.VertexId[1]).pos);
		mAABB3D.IncludePoint(mpOwner->Vertex(info.VertexId[2]).pos);
	}
	mAABB2D.Clear();
	for (int i = 0 ; i < mVertex.size(); i++)
	{
		float2 pos; 
		pos.x = mVertex[i].Pos.x;
		pos.y = mVertex[i].Pos.y;
		mAABB2D.AddPoint(pos);
	}
}


template<typename Precision> 
int  Surface<Precision>::AddUniqueVertex(const vec2<Precision>& point2D )
{
	std::map<vec2<Precision>, int, Vec2LessThan<Precision>>::iterator  it;
	if ((it =mVertexDict.find(point2D) ) == mVertexDict.end())
	{
        CheckVertexCapacity();
		SegVertex segVertex;
		segVertex.Pos = point2D;
		int index = mVertex.size();
		mVertex.push_back(segVertex);
        mVertexDict[point2D] = index;
		return index;
	}
	return it->second;
}

template<typename Precision> 
bool Surface<Precision>::IsCCW() const 
{

    std::vector<vec2<Precision> > vertices;
	int first =  mOutline[0].VId[0];
    int current = first ;
    vertices.reserve(mOutline.size());
    for (;;)
    { 
        vertices.push_back(mVertex[current].Pos);
        current =  mVertex[current].Outer_NextId;
        if (current == -1 || current == first)
            break;
    }
    // find y max 
    int k = 0 ; 
    Precision ymax = vertices[0].y;
    int n = vertices.size();
    for (int i = 1; i < vertices.size() ; i++)
    {
        if ((vertices[i].y - ymax) > EPSF)
        {
            k = i; 
            ymax = vertices[i].y;
        }
    }

    int prevk = (k -1 + n )% n;
    int nextk = (k+1 +n)% n;
    Precision s = cross(vertices[k] - vertices[prevk], vertices[k] - vertices[nextk]);
   // vec2<Precision> p = (vertices[prevk] + vertices[nextk])*0.5;
    if (IsEqual(s , (Precision)0., (Precision)EPSF))
    {
        vec2<Precision> base(0., 1.0);
        vec2<Precision>  p = vertices[k] - length(vertices[k] - vertices[prevk]) *base;
        s = cross(p - vertices[prevk], p - vertices[nextk]);
    } 
   
    if ( s >= EPSF) 
        return false ;
    return true;
         

}



template<typename Precision> 
void Surface<Precision>::Clear()
{
	mSeg.clear();
	mOutline.clear();
	mVertex.clear();
	mTriIdList.clear();
	mVertexDict.clear();
	mbTriangulated = false ; 
    mVertexCapacity=INIT_VERTEX_CAPACITY;
    mSegCapacity=INIT_SEG_CAPACITY;
   
}


template<typename Precision> 
bool Surface<Precision>::DoCheckIsConcave()
{
	if(!mOutline.size())
	 return false ;
	for (int i = 0 ; i< mOutline.size(); i++)
		mVertex[mOutline[i].VId[0]].Outer_NextId = mOutline[i].VId[1];
	int NowId = mOutline[0].VId[0];
	if ((NowId < 0) || (NowId >= mVertex.size()))
		return false ;
    if (mVertex.size() == 3)
        return true;
    if (mVertex.size() == 4)
    {

        int bIdx = mVertex[NowId].Outer_NextId; 
        int cIdx= mVertex[bIdx].Outer_NextId;
        int dIdx= mVertex[cIdx].Outer_NextId;
      
        vec2<Precision> db = mVertex[dIdx].Pos  - mVertex[bIdx].Pos; 
        vec2<Precision> ab = mVertex[NowId].Pos - mVertex[bIdx].Pos;
        vec2<Precision> cb = mVertex[cIdx].Pos  - mVertex[bIdx].Pos;
        vec2<Precision> dir1 =  cross (db, ab);
        vec2<Precision> dir2 = cross(db, cb);
        if (dot (dir1, dir2) >= EPSF)
            return false ; 
        vec2<Precision> ca = mVertex[cIdx].Pos - mVertex[NowId].Pos;
        vec2<Precision> da = mVertex[dIdx].Pos - mVertex[NowId].Pos;
        vec2<Precision> ba = mVertex[bIdx].Pos - mVertex[NowId].Pos;
        dir1 = cross (ca, da);
        dir2 = cross(ca, ba);
        return dot(dir1, dir2) < EPSF;
    }

	bool* Tested = new bool[mVertex.size()];
    memset (Tested, 0 , mVertex.size()*sizeof (bool));
	while(!Tested[NowId])
	{
		Tested[NowId] = true;
		int Next1 = mVertex[NowId].Outer_NextId;
        int Next2 = mVertex[Next1].Outer_NextId;
		if ((Next1 < 0) ||(Next2 < 0) || (Next1 >= mVertex.size()) || (Next2 >= mVertex.size()))
        {
            delete[]  Tested;
            return false;
        }
        vec2<Precision> dir1 = mVertex[Next1].Pos - mVertex[NowId].Pos;
		vec2<Precision> dir2 = mVertex[Next2].Pos - mVertex[Next1].Pos;
		if (cross(dir1, dir2) < EPSF)
        {
			delete[] Tested;
            return true;
        }
        NowId = Next1;
	}
	for (int i = 0 ; i < mVertex.size() ; i++)
	{
		if (!Tested[i])
        {
            delete[]  Tested;
			return true;
        }
	}
   delete[]  Tested;
   return false ;
	
}



template<typename Precision> 
bool Surface<Precision>::GenerateOuterLineByTri()
{

	bool Result = false ;
	mSeg.clear();
	mOutline.clear();
	IndexPair SegIndex;
	const int Next[] = {1, 2, 0};
    std::hash_map<IndexPair, int, IndexPairCompare>  Rec;
	std::vector<Seg<Precision> > SegCandi;
	bool bFirstTime = false ;

    int ID[2] ; 
	for (int i = 0 ; i < mTriIdList.size(); i++)
	{
		const TriInfo& info =   mpOwner->TriangleInfo(mTriIdList[i]);
		for (int k = 0; k< 3; k++)
		{
            
            ID[0] = info.VertexId[k];
            ID[1] = info.VertexId[Next[k]];
            MakeIndex(ID, SegIndex);
			Rec[SegIndex]++;
		}
	}
	bFirstTime = true;
	std::hash_map<IndexPair, int, IndexPairCompare>::iterator iter = Rec.begin();
	for (; iter != Rec.end(); iter++)
	{
        
		if (iter->second == 1)
		{
            GetIDFromIndex(ID, iter->first);
			if (bFirstTime) 
			{
				bFirstTime = false;
				mOrigin = mpOwner->Vertex(ID[0]).pos;
				mN =  mpOwner->TriangleInfo(mTriIdList[0]).Normal;  
				mU = normalize( mpOwner->Vertex(ID[1]).pos - mOrigin);
                mV  = cross(mN, mU);
				mColor = mpOwner->Vertex(ID[0]).color;
			    mNormal =  mpOwner->Vertex(ID[0]).normal;
			}
			vec3<Precision> sp = mpOwner->Vertex(ID[0]).pos;
			vec3<Precision> ep = mpOwner->Vertex(ID[1]).pos;
			vec3<Precision> sp3 = PosToLocal(mU, mV, mN, mOrigin, sp);
            vec3<Precision> ep3 = PosToLocal(mU, mV, mN, mOrigin, ep);
            Seg<Precision> seg;
			
            seg.MeshID = mpOwner->GetID();
            seg.VId[0] = AddUniqueVertex(sp3.xy);
            seg.VId[1] = AddUniqueVertex(ep3.xy);
            seg.dir =  mVertex[seg.VId[1]].Pos-mVertex[seg.VId[0]].Pos;
            seg.len = length(seg.dir);
			if (IsEqual( seg.len, (Precision)0., (Precision)EPSF))
				continue;
            seg.dir*= (1/ seg.len);
			seg.AABB2D.AddPoint(mVertex[seg.VId[1]].Pos);
		    seg.AABB2D.AddPoint(mVertex[seg.VId[0]].Pos);
			SegCandi.push_back(seg);
	        //NOTE:其实这里根本没有必要修补流形
            //但是我多次求并之后流形不正确，没办法只有补了
//          MendManifold(SegCandi, mpOwner->ID());
            for (int k = 0 ; k < SegCandi.size(); k++)
				AddSeg(SegCandi[k]);
		}
	}
	 if (mSeg.size())
	 {
         mOutline.reserve(mSeg.size());
		 for (int k = 0; k < mSeg.size(); k++)
			mOutline.push_back(mSeg[k]);
		 mbConcave = DoCheckIsConcave();
        Result = true;
	 }
	 Rec.clear();
	 SegCandi.clear();
	 return Result;
}

template<typename Precision> 
void Surface<Precision>::GetLine(std::vector<vec3<Precision> >& lines)
{
    if (mOutline.size() == 0)
        return ; 
    
    int first =  mOutline[0].VId[0];
    int current = first ;
    lines.reserve(mOutline.size());
    for (;;)
    { 
        lines.push_back( PosToGlobal(mU, mV, mN, mOrigin, vec3<Precision>(mVertex[current].Pos, 0.f) ));
        current =  mVertex[current].Outer_NextId;
        if (current == -1 || current == first)
            break;
    }
}

template<typename Precision> 
void Surface<Precision>::AddConstraint(const vec3<Precision>& StartPos, const vec3<Precision>& EndPos,  int FromMeshID)
{
	vec3<Precision> startP = PosToLocal(mU, mV, mN, mOrigin, StartPos);
	vec3<Precision> endP(PosToLocal(mU, mV, mN, mOrigin, EndPos));
	return DoAddConstaint(startP.xy, endP.xy, FromMeshID);
}

template<typename Precision> 
void Surface<Precision>::Triangulate(ListOfvertices& results)
{
	if (mbHasConstraint)
	{
		std::vector<vec2<Precision> > TriArray;
		
		//if (meTriMethod == tmOld)
		//{
		//	std::vector<int> SegIdArray(mSeg.size()*2) ;
		//	for(int i = 0; i<mSeg.size(); i++ )
		//	{
		//		SegIdArray.push_back(mSeg[i].VId[0]+1);
		//		SegIdArray.push_back(mSeg[i].VId[1]+1);
		//	}
		//	std::vector<vec2<Precision> > PointArray(mVertex.size());
        //    for (int i = 0 ; i< mVertex.size(); i++)
		//		PointArray.push_back(mVertex[i].Pos);
        //      //TriangulateEx(PointArray, SegIdArray, TriArray, 0);
		//	
		//}else
		{
			/*std::vector<vec2<Precision> > SegArray(mSeg.size()*2) ;
			for(int i = 0; i<mSeg.size(); i++ )
			{
				SegArray.push_back(mVertex[mSeg[i].VId[0]].Pos);
				SegArray.push_back(mVertex[mSeg[i].VId[1]].Pos);
			}*/
			//RobustTriangulateEx(SegArray, TriArray);
			ConstrainTriangulate(TriArray);
		}
		if (mbConcave) 
			CheckTriangle(TriArray);
        results.reserve(TriArray.size());

		for (int i = 0; i < TriArray.size(); i++ )
		{ 
			vec3<Precision> point(TriArray[i], 0.f);
			point = PosToGlobal(mU, mV, mN, mOrigin, point);
			VertexInfo vi(point, mNormal, mColor);
            results.push_back(vi);
		} 
	}
  else{
	  results.reserve(mTriIdList.size()*3);
	  for(int i = 0 ; i < mTriIdList.size(); i++)
	  {
		  const TriInfo& info = mpOwner->TriangleInfo(mTriIdList[i]);
		  
		  results.push_back(mpOwner->Vertex(info.VertexId[0]));
		  results.push_back(mpOwner->Vertex(info.VertexId[1]));
	      results.push_back(mpOwner->Vertex(info.VertexId[2]));
	  }
  
  }
}

template<typename Precision> 
void Surface<Precision>::AddSeg(const Seg<Precision>& seg)
{
	if(seg.VId[0] != seg.VId[1])
	{
		IndexPair indexPair; 
		MakeIndex(seg.VId, indexPair);
		if (mSegIndexHash.find(indexPair) == mSegIndexHash.end())
		{
            CheckSegCapacity();
			mSeg.push_back(seg);
			mSegIndexHash[indexPair] = true;
		}
	}
 
}



template<typename Precision> 
void Surface<Precision>::AddSeg(Segment2D<Precision> AnsSeg[],int& AnsCnt, const vec2<Precision>& NewP, const vec2<Precision> NewDir,
		          Precision st, Precision et, double DIST_EPS)
{

	if(et - st >= DIST_EPS)
	{
		AnsSeg[AnsCnt].Dir = NewDir;
		AnsSeg[AnsCnt].Len = et -st;
		AnsSeg[AnsCnt].P1 = NewP + NewDir* st;
		AnsSeg[AnsCnt].P2 = NewP + NewDir* et;
		AnsCnt++;
	}
}

 
 
template<typename Precision>
void Surface<Precision>::DeleteSeg(int i)
{
	IndexPair indexPair; 
	MakeIndex(mSeg[i].VId, indexPair);
	mSegIndexHash.erase(indexPair);
	mSeg.erase(mSeg.begin()+i);
}

 

/*
 function ToSeg(const Seg2D: TSegment2D; MeshID: integer): TSeg;
  begin
    Result := TSeg.Create;
    Result.MeshID := MeshID;
    with Result do
    begin
      VId[0] := AddUniqueVertex(Seg2D.P1);
      VId[1] := AddUniqueVertex(Seg2D.P2);
      Dir := Seg2D.Dir;
      Len := Seg2D.Len;
      with TLinearAlgebra do
      begin
        ClearAABB(AABB);
        Include(AABB, Seg2D.P1);
        Include(AABB, Seg2D.P2);
      end;
    end;
  end;
*/

template<typename Precision>
void Surface<Precision>::MendManifold(std::vector<Seg<Precision> >& SegCandi, int FromMeshID)
{

	bool BreakSeg;
    bool BreaktestSeg;

	Segment2D<Precision> AnsArray[4], testAnsArray[4];
	int i = 0; 
	int segCandiCount = SegCandi.size();
	while( i< segCandiCount)
	{
		Seg<Precision>& seg = SegCandi[i];
		BreakSeg = false ;
        int j = 0;
		int segCount = mSeg.size();
		while( j < segCount)
		{
			BreaktestSeg = false ;
			Seg<Precision>& testSeg  = mSeg[j];
			if (seg.AABB2D.Intersects(testSeg.AABB2D, EPSF))
			{
				LineSeg2D<Precision> line1(mVertex[seg.VId[0]].Pos,mVertex[seg.VId[1]].Pos,
									 seg.dir,  seg.len);
				LineSeg2D<Precision> line2(mVertex[testSeg.VId[0]].Pos, mVertex[testSeg.VId[1]].Pos,
					             testSeg.dir,testSeg.len);
				int AnsCnt;
				int testAnsCnt ;
				SegBreak(line1,line2,EPSF, EPSF,
                         AnsArray, testAnsArray,AnsCnt, testAnsCnt);
                         
				if(testAnsCnt != 0 )
				{
					BreaktestSeg = true;
					DeleteSeg(j);
					segCount--;
					for( int k = 0; k <  testAnsCnt ; k++ )
					{
						if (testAnsArray[k].Len > EPSF) 
							AddSeg(ToSeg(testAnsArray[k], testSeg.MeshID));
					}
           
				}
			    if (AnsCnt != 0)
				{
					BreaktestSeg = true;
					BreakSeg = true;
					segCandiCount--;
					SegCandi.erase(SegCandi.begin()+i);
					for (int k = 0 ; k <AnsCnt ; k++)
					{
						if (AnsArray[k].Len > EPSF) 
							SegCandi.push_back(ToSeg(AnsArray[k], FromMeshID));
					}
					break;
				}
		    }
			if (!BreaktestSeg)
				j++;
		}
		 if (!BreakSeg)
			 i++;
	}
}

template<typename Precision> 
void Surface<Precision>::DoAddConstaint(const vec2<Precision>& StartP, const vec2<Precision>& EndP, int FromMeshID )
{

	IndexPair indexPair;
    int ID[2] ; 
     ID[0] = AddUniqueVertex(StartP);
	 ID[1] = AddUniqueVertex(EndP);
     MakeIndex(ID, indexPair);
	if (mSegIndexHash.find(indexPair) != mSegIndexHash.end())
		return ;
	Precision SegLen= length(EndP-StartP);
	if (IsEqual(SegLen, (Precision)0., (Precision)EPSF))
		return ;
	mbHasConstraint = true;
	std::vector<Seg<Precision> > SegCandi;
	Seg<Precision> seg;
	seg.Owner = this;
    seg.MeshID = FromMeshID;
    GetIDFromIndex(seg.VId, indexPair);
    seg.dir = mVertex[seg.VId[1]].Pos - mVertex[seg.VId[0]].Pos;
	seg.dir*= 1/SegLen;
    seg.len = SegLen;
	seg.AABB2D.AddPoint(mVertex[seg.VId[0]].Pos);
    seg.AABB2D.AddPoint(mVertex[seg.VId[1]].Pos);
	SegCandi.push_back(seg);
	MendManifold(SegCandi, FromMeshID);
	for(int i = 0; i< SegCandi.size(); i++)
		AddSeg(SegCandi[i]);
	SegCandi.clear();
	mbTriangulated = false ;
}


template<typename Precision>
void Surface<Precision>::SegBreak(const LineSeg2D<Precision>& line1, const LineSeg2D<Precision>& line2, 
								  double DIR_EPS,double DIST_EPS, Segment2D<Precision> AnsSeg1[], 
								  Segment2D<Precision> AnsSeg2[], int& AnsSeg1Cnt, int& AnsSeg2Cnt)
{

	  AnsSeg1Cnt = 0;
      AnsSeg2Cnt = 0;
	  Precision s, t;
	  LineRelation LineRelation = line1.IntersectTest(line2, s,t,DIR_EPS,DIST_EPS);
	  switch (LineRelation)
	  {
		case Colinear:
				DoOverlapBreak(0, line1.Length(), s, t, line1.Dir(), line1.Start(), DIST_EPS, AnsSeg1, AnsSeg2,
						        AnsSeg1Cnt, AnsSeg2Cnt);
		  return ;
		case InteralIntersect:
		{
			if ((DIST_EPS < s) && (s < line1.Length() - DIST_EPS)) 
			{
				AddSeg(AnsSeg1, AnsSeg1Cnt, line1.Start(), line1.Dir(), 0, s, DIST_EPS);
				AddSeg(AnsSeg1, AnsSeg1Cnt, line1.Start(), line1.Dir(), s, line1.Length(), DIST_EPS);
		    }
		   if ((DIST_EPS < t) && (t < line2.Length() - DIST_EPS)) 
		   {
				AddSeg(AnsSeg2, AnsSeg2Cnt, line2.Start(), line2.Dir(), 0, t,DIST_EPS );
				AddSeg(AnsSeg2, AnsSeg2Cnt, line2.Start(), line2.Dir(), t , line2.Length(),DIST_EPS);
		   }
		}
		default : 
			break;
	  }			  			  
}

template<typename Precision>
void  Surface<Precision>::DoOverlapBreak(Precision t1, Precision t2, Precision q1, Precision q2, const vec2<Precision>& SegDir,
		                 const vec2<Precision>& SegP, Precision DIST_EPS,Segment2D<Precision> AnsSeg1[], 
								  Segment2D<Precision> AnsSeg2[], int& AnsSeg1Cnt, int& AnsSeg2Cnt)
{
	if (IsEqual(q1, t1, DIST_EPS ))
	{
	   //以下示意图t下q上
       //重叠，删除Seg1
       //___
       //___
		if (IsEqual(q2, t2, DIST_EPS)) 
			AnsSeg1Cnt = -1;
	  //____
      //______
	   else if ((t1 + DIST_EPS < q2) && (q2 < t2 - DIST_EPS)) 
        AddSeg(AnsSeg1, AnsSeg1Cnt, SegP, SegDir, q2, t2,DIST_EPS);
	  //_______
      //____
      else if (q2 > t2 + DIST_EPS)
	  {
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, q1, t2, DIST_EPS);
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, t2, q2, DIST_EPS);
        AnsSeg1Cnt = -1; //代表Seg1需要被删除
	   }

	}else if ((t1 + DIST_EPS < q1) && (q1 < t2 - DIST_EPS) ){
	  //   ___
      //______
      if (IsEqual(q2, t2, DIST_EPS)) 
        AddSeg(AnsSeg1, AnsSeg1Cnt, SegP, SegDir, t1, q1,DIST_EPS);
      //    ___
      //___________
      else if ((t1 + DIST_EPS < q2) && (q2 < t2 - DIST_EPS)) 
	  {
        AddSeg(AnsSeg1, AnsSeg1Cnt, SegP, SegDir, t1, q1,DIST_EPS);
        AddSeg(AnsSeg1, AnsSeg1Cnt, SegP, SegDir, q2, t2,DIST_EPS);
	  }
      //  ____
      //____
      else if (q2 > t2 + DIST_EPS) 
	  {
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, q1, t2,DIST_EPS);
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, t2, q2,DIST_EPS);
        AddSeg(AnsSeg1, AnsSeg1Cnt, SegP, SegDir, t1, q1,DIST_EPS);
	  }
	
	}else if(q1 < t1 - DIST_EPS){
	  //________
      //     ___ 
      if (IsEqual(q2, t2, DIST_EPS)) 
	  {
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, q1, t1,DIST_EPS);
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, t1, q2,DIST_EPS);
        AnsSeg1Cnt = -1;
	  }
      //______
      //   ______
      else if ((t1 + DIST_EPS < q2) && (q2 < t2 - DIST_EPS))
	  {
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, q1, t1,DIST_EPS);
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, t1, q2,DIST_EPS);
        AddSeg(AnsSeg1, AnsSeg1Cnt, SegP, SegDir, q2, t2,DIST_EPS);
	  }
      //_________
      //   ___
      else if (q2 > t2 + DIST_EPS) 
	  {
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, q1, t1,DIST_EPS);
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, t1, t2,DIST_EPS);
        AddSeg(AnsSeg2, AnsSeg2Cnt, SegP, SegDir, t2, q2,DIST_EPS);
        AnsSeg1Cnt = -1;
	  }

	}
}

//template<typename Precision>
//void  Surface<Precision>::DoInterBreak(Segment2D<Precision> AnsSeg[], int& AnsCnt, 
//									  Precision t, Precision et, const vec2<Precision>& SegP, const vec2<Precision>& SegDir)
//{
//	  if ((DIST_EPS < t) && (t < et - DIST_EPS))
//	  {
//        //ep := Add(SegP, Mul(SegDir, et));
//        AddSeg(AnsSeg, AnsCnt, SegP, SegDir, 0, t);
//        AddSeg(AnsSeg, AnsCnt, SegP, SegDir, t, et);
//	  }
//}

//template<typename Precision>
//bool  Surface<Precision>::SameLineTest(const vec2<Precision>& Seg1P1, const vec2<Precision> Seg1P2,
//									   const vec2<Precision>& Seg2P1, const vec2<Precision> Seg2P2,
//									   const vec2<Precision>& Seg1Dir, const vec2<Precision> Seg2Dir)
//{
//	bool result = false ;
//	if(IsEqual(cross(Seg1Dir,Seg2Dir), 0, DIR_EPS))
//	{
//		result = true;
//		vec2<Precision> n1(-Seg1Dir.y, Seg1Dir.x);
//		
//        ProjLen = abs(dot( Seg2P1 - Seg1P1, n1));
//        ProjLen = abs(dot(Seg2P2-Seg1P1, n1))- ProjLen;
//        if ((-DIST_EPS < ProjLen) && (ProjLen < DIST_EPS))  //直线共线
//		{
//			 Precision t1 = dot(Seg2P1-Seg1P1, Seg1Dir);
//             Precision t2 = dot( Seg2P2- Seg1P1, Seg1Dir);
//            if (t1 > t2)
//			 std::swap(t1, t2);
//            DoOverlapBreak(0, Seg1Len, t1, t2, Seg1Dir, Seg1P1);
//		}
//	}
//}


//template<typename Precision>
//void  Surface<Precision>::InterLineTest(const vec2<Precision>& Seg1P1, const vec2<Precision> Seg1P2,
//									   const vec2<Precision>& Seg2P1, const vec2<Precision> Seg2P2,
//									   const vec2<Precision>& Seg1Dir, const vec2<Precision> Seg2Dir,
//									   Precision Seg1Len, Precision Seg2Len)
//{
//
//	//Ends Intersect
//	if(IsEqual(Seg1P1, Seg2P1, DIST_EPS) ||  IsEqual(Seg1P1, Seg2P2, DIST_EPS)
//       || IsEqual(Seg1P2, Seg2P1, DIST_EPS) || IsEqual(Seg1P2, Seg2P2, DIST_EPS))
//	   return ;
//	vec2<Precision> n1(-Seg1Dir.y, Seg1Dir.x);
//	vec2<Precision> n2(-Seg2Dir.y, Seg2Dir.x);
//	Precision t1 = (dot(Seg2P1, n2) - dot(Seg1P1, n2)) / dot(Seg1Dir, n2);
//   if ((-DIST_EPS < t1) && (t1 < Seg1Len + DIST_EPS))
//   {
//		Precision t2 = (dot(Seg1p1, n1) - dot(Seg2p1, n1)) / dot(Seg2Dir, n1);
//        if ((-DIST_EPS < t2) && (t2 < Seg2Len + DIST_EPS)) 
//		{
//          if ((DIST_EPS < t1) && (t1 < Seg1Len - DIST_EPS)) 
//		  {
//            AddSeg(AnsSeg1, AnsSeg1Cnt, Seg1P1, Seg1Dir, 0, t1);
//            AddSeg(AnsSeg1, AnsSeg1Cnt, Seg1P1, Seg1Dir, t1, Seg1Len);
//		  }
//          if ((DIST_EPS < t2) && (t2 < Seg2Len - DIST_EPS)) 
//		  {
//            AddSeg(AnsSeg2, AnsSeg2Cnt, Seg2P1, Seg2Dir, 0, t2);
//            AddSeg(AnsSeg2, AnsSeg2Cnt, Seg2P1, Seg2Dir, t2, Seg2Len);
//		  }
//		}
//   }  
//
// 
//}

template<typename Precision>
bool Surface<Precision>::IsInOuterLine(const vec2<Precision>& point)
{
	if (!mAABB2D.IsInBox(point,EPSF ))
		return false ;
	double Angle =0 ; 
	for (int i = 0; i < mOutline.size(); i++)
	{
		double Seg_Angle; 
		vec2<Precision> n(mOutline[i].dir.y, -mOutline[i].dir.x);
        vec2<Precision> v1 =  mVertex[mOutline[i].VId[0]].Pos - point;
        Precision  PointToLineDist = dot(n, v1);
		if (IsEqual(PointToLineDist, (Precision)0. , (Precision)EPSF))
			Seg_Angle = 0;
		else {

			  vec2<Precision> v2 = mVertex[mOutline[i].VId[1]].Pos - point;
              Seg_Angle  = std::acos(dot(normalize(v1), normalize(v2)));
              if (cross(v1, v2) < 0 )
				Seg_Angle = -Seg_Angle;
   
		}
		Angle += Seg_Angle;

	}
	return fabs(Angle)> M_PI;
	
}

template<typename Precision>
void  Surface<Precision>::CheckTriangle(std::vector<vec2<Precision> > & TriArray)
{
	int TriCnt = TriArray.size();
	while(TriCnt >=3 )
	{

		vec2<Precision> cp = CenterOfGravity(TriArray[TriCnt-3], TriArray[TriCnt-2], TriArray[TriCnt-1]);
		if (!IsInOuterLine(cp))
		{   
			// delete the triangle 
			TriArray.erase(TriArray.begin()+TriCnt-3, TriArray.begin()+TriCnt);
		}

		TriCnt-=3;
	}

}


template <typename Precision>
void Surface<Precision>::ConstrainTriangulate(std::vector<vec2<Precision> >& TriArray)
{
	GEOM_FADE2D::Fade_2D dt;
    std::vector<GEOM_FADE2D::Segment2 > vSegments;
	for(int i = 0; i<mSeg.size(); i++ )
	{
		vec2<Precision> v0 = mVertex[mSeg[i].VId[0]].Pos;
		vec2<Precision> v1 = mVertex[mSeg[i].VId[1]].Pos;
      
		GEOM_FADE2D::Point2 p0(v0.x , v0.y);
		GEOM_FADE2D::Point2 p1(v1.x, v1.y);
        GEOM_FADE2D::Segment2 seg(p0,p1);
		vSegments.push_back(seg);
    }
    // Use the segments to create a constraint object. The insertion strategy
	// can be CIS_KEEP_DELAUNAY or CIS_IGNORE_DELAUNAY. The difference is that
	// in the former case the Delaunay triangulation will be kept (by inserting
	// additional points if necessary). Use one of the following two lines. 	
	GEOM_FADE2D::ConstraintGraph2* pConstraint=dt.createConstraint(vSegments, GEOM_FADE2D::CIS_IGNORE_DELAUNAY);
	//ConstraintGraph2* pConstraint=dt.createConstraint(vSegments,CIS_IGNORE_DELAUNAY);
    dt.applyConstraintsAndZones();
    // Retrieve the triangles of the triangulation. Draw them in black
	std::vector<GEOM_FADE2D::Triangle2*> vAllDelaunayTriangles;
	dt.getTrianglePointers(vAllDelaunayTriangles);
	for(std::vector<GEOM_FADE2D::Triangle2*>::iterator it=vAllDelaunayTriangles.begin();it!=vAllDelaunayTriangles.end();++it)
	{
		GEOM_FADE2D::Triangle2* t(*it);
		GEOM_FADE2D::Point2* p0=t->getCorner(0);
		GEOM_FADE2D::Point2* p1=t->getCorner(1);
		GEOM_FADE2D::Point2* p2=t->getCorner(2);
       vec2<Precision> v2(p0->x(), p0->y());
	   vec2<Precision> v1(p1->x(), p1->y());
	   vec2<Precision> v0(p2->x(), p2->y());   
	    if (IsEqual(cross(v0- v2, v1-v2), Precision(0.), (Precision)EPSF ))
		   continue; //
	   TriArray.push_back(v0);
	   TriArray.push_back(v1);
	   TriArray.push_back(v2);
	}
		 
}

//template <typename Precision>
//void Surface<Precision>::ConstrainTriangulate(std::vector<vec2<Precision> >& TriArray)
//{
//	typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
//
//	typedef CGAL::Triangulation_vertex_base_2<K>                     Vb;
//	typedef CGAL::Constrained_triangulation_face_base_2<K>           Fb;
//	typedef CGAL::Triangulation_data_structure_2<Vb,Fb>              TDS;
//	//typedef CGAL::Exact_predicates_tag                               Itag;
//	typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, CGAL::No_intersection_tag> CDT;
// 
//	std::map<vec2<Precision>, CDT::Vertex_handle, Vec2LessThan<Precision>> vertexMap;
//	CDT cdt;
//	CDT::Vertex_handle vh1, vh2; 
//	for(int i = 0; i<mSeg.size(); i++ )
//	{
//		vec2<Precision> v0 = mVertex[mSeg[i].VId[0]].Pos;
//		vec2<Precision> v1 = mVertex[mSeg[i].VId[1]].Pos;
//		CDT::Point p0(v0.x , v0.y);
//		CDT::Point p1(v1.x, v1.y);
//        /*if (vertexMap.find(v0) != vertexMap.end())
//			    vh1 = vertexMap[v0];
//		   else */
//		   {
//			   vh1 = cdt.insert(p0);
//			   //vertexMap[v0] = vh1;
//		   }
//		  /* if (vertexMap.find(v1) != vertexMap.end())
//			    vh2 = vertexMap[v1];
//		   else */
//		   {
//			    vh2 = cdt.insert(p1);
//				 //vertexMap[v1] = vh2;
//		   }
//		   cdt.insert_constraint(vh1, vh2);
//	}
//	int count = cdt.number_of_faces() ; 
//	TriArray.reserve(count*3);
//    for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin();
//       fit != cdt.finite_faces_end(); ++fit)
//   {
//	   vec2<Precision> v0(fit->vertex(2)->point().x(),fit->vertex(2)->point().y() );
//	   vec2<Precision> v1(fit->vertex(1)->point().x(),fit->vertex(1)->point().y() );
//	   vec2<Precision> v2(fit->vertex(0)->point().x(),fit->vertex(0)->point().y() );   
//	   if (IsEqual(cross(v0- v2, v1-v2), Precision(0.), (Precision)EPSF ))
//		   continue; //
//	   TriArray.push_back(v0);
//	   TriArray.push_back(v1);
//	   TriArray.push_back(v2);
//
//	   /*TriArray.push_back(vec2<Precision> (fit->vertex(2)->point().x(),fit->vertex(2)->point().y() ));
//	   TriArray.push_back(vec2<Precision> (fit->vertex(1)->point().x(),fit->vertex(1)->point().y() ));
//	   TriArray.push_back(vec2<Precision> (fit->vertex(0)->point().x(),fit->vertex(0)->point().y() ));*/
//		 
//   }



//}

template class Surface<float>;
template class Surface<double>;

}