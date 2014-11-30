#ifndef _PLOYGON_H_
#define _PLOYGON_H_
#include <vector>
#include <map>
#include <iostream>
#include "typedefs.h"
#include "Plane.h"
#include "Box3.h"

namespace GS{

enum OutputSymbol{
    Empty,
    B, 
    HB,
    PB, 
    PBB,
};

struct Polygon {
 std::vector<vec3<double> > points; 
 vec3<double> normal;
 float4       color ; 
 Box3         bbox;

  int  VertexCount() const {return points.size();}
  void Trianglate(ListOfvertices& results) const; 
  void Trianglate1(ListOfvertices& results) const; 
  bool IsCCW() const ; 
  bool IsConvex() const ; 
  bool IntersectWithBox(const Box3& bbox) const ;
  RelationToPlane ClassifyPloygonToPlane(const Plane<double>& plane) const;
  void GenAABB();
  void Clear();
//  RelationToPlane ClassifyEdgeToPlane(const Plane<double>& plane, int idx);
  RelationToPlane ClipByPlane(const Plane<double>& bp, Polygon& front);// only output the polygon in the front of  clipping plane
  RelationToPlane ClipByPlane(const Plane<double>& bp, Polygon& front, Polygon& back);
 
private: 
    void TriangulatePloygon(std::vector<int>& tri) const ;
    void TriangulateConvexPolygon(std::vector<int>& tri) const ;
    void TriangulateConcavePolygon(std::vector<int>& tri) const;
};

struct PlanePolygon{
        Plane<double> splane;
        std::vector<Plane<double> > bplanes;
        float4          color; 
        
       PlanePolygon() {};
       PlanePolygon(const Polygon& poly);
       void Clear() { bplanes.clear();}
       RelationToPlane ClassifyPloygonToPlane(const Plane<double>& plane) const;
       RelationToPlane ClassifyEdgeToPlane(const Plane<double>& plane, int idx) const;
       RelationToPlane ClipByPlane(const Plane<double>& bp, PlanePolygon& front) const; // only output the polygon in the front of  clipping plane
       RelationToPlane ClipByPlane(const Plane<double>& bp, PlanePolygon& front, PlanePolygon& back) const; 
	   void Negate();
	 friend std::ostream& operator<<(std::ostream& out, const PlanePolygon& poly);
private :
       OutputSymbol    LookupEncodingTable(RelationToPlane prevPtPos, RelationToPlane currentPtPos, RelationToPlane nextPtPos) const;
       RelationToPlane ReverseRelation(RelationToPlane relation) const;
       void    InsertBPlane(const Plane<double>& bp, std::map<Plane<double>, bool, PlaneCompare<double> >&);
};


}


#endif