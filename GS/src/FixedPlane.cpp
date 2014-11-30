#include "FixedPlane.h"
#include "configure.h"
#include "adaptive.h"

namespace GS{

FixedPlane& FixedPlane::operator= (const FixedPlane& other)
{
    mNormal = other.mNormal;
    mD  = other.mD;
    return *this;
}

bool FixedPlane::IsCoincidence(const FixedPlane& p ) const
{
	if (adaptiveDet2x2Sign(mNormal.x, mD, p.mNormal.x, p.mD) != 0) return false;
	if (mD == 0.0)
	{
		if (p.mD != 0.0) return false;
		else return IsParallel(p);
	}
	else
	{
		if (mNormal.x != 0.0) return IsParallel(p);
		if (mNormal.y != 0.0)
		{
			return (adaptiveDet2x2Sign(mNormal.y, mD, p.mNormal.y, p.mD) == 0.0
				&& IsParallel(p));
		}
		if (mNormal.z != 0.0)
		{
			return (adaptiveDet2x2Sign(mNormal.z, mD, p.mNormal.z, p.mD) == 0.0
				&& IsParallel(p));
		}
		assert(0);
		return false;
	}
}
  
bool FixedPlane::IsParallel(const FixedPlane& p) const
{
	if (adaptiveDet2x2Sign(mNormal.x, mNormal.y, p.mNormal.x, p.mNormal.y) == 0
		&& adaptiveDet2x2Sign(mNormal.y, mNormal.z, p.mNormal.y, p.mNormal.z) == 0
		&& adaptiveDet2x2Sign(mNormal.x, mNormal.z, p.mNormal.x, p.mNormal.z) == 0)
		return true;
	else return false;
}

bool FixedPlane::IsSimilarlyOrientation(const FixedPlane& p) const
{
	return adaptiveDot3Sign(mNormal, p.mNormal) >= 0;
}

bool FixedPlane::operator ==(const FixedPlane& p) const
 {
	 if (Distance() != p.Distance()) return false;
     return Normal() == p.Normal();
 }

RelationToPlane FixedPlane::ClassifyPointToPlane(const double3& p) const 
{
    auto sign = adaptivePointClassify(mNormal, mD, p);

    if (sign > 0.0) return Front;
	if (sign < 0.0) return Behind;
    return On;
}

RelationToPlane FixedPlane::ClassifyPointToPlane(const FixedPlane & p, const FixedPlane & q, const FixedPlane & r) const
{
	double3x3 ptMat;
    ptMat[0]= p.mNormal;
    ptMat[1] =q.mNormal;
    ptMat[2] = r.mNormal;

    double4x4 matPlane;
    matPlane[0].xyz = p.mNormal;
    matPlane[0].w = p.mD;
    matPlane[1].xyz = q.mNormal;
    matPlane[1].w = q.mD;
    matPlane[2].xyz = r.mNormal;
    matPlane[2].w = r.mD;
    matPlane[3].xyz = mNormal;
    matPlane[3].w = mD;

	//auto a = determinant(ptMat);
	//auto b = determinant(matPlane);
	auto dist =  adaptiveDet3x3Sign(ptMat)*adaptiveDet4x4Sign(matPlane);
	//REAL Declare_Var, Declare_VarEX, m[2], n[2];
	//Two_Product(matPlane[1][1], matPlane[3][3], m[1], m[0]);
	//Two_Product(matPlane[3][1], matPlane[1][3], n[1], n[0]);
	if (dist > 0) return Front;
	if (dist < 0) return Behind;
	return On;
}

double3 InexactComputePoint(const FixedPlane& p, const FixedPlane& q, const FixedPlane& r)
{
    double3x3 mat; 
    mat[0] = p.Normal();
    mat[1] = q.Normal();
    mat[2] = r.Normal();
	double D = -inexactDet3x3(mat);

	mat[0][0] = p.Distance();
	mat[1][0] = q.Distance();
	mat[2][0] = r.Distance();
	double D1 = inexactDet3x3(mat);

	mat[0][1] = p.Distance();
	mat[1][1] = q.Distance();
	mat[2][1] = r.Distance();

	mat[0][0] = p.Normal().x;
	mat[1][0] = q.Normal().x;
	mat[2][0] = r.Normal().x;
	double D2 = inexactDet3x3(mat);

	mat[0][2] = p.Distance();
	mat[1][2] = q.Distance();
	mat[2][2] = r.Distance();

	mat[0][1] = p.Normal().y;
	mat[1][1] = q.Normal().y;
	mat[2][1] = r.Normal().y;
	double D3 = inexactDet3x3(mat);

	return double3(D1/D, D2/D, D3/D);
}

bool IsPointVaild(const FixedPlane& p, const FixedPlane& q, const FixedPlane& r)
{
	double3x3 mat; 
    mat[0] = p.Normal();
    mat[1] = q.Normal();
    mat[2] = r.Normal();
	return adaptiveDet3x3Sign(mat) != 0;
}

std::ostream& operator<<(std::ostream& out, const FixedPlane& p)
{
	out << p.Normal()[0] << "\t" 
		<< p.Normal()[1] << "\t"
		<< p.Normal()[2] << "\t";
	out << p.Distance();
	out << std::endl;
	return out;
}

}