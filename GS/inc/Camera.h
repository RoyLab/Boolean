#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <D3DX10math.h>
#include "typedefs.h"
#include "arithmetic.h"
#include "Box3.h"

namespace GS{

class Camera{

public :
	Camera();
	Camera(const float3& pos, const float3& target, const float3& up);
	void  SetPosition(const float3& pos , const float3& target);
	void   SetPosition(const float3& pos , const float3& target, const float3& up);
	void  SetProjectionMode(bool bPrespective);
	void  SetProjectionPlanes(float Near, float Far);
	void  SetExtends(float w, float h);
	bool  SetOrthoRange (const Box3& bounds);
	void  SetOrthoRange (float orthoRange);
	void  Orbit(const float3& centerPoint, double yawDelta,double pitchDelta);
    Camera& operator=(const Camera&);
	const float4x4& ViewMatrix() const  { return mViewMatrix;}
 	const float4x4& ProjectionMatrix() const {return mProjectionMatrix;}
		  float   HorizontalExtent() const;
		  float   VerticalExtent()   const;
	      bool    IsPerspective() const { return mbPerspective;}
		  float3  Target()    const {return mTarget; } 
          float3  Eye()       const {return  mEye;}
	      float3  Direction() const {return float3(mViewMatrix[0].z, mViewMatrix[1].z, mViewMatrix[2].z);}
	      float3  Up()        const {return float3(mViewMatrix[0].y, mViewMatrix[1].y, mViewMatrix[2].y);}
	      float3  Right()     const {return float3(mViewMatrix[0].x, mViewMatrix[1].x, mViewMatrix[2].x);}
		  float   Distance() const  {return length(Target() - Eye());}
	
    void ComputeProjectMatrix(float W, float H, float Near, float Far);
	void ComputeFovProjectMatrix(float fov, float aspect, float Near, float Far );
	void ComputeViewMatrix(const float3& pos, const float3& target, const float3& up);
	void ComputeOrthoRange (const Box3& bounds, double& min, double& max) const;
	void SetMinMax (const double inflateFactor, const double min, const double max);
	void MinMax(double& min, double& max) const;
private:
	float4x4 mViewMatrix;
	float4x4 mProjectionMatrix;
	float3 mTarget;
	float3 mEye;
	float  mOrthoRange;
	float  mNearPlane;
	float  mFarPlane;
	bool   mbPerspective;
	

};

}

#endif 




