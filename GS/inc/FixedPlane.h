#pragma once
#include <cstdio>
#include <hash_map>
#include "typedefs.h"
#include "arithmetic.h"
#include "configure.h"
#include <iostream>
#include <cassert>


namespace GS{

class FixedPlane
{
public:
	FixedPlane(void) {}
    FixedPlane(const double3& normal, double d):mNormal(normal),mD(d) {}
	FixedPlane (const double3& normal, const double3& pos): mNormal(normal), mD(-dot(normal, pos)){}
	FixedPlane(const double3& a, const double3& b, const double3& c);
	FixedPlane& operator= (const FixedPlane& other);

	~FixedPlane(void) {}
	
    void Negate() { mNormal = -mNormal; mD = -mD;}
    bool operator == (const FixedPlane& plane) const;
    bool IsCoincidence(const FixedPlane& ) const;
    bool IsParallel(const FixedPlane&) const; 
    bool IsSimilarlyOrientation(const FixedPlane&) const;
    RelationToPlane  ClassifyPointToPlane(const double3&) const ; 
    RelationToPlane  ClassifyPointToPlane(const FixedPlane & p, const FixedPlane & q, const FixedPlane & r) const;
    const double3& Normal() const { return mNormal;}
    const double&  Distance() const {return mD;}
private :

    double3 mNormal;
    double      mD;
};

class FixedPlaneCompare : public std::binary_function<FixedPlane, FixedPlane, bool> {
public: 
	bool operator() (const FixedPlane& p1, const FixedPlane& p2)
	{
		auto &n1 = p1.Normal();
		auto &n2 = p2.Normal();

		if (n1.x < n2.x) return true;
		else if (n1.x != n2.x) return false;
		else
		{
			if (n1.y < n2.y) return true;
			else if (n1.y!= n2.y) return false;
			else
			{
				if (n1.z < n2.z) return true;
				else if (n1.z!= n2.z) return false;
				else
				{
					if (p1.Distance() < p2.Distance()) return true;
					else return false;
				}
			}
		}
        return false ;
    }
};

double3 InexactComputePoint(const FixedPlane& p, const FixedPlane& q, const FixedPlane& r);

bool IsPointVaild(const FixedPlane& p, const FixedPlane& q, const FixedPlane& r);

std::ostream& operator<<(std::ostream& out, const FixedPlane& p);

}