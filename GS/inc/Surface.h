#ifndef _SURFACE_H_
#define _SURFACE_H_
#include <vector>
#include <map>
#include <hash_map>
#include "configure.h"
#include "arithmetic.h"
#include "Box2.h"
#include "Box3.h"
#include "hashlookup.h"
#include "LineSegment.h"
namespace GS{


enum  TriMethod{
	tmNew, 
	tmOld
};

template<typename Precision> class Surface ;

//struct IndexPair { 
//	int ID[2];
//};


template <typename P = float> struct Segment2D{
	vec2<P> P1;
	vec2<P> P2;
	vec2<P> Dir;
	P       Len;
};


template <typename P = float > struct Seg{
	 Surface<P>*  Owner;
	 int      VId[2];
	 int      MeshID;
	 vec2<P>  dir;
	 P        len;
     Box2     AABB2D;


 };

class BaseMesh;
template <typename Precision = float> class Surface {
//public:
// struct Seg{
//	 int              VId[2];
//	 int              MeshID;
//	 vec2<Precision>  dir;
//	 Precision        len;
//     Box2     AABB2D;
// };

 struct SegVertex {
    vec2<Precision> Pos;
    int             Outer_NextId;//该字段仅仅对OuterLine有用
	SegVertex() {Outer_NextId = -1;}
 };
public:
	Surface(BaseMesh* pOwner, std::vector<int>& TriIdList);
	Surface(BaseMesh* pOwner, const Surface<Precision>&);
	~Surface();
	void GenerateAABB();
    BaseMesh* GetParent() const {return mpOwner;}
    bool InterCarve(Surface<Precision>* );
	void Clear();
    void ClearTriangleIdList() {return mTriIdList.clear();}
    void AddTriangleId(int triId) { mTriIdList.push_back(triId);} 

	bool GenerateOuterLineByTri();
	  //取出轮廓边(不包括约束)，返回形如(p1, p2)，坐标点定义在全局坐标系下
    void GetLine(std::vector<vec3<Precision> >& lines);
	    //添加约束边
    void AddConstraint(const vec3<Precision>& StartPos, const vec3<Precision>& EndPos,  int FromMeshID);
	void Triangulate(ListOfvertices& results);
	bool IsConcave() const { return mbConcave;}
	bool HasConstraint() const {return mbHasConstraint;}
	void SetConstraint(bool bConstraint)   {  mbHasConstraint = bConstraint;}
	void SetTriangulateMethod(TriMethod method) {meTriMethod = method;}
	TriMethod GetTriangulateMethod() const {return meTriMethod;}
	const vec3<Precision>& U() const {return mU;}
	const vec3<Precision>& V() const {return mV;}
	const vec3<Precision>& N() const {return mN;}
	const vec3<Precision>& Origin() const {return mOrigin;}
	const Box3& AABB() const {return mAABB3D;}
	int GetTriangleId(int idx) const { assert(idx < mTriIdList.size()); return mTriIdList[idx];}
	int  TriangleCount() const {return mTriIdList.size();}
	bool Triangulated() const { return mbTriangulated;}
    void Invalidate()  { mbTriangulated = false ;}
    const float4& SurfaceColor() const {return mColor;}
    bool  IsCCW() const; 
private :
    void CheckVertexCapacity();
    void CheckSegCapacity();
	void CheckTriangle(std::vector<vec2<Precision> > & TriArray);
	bool IsInOuterLine(const vec2<Precision>& point) ;
	int  AddUniqueVertex(const vec2<Precision>& point2D);
	bool DoCheckIsConcave();
	void AddSeg(const Seg<Precision>& seg);
    void DeleteSeg(int i);
	void MendManifold(std::vector<Seg<Precision> > & SegCandi, int FromMeshID);
    void DoAddConstaint(const vec2<Precision>& StartP, const vec2<Precision>& EndP, int FromMeshID );
	void SegBreak(const LineSeg2D<Precision>& line1, const LineSeg2D<Precision>& line2, double DIR_EPS,double DIST_EPS,
				  Segment2D<Precision> AnsSeg1[], Segment2D<Precision> AnsSeg2[], int& AnsSeg1Cnt, int& AnsSeg2Cnt);			  
	void  DoOverlapBreak(Precision t1, Precision t2, Precision q1, Precision q2, const vec2<Precision>& SegDir,
		                 const vec2<Precision>& SegP, Precision DIST_EPS,Segment2D<Precision> AnsSeg1[], 
								  Segment2D<Precision> AnsSeg2[], int& AnsSeg1Cnt, int& AnsSeg2Cnt);
	/*void  DoInterBreak(Segment2D<Precision> AnsSeg[], int& AnsCnt, Precision t, Precision et, 
		               const vec2<Precision>& SegP, const vec2<Precision>& SegDir);*/
	/*bool  SameLineTest(const vec2<Precision>& Seg1P1, const vec2<Precision> Seg1P2,
					   const vec2<Precision>& Seg2P1, const vec2<Precision> Seg2P2,
					   const vec2<Precision>& Seg1Dir, const vec2<Precision> Seg2Dir);*/
	/*void InterLineTest(const vec2<Precision>& Seg1P1, const vec2<Precision> Seg1P2,
				  const vec2<Precision>& Seg2P1, const vec2<Precision> Seg2P2,
				  const vec2<Precision>& Seg1Dir, const vec2<Precision> Seg2Dir,
				  Precision Seg1Len, Precision Seg2Len);*/
	void  AddSeg( Segment2D<Precision> AnsSeg[],int& AnsCnt, const vec2<Precision>& NewP, const vec2<Precision> NewDir,
		          Precision st, Precision et,double DIST_EPS);
	Seg<Precision>  ToSeg(const Segment2D<Precision>& Seg2D, int MeshID)
	{
		Seg<Precision>  seg;
		seg.Owner = this ;
		seg.MeshID = MeshID;
		seg.VId[0] = AddUniqueVertex(Seg2D.P1);
		seg.VId[1] = AddUniqueVertex(Seg2D.P2);
        seg.dir = Seg2D.Dir;
        seg.len = Seg2D.Len;
	    seg.AABB2D.Clear();
	    seg.AABB2D.AddPoint(Seg2D.P1.x, Seg2D.P1.y);
		seg.AABB2D.AddPoint(Seg2D.P2.x, Seg2D.P2.y);
		return seg;
	}
	void ConstrainTriangulate(std::vector<vec2<Precision> >& TriArray);
   
private:
	BaseMesh*  mpOwner;
	std::vector<Seg<Precision> > mSeg;
	std::vector<Seg<Precision> > mOutline;
	std::vector<SegVertex> mVertex;
	std::vector<int> mTriIdList;
	std::map<vec2<Precision>, int, Vec2LessThan<Precision>> mVertexDict; 
	std::hash_map<IndexPair, bool>   mSegIndexHash;
	Box3              mAABB3D;
	Box2              mAABB2D;
	bool              mbHasConstraint;
	bool              mbConcave;
	vec3<Precision>   mU;
	vec3<Precision>   mV;
	vec3<Precision>   mN;
    vec3<Precision>   mOrigin;
	vec3<Precision>   mNormal;
	float4            mColor;
	int               mnTag;
	TriMethod         meTriMethod;
	bool              mbTriangulated;
    int               mSegCapacity;
    int               mVertexCapacity;
};

}

#endif