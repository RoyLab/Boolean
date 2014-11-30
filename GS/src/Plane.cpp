#include "Plane.h"
#include "configure.h"
#include "arithmetic.h"
namespace GS{

template<typename P> 
Plane<P>::Plane (const vec3<P>& normal, const vec3<P>& pos)
        : mNormal(normalize(normal))
{
    mD = -dot(mNormal, pos);
}


template<typename P> 
bool Plane<P>::IsCoincidence(const Plane<P>& p ) const
{

    if (!IsEqual(mNormal.x * p.mNormal.y -mNormal.y *p.mNormal.x, (P)0., (P)EPSF))
         return false ; 
    if (!IsEqual(mNormal.x * p.mNormal.z -mNormal.z *p.mNormal.x, (P)0., (P)EPSF))
        return false ;
    if (!IsEqual(mNormal.x * p.mD -mD *p.mNormal.x, (P)0., (P)EPSF))
        return false ;
    if (!IsEqual(mNormal.y * p.mD -mD *p.mNormal.y, (P)0., (P)EPSF))
        return false ;
    if (!IsEqual(mNormal.y * p.mNormal.z -p.mNormal.y * mNormal.z, (P)0., (P)EPSF))
        return false ;
    if (!IsEqual(mNormal.z * p.mD -mD*p.mNormal.z, (P)0., (P)EPSF))
        return false ;
   return true; 
}
  
template<typename P> 
bool Plane<P>::IsParallel(const Plane<P>& p) const
 {
     return IsEqual(dot(cross(mNormal,  p.mNormal)), (P)0., (P)EPSF);
 }


template<typename P> 
bool Plane<P>::IsSimilarlyOrientation(const Plane<P>& p) const
{
    return (mNormal.x * p.mNormal.x) + (mNormal.y * p.mNormal.y) 
        + (mNormal.z* p.mNormal.z) >=0; 
}

template<typename P> 
 bool Plane<P>::operator ==(const Plane<P>& p) 
 {
     if (!vec3_equal(mNormal, p.mNormal))
         return false ; 
     return IsEqual(mD, p.mD, (P)EPSF);
 }

template<typename P> 
RelationToPlane  Plane<P>::ClassifyPointToPlane(const vec3<P>& p) const 
{
    P dist =  (dot(mNormal, p) + mD);
    if (dist < -EPSF)
        return Behind;
    else if (dist > EPSF)
        return Front;
    else 
        return On;

}

template<typename P> 
RelationToPlane  Plane<P>::ClassifyPointToPlane(const Plane<P> & p, const Plane<P> & q, const Plane<P> & r) const
{
    return ClassifyPointToPlane(ComputePoint(p, q, r));
   /* mat3<P> ptMat; 
    ptMat[0]= p.mNormal;
    ptMat[1] =q.mNormal;
    ptMat[2] = r.mNormal;
    P detPt =  determinant(ptMat);
    mat4<P> matPlane;
    matPlane[0].xyz = p.mNormal;
    matPlane[0].w = p.mD;
    matPlane[1].xyz = q.mNormal;
    matPlane[1].w = q.mD;
    matPlane[2].xyz = r.mNormal;
    matPlane[2].w = r.mD;
    matPlane[3].xyz = mNormal;
    matPlane[3].w = mD;
    P dist =  detPt* determinant(matPlane);
     if (dist < -EPSF)
        return Behind;
    else if (dist > EPSF)
        return Front;
    else 
        return On;*/
}

template <typename P> 
RelationToPlane Plane<P>::ClassifyPlaneToPlane(const Plane<P>& p) const 
{
    if (IsParallel(p) )
    {
         vec3<P> pt; 
         P x,  y, z;
         x = fabs(p.mNormal.x);
         y = fabs(p.mNormal.y);
         z = fabs(p.mNormal.z);
         if ( x > y && x > z)
         {
            pt.y = pt.z = 0;
            pt.x = -p.mD/p.mNormal.x;
         }else if (y > x && y >z){
            pt.x = pt.z = 0;
            pt.y = -p.mD / p.mNormal.y;
        }else {
            pt.x = pt.y = 0;
            pt.z = -p.mD /p.mNormal.z;
         }
         return ClassifyPointToPlane(pt);
    }
    return Straddling;

}


template class Plane<float>;
template class Plane<double>;
}
