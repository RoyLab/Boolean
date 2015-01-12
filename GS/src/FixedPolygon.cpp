#include "FixedPolygon.h"
#include "topology.h"

namespace GS{

/////////////////////////////////////////////////////////////////////

#define X0 0
#define X1 1

#define Y0 2
#define Y1 3

#define Z0 4
#define Z1 5
//bbox: x+ x- y+ y- z+ z-

void FixedPlanePolygon::CreatePolygon(const FixedPlane& splane,  const FixedPlane* bbox, FixedPlanePolygon& output)
{
	auto &normal = splane.Normal();
	unsigned maxIndex;
	auto a = fabs(normal.x),
		b = fabs(normal.y),
		c = fabs(normal.z);
	if (a > b)
	{
		if (a > c) maxIndex = 0;
		else maxIndex = 2;
	}
	else 
	{
		if (b > c) maxIndex = 1;
		else maxIndex = 2;
	}

    output.splane = splane;
	FixedPlanePolygon tmp;
	switch (maxIndex)
	{
	case 0:
		if (normal.x > 0)
		{
			output.bplanes.push_back(bbox[Y0]);
			output.bplanes.push_back(bbox[Z1]);
			output.bplanes.push_back(bbox[Y1]);
			output.bplanes.push_back(bbox[Z0]);

			if (On == output.ClipByPlane(bbox[X0],	tmp)) break;
			output.Clear();
			tmp.ClipByPlane(bbox[X1], output);
		}
		else
		{
			output.bplanes.push_back(bbox[Z0]);
			output.bplanes.push_back(bbox[Y1]);
			output.bplanes.push_back(bbox[Z1]);
			output.bplanes.push_back(bbox[Y0]);

			if (On == output.ClipByPlane(bbox[X0],	tmp)) break;
			output.Clear();
			tmp.ClipByPlane(bbox[X1], output);
		}
		break;
	case 1:
		if (normal.y > 0)
		{
			output.bplanes.push_back(bbox[X0]);
			output.bplanes.push_back(bbox[Z0]);
			output.bplanes.push_back(bbox[X1]);
			output.bplanes.push_back(bbox[Z1]);
			
 			if (On == output.ClipByPlane(bbox[Y0],	tmp)) break;
			output.Clear();
			tmp.ClipByPlane(bbox[Y1], output);
		}
		else
		{
			output.bplanes.push_back(bbox[Z1]);
			output.bplanes.push_back(bbox[X1]);
			output.bplanes.push_back(bbox[Z0]);
			output.bplanes.push_back(bbox[X0]);
			
			if (On == output.ClipByPlane(bbox[Y0],	tmp)) break;
			output.Clear();
			tmp.ClipByPlane(bbox[Y1], output);
		}
		break;
	case 2:
		if (normal.z > 0)
		{
			output.bplanes.push_back(bbox[Y0]);
			output.bplanes.push_back(bbox[X0]);
			output.bplanes.push_back(bbox[Y1]);
			output.bplanes.push_back(bbox[X1]);
			
			if (On == output.ClipByPlane(bbox[Z0],	tmp)) break;
			output.Clear();
			tmp.ClipByPlane(bbox[Z1], output);
		}
		else
		{
			output.bplanes.push_back(bbox[X1]);
			output.bplanes.push_back(bbox[Y1]);
			output.bplanes.push_back(bbox[X0]);
			output.bplanes.push_back(bbox[Y0]);
			
			if (On == output.ClipByPlane(bbox[Z0],	tmp)) break;
			output.Clear();
			tmp.ClipByPlane(bbox[Z1], output);
		}
		break;
	}
}


std::ostream& operator<<(std::ostream& out, const FixedPlanePolygon& poly)
{
	out << "splane: \n";
	out << poly.splane;
	out << "bplane: \n";
	for (int i = 0; i < poly.bplanes.size(); i++)
	{
		out << poly.bplanes[i];
	}
	return out;
}

 FixedPlanePolygon::FixedPlanePolygon(const vec3<double>& v1, const vec3<double>& v2, const vec3<double>& v3)
 {


	auto edge1 = v3 - v2;
	auto edge2 = v2 - v1;
	auto edge3 = v1 - v3;
	double3 norm = cross(edge1, edge2);
    splane = FixedPlane(norm, v1);

	double3 bnormal;
	bnormal = cross(edge3, norm);
	bplanes.push_back(FixedPlane(bnormal, v1));
	bnormal = cross(edge2, norm);
	bplanes.push_back(FixedPlane(bnormal, v2));
	bnormal = cross(edge1, norm);
	bplanes.push_back(FixedPlane(bnormal, v3));

 }

FixedPlanePolygon::FixedPlanePolygon(const Polygon& poly)
	:splane(poly.normal, poly.points[0])
{
    int nPoints = poly.points.size();

 //   for (int j = 0; j< nPoints; j++)
 //   {
 //       vec3<double> edge = poly.points[(j+1)%nPoints] - poly.points[j];
 //       vec3<double> normal =  cross(poly.normal, edge);//normalize( cross(poly.normal, edge));
 //       if (!bCCW)
 //       {
 //           normal.x = - normal.x;
 //           normal.y = - normal.y;
 //           normal.z = - normal.z;
 //       }
 //       Plane<double> bplane( normal, poly.points[j]);
 //       bplanes.push_back(bplane);
 //   }
	//int n = PrimitiveCount();
	//for (int i = 0; i < n; i++)
	//{
	//	poly.bplanes.clear();
	//	auto &v1 = Vertex(tri[i].VertexId[0]).pos;
	//	auto &v2 = Vertex(tri[i].VertexId[1]).pos;
	//	auto &v3 = Vertex(tri[i].VertexId[2]).pos;

	//	auto edge1 = v3 - v2;
	//	auto edge2 = v2 - v1;
	//	auto edge3 = v1 - v3;
	//	double3 norm = normalize(cross(edge1, edge2));
	//	Plane<double> splane(norm, v1);
	//	poly.splane = splane;

	//	double3 bnormal;
	//	bnormal = normalize(cross(edge3, norm));
	//	poly.bplanes.push_back(Plane<double>(bnormal, v1));
	//	bnormal = normalize(cross(edge2, norm));
	//	poly.bplanes.push_back(Plane<double>(bnormal, v2));
	//	bnormal = normalize(cross(edge1, norm));
	//	poly.bplanes.push_back(Plane<double>(bnormal, v3));

	//	polygons.push_back(poly);
	//}
}


RelationToPlane FixedPlanePolygon::ClassifyPloygonToPlane(const FixedPlane& plane) const
{
    int numInFront = 0 ; 
    int numInBack = 0; 
    
    for (int i = 0; i < bplanes.size(); i++)
    {
         int pBpId = i > 0? i -1: bplanes.size() -1;
         RelationToPlane PtToPlane= plane.ClassifyPointToPlane(splane, bplanes[pBpId], bplanes[i]);
         if (PtToPlane == Front)
             numInFront++;
         else if (PtToPlane == Behind)
             numInBack ++;
    }
    if (numInFront != 0 && numInBack != 0)
        return Straddling;
    if (numInFront != 0)
        return Front;
    if (numInBack != 0)
        return Behind;
    return On;

}


RelationToPlane FixedPlanePolygon::ClassifyEdgeToPlane(const FixedPlane& plane, int idx) const
{
     int nSize = bplanes.size();
     int pBpId = idx > 0? idx -1: nSize -1;
     int aBpId = idx +1 >= nSize? 0: idx +1; 
     // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
     RelationToPlane currentPtPos= plane.ClassifyPointToPlane(splane, bplanes[pBpId], bplanes[idx]);
     RelationToPlane nextPtPos = plane.ClassifyPointToPlane(splane,   bplanes[idx], bplanes[aBpId]);
     if (nextPtPos == On)
         return currentPtPos;
     if (currentPtPos == On)
         return nextPtPos;
     if (currentPtPos != nextPtPos)
         return Straddling;
     return currentPtPos;    
}


// Clip polygon and only output the polygon in the front of plane 
RelationToPlane FixedPlanePolygon::ClipByPlane(const FixedPlane& bp, FixedPlanePolygon& front) const
{
     if (splane.IsCoincidence(bp))
        return On;

	auto n = bplanes.size();
    bool bBack = false;

	RelationToPlane *relation = new RelationToPlane[n];
	for (int i = 0; i < n; i++)
    {
		relation[i] = bp.ClassifyPointToPlane(splane,
            bplanes[i], bplanes[(i+1)%n]);
        if (relation[i] == Behind) bBack = true;
    }

    bool bPlaneInserted = false ; 

    for (int i = 0 ; i < n; i++)
    {
        // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
        RelationToPlane  prevPtPos = relation[(i+n-2)%n];
        RelationToPlane currentPtPos= relation[(i+n-1)%n];
        RelationToPlane nextPtPos = relation[i];

        OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
        switch (signal)
        {
            case B:
                front.bplanes.push_back(bplanes[i]);
                break;
            case HB:
                if (!bPlaneInserted)
                {
                    front.bplanes.push_back(bp);
                    bPlaneInserted = true;
                }
                front.bplanes.push_back(bplanes[i]);
                break;
            default:
                break;
        }
    }
	delete [] relation;
    front.splane = splane;

    if (front.Size() == 0) return Behind;
    if (bBack) return Straddling;
    return Front;
}

// Clip polygon and only output the polygon in the front of plane 
RelationToPlane FixedPlanePolygon::ClipByPlaneNoFront(FixedPlane& bp)
{
     if (splane.IsCoincidence(bp))
        return On;

	auto n = bplanes.size();
    bool bBack = false;

	RelationToPlane *relation = new RelationToPlane[n];
	for (int i = 0; i < n; i++)
    {
		relation[i] = bp.ClassifyPointToPlane(splane,
            bplanes[i], bplanes[(i+1)%n]);
        if (relation[i] == Behind) bBack = true;
    }

    bool bPlaneInserted = false ; 
    std::vector<FixedPlane> front;

    for (int i = 0 ; i < n; i++)
    {
        // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
        RelationToPlane  prevPtPos = relation[(i+n-2)%n];
        RelationToPlane currentPtPos= relation[(i+n-1)%n];
        RelationToPlane nextPtPos = relation[i];

        OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
        switch (signal)
        {
            case B:
                front.push_back(bplanes[i]);
                break;
            case HB:
                if (!bPlaneInserted)
                {
                    front.push_back(bp);
                    bPlaneInserted = true;
                }
                front.push_back(bplanes[i]);
                break;
            default:
                break;
        }
    }
	delete [] relation;
    front.swap(bplanes);

    if (Size() == 0) return Behind;
    if (bBack) return Straddling;
    return Front;
}


RelationToPlane FixedPlanePolygon::ClipByPlane(const FixedPlane& bp, FixedPlanePolygon& front, FixedPlanePolygon& back) const
{
     if (splane.IsCoincidence(bp))
        return On;

	auto n = bplanes.size();
	RelationToPlane *relation = new RelationToPlane[n];
	for (int i = 0; i < n; i++)
		relation[i] = bp.ClassifyPointToPlane(splane, bplanes[i], bplanes[(i+1)%n]);

    bool bFrontInserted = false ; 
    bool bBackInserted = false ; 

    for (int i = 0 ; i < n; i++)
    {
        // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
        RelationToPlane  prevPtPos = relation[(i+n-2)%n];
        RelationToPlane currentPtPos= relation[(i+n-1)%n];
        RelationToPlane nextPtPos = relation[i];

        OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
        switch (signal)
        {
            case B:
                front.bplanes.push_back(bplanes[i]);
                break;
            case HB:
                if (!bFrontInserted)
                {
                    front.bplanes.push_back(bp);
                    bFrontInserted = true;
                }
                front.bplanes.push_back(bplanes[i]);
                break;
            default:
                break;
        }
       
        // output back plane
        OutputSymbol backSignal =  LookupEncodingTable(ReverseRelation(prevPtPos), ReverseRelation(currentPtPos), ReverseRelation(nextPtPos));
        switch (backSignal)
        {
            case B:
                back.bplanes.push_back(bplanes[i]);
                break;
            case HB:
                if (!bBackInserted)
                {
                    FixedPlane hp (-bp.Normal(), -bp.Distance());
                    back.bplanes.push_back(hp);
                    bBackInserted = true;
                }
                back.bplanes.push_back(bplanes[i]);
                break;
            default:
                break;
        }

    }
	delete [] relation;
 	auto res = On;
    if (front.bplanes.size() )
    {
        front.splane = splane;
        res = Front;
    }
    if (back.bplanes.size())
    {
        back.splane = splane;
        res = Behind;
    }
    if (front.bplanes.size() && back.bplanes.size() )
        return Straddling;

    return res;
}

RelationToPlane FixedPlanePolygon::ClipByPlaneNoFront(FixedPlane& bp, FixedPlanePolygon& back)
{
     if (splane.IsCoincidence(bp))
        return On;

	auto n = bplanes.size();
	RelationToPlane *relation = new RelationToPlane[n];
	for (int i = 0; i < n; i++)
		relation[i] = bp.ClassifyPointToPlane(splane, bplanes[i], bplanes[(i+1)%n]);

    bool bFrontInserted = false; 
    bool bBackInserted = false;
    
    std::vector<FixedPlane> front;

    for (int i = 0 ; i < n; i++)
    {
        // classify  point Vx-1 , Vx , Vx+1 to Plane  bp
        RelationToPlane  prevPtPos = relation[(i+n-2)%n];
        RelationToPlane currentPtPos= relation[(i+n-1)%n];
        RelationToPlane nextPtPos = relation[i];

        OutputSymbol signal =  LookupEncodingTable(prevPtPos, currentPtPos, nextPtPos);
        switch (signal)
        {
            case B:
                front.push_back(bplanes[i]);
                break;
            case HB:
                if (!bFrontInserted)
                {
                    front.push_back(bp);
                    bFrontInserted = true;
                }
                front.push_back(bplanes[i]);
                break;
            default:
                break;
        }
       
        // output back plane
        OutputSymbol backSignal =  LookupEncodingTable(ReverseRelation(prevPtPos), ReverseRelation(currentPtPos), ReverseRelation(nextPtPos));
        switch (backSignal)
        {
            case B:
                back.bplanes.push_back(bplanes[i]);
                break;
            case HB:
                if (!bBackInserted)
                {
                    FixedPlane hp (-bp.Normal(), -bp.Distance());
                    back.bplanes.push_back(hp);
                    bBackInserted = true;
                }
                back.bplanes.push_back(bplanes[i]);
                break;
            default:
                break;
        }

    }
	delete [] relation;
 	auto res = On;
    bplanes.swap(front);

    if (bplanes.size() )
    {
        res = Front;
    }
    if (back.bplanes.size())
    {
        back.splane = splane;
        res = Behind;
    }
    if (bplanes.size() && back.bplanes.size() )
        return Straddling;

    return res;
}

OutputSymbol   FixedPlanePolygon::LookupEncodingTable(RelationToPlane prevPtPos, RelationToPlane currentPtPos, RelationToPlane nextPtPos) const
{
    if (currentPtPos == Front)
        return B;
    if (currentPtPos == Behind)
    {
        if (nextPtPos == Front)
            return HB;
        return Empty;
    }
    if (nextPtPos ==Front)
    {
        if (prevPtPos == Front )
            return B;
        return  HB;
    }else 
        return Empty;
}


RelationToPlane FixedPlanePolygon::ReverseRelation(RelationToPlane relation) const
{
    switch (relation)
    {
        case Front: 
             return Behind;
        case Behind:
             return Front;
        default:
            return relation;
    }

}

void FixedPlanePolygon::ToPolygon(Polygon& poly)
{
    assert(Size() >=3);
    poly.Clear();
    poly.normal = splane.Normal();
    for (int j = 0 ; j <  bplanes.size(); j++)
    {
        int prevPtIdx = j == 0? bplanes.size() -1 : j -1;
        Point3D pt = InexactComputePoint(splane, bplanes[prevPtIdx], bplanes[j] );
        bool bFound = false ;
        for (int i =0; i < poly.points.size(); i++)
        {
            if (vec3_equal(pt, poly.points[i]))
            {
                bFound = true;
                assert(0);
                break;
            }
        }
        if (!bFound)
            poly.points.push_back(pt);
    }
}

void FixedPlanePolygon::Negate()
{
	splane.Negate();
	auto copy = bplanes;
	bplanes.clear();
	int n = copy.size();
	for (int i = 0; i < n; i++)
	{
		bplanes.push_back(copy.back());
		copy.pop_back();
	}
}

}
