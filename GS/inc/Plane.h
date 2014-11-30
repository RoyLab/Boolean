#ifndef _PLANE_H_
#define _PLANE_H_
#include <cstdio>
#include <cmath>
#include <hash_map>
#include "typedefs.h"
#include "configure.h"
#include <iostream>
namespace GS{

//enum RelationToPlane{
//    Front, 
//    On,
//    Behind, 
//    Straddling
//};

 
template< typename P= float> 
class Plane {
public:
    Plane() {}
    Plane(const vec3<P>& normal, P d)
        :mNormal(normal)
        ,mD(d) {}
    Plane (const vec3<P>& normal, const vec3<P>& pos);

    void Negate() { mNormal = -mNormal; mD = -mD;}
    bool operator ==(const Plane<P>& plane);
    bool IsCoincidence(const Plane<P>& ) const;
    bool IsParallel(const Plane<P>&) const; 
    bool IsSimilarlyOrientation(const Plane<P>&) const;
 
    RelationToPlane  ClassifyPlaneToPlane(const Plane<P>& p) const;
    RelationToPlane  ClassifyPointToPlane(const vec3<P>&) const ; 
    RelationToPlane  ClassifyPointToPlane(const Plane<P> & p, const Plane<P> & q, const Plane<P> & r) const;
    const vec3<P>& Normal() const { return mNormal;}
    P    Distance() const {return mD;}
private : 
    vec3<P> mNormal;
     P      mD;
};

template <typename P = float > 
class PlaneCompare : public std::binary_function<Plane<P>, Plane<P>, bool> {
public: 
	bool operator() (const Plane<P>& p1, const Plane<P>& p2)
	{
        if (vec3_lessthan(p1.Normal(), p2.Normal()))
            return true;
        if ( vec3_equal(p1.Normal(), p2.Normal()))
        {
            return p1.Distance() < (p2.Distance() -EPSF );
        }
        return false ;
    }
};


template <typename P>
class PlaneHashCompare : public std::hash_compare<Plane<P>  >
{
public:
    size_t operator()(const Plane<P>& _Key) const 
    { 
        int value[4];
        value[0] = round(_Key.Normal().x*10000);
        value[1] = round(_Key.Normal().y*10000);
        value[2] = round(_Key.Normal().z*10000);
        value[3] = round(_Key.Distance()*10000);
  
        return hashword((const uint32_t*)value, 4, 0);
    }
            
  bool operator()(const Plane<P>& _Keyval1, const Plane<P>& _Keyval2) const 
  { 
	  if (vec3_lessthan(_Keyval1.Normal(), _Keyval2.Normal()))
		return true;
      return _Keyval1.Distance() < (_Keyval2.Distance() -EPSF); 
  } 
};

template< typename P>
vec3<P> ComputePoint(const Plane<P>& p, const Plane<P>& q, const Plane<P>& r)
{
    mat3<P> mat; 
    mat[0] = p.Normal();
    mat[1] = q.Normal();
    mat[2] = r.Normal();
    vec3<P> qr = cross(q.Normal(), r.Normal());
    vec3<P> rp = cross(r.Normal(), p.Normal());
    vec3<P> pq = cross(p.Normal(), q.Normal());
    return (-p.Distance()* qr- q.Distance()* rp -r.Distance() * pq) /determinant(mat);
}

template <typename P> 
bool IsPointVaild(const Plane<P>& p, const Plane<P>& q, const Plane<P>& r)
{
    mat3<P> mat; 
    mat[0] = p.Normal();
    mat[1] = q.Normal();
    mat[2] = r.Normal();
    return !IsEqual(determinant(mat), (P)0. , (P)EPSF); 
}


template <typename P> 
std::ostream& operator<<(std::ostream& out, const Plane<P>& p)
{
	out << p.Normal()[0] << "\t" 
		<< p.Normal()[1] << "\t"
		<< p.Normal()[2] << "\t";
	out << p.Distance();
	out << std::endl;
	return out;
}

}

#endif 