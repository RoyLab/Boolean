#pragma once
#include <vector>
#include <iostream>
#include "FixedPlane.h"
#include "Polygon.h"
#include "Box3.h"

namespace GS{

struct FixedPlanePolygon{
        FixedPlane splane;
        std::vector<FixedPlane> bplanes;
        float4 color;

		FixedPlanePolygon(){}
        FixedPlanePolygon(const Polygon&);
        FixedPlanePolygon(const vec3<double>& v1, const vec3<double>& v2, const vec3<double>& v3);
        int Size() const {return bplanes.size();}
        void Clear() { bplanes.clear();}
        void ToPolygon(Polygon&);
       RelationToPlane ClassifyPloygonToPlane(const FixedPlane& plane) const;
       RelationToPlane ClassifyEdgeToPlane(const FixedPlane& plane, int idx) const;
       RelationToPlane ClipByPlane(const FixedPlane& bp, FixedPlanePolygon& front) const; // only output the polygon in the front of  clipping plane
       RelationToPlane ClipByPlaneNoFront(FixedPlane& bp); // only output the polygon in the front of  clipping plane
       RelationToPlane ClipByPlane(const FixedPlane& bp, FixedPlanePolygon& front, FixedPlanePolygon& back) const; 
       RelationToPlane ClipByPlaneNoFront(FixedPlane& bp, FixedPlanePolygon& back); 
	   void Negate();
	 friend std::ostream& operator<<(std::ostream& out, const FixedPlanePolygon& poly);
	 static void CreatePolygon(const FixedPlane& splane,  const FixedPlane* bbox, FixedPlanePolygon& output);
private :
       OutputSymbol    LookupEncodingTable(RelationToPlane prevPtPos, RelationToPlane currentPtPos, RelationToPlane nextPtPos) const;
       RelationToPlane ReverseRelation(RelationToPlane relation) const;
};

typedef FixedPlanePolygon FFPlanePolygon;
}