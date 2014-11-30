#include <assert.h>
#include "Camera.h"
#include "transform.h"
#include "configure.h"
#include "quaternion.h"

#define MATRIX_EPSILON 0.000001f

namespace GS{

static const double kOrthoRangeFactor  = 2.0;

Camera::Camera()
	:mViewMatrix(id4x4())
	,mbPerspective(false)
	,mNearPlane(0.01)
	,mFarPlane(1000.0)
	,mOrthoRange(0)
	,mEye(0)
{

	ComputeProjectMatrix(800.0, 600.0, mNearPlane, mFarPlane);
}


Camera::Camera(const float3& pos, const float3& target, const float3& up)
	:mbPerspective(false)
	,mNearPlane(0.01)
	,mFarPlane(1000.0)
	,mTarget(target)
	,mOrthoRange(0)
	,mEye(pos)
{
	ComputeViewMatrix(mEye, mTarget, up);
	ComputeProjectMatrix(800.0, 600.0, mNearPlane, mFarPlane);
	
}

void Camera::SetPosition(const float3& pos , const float3& target)
{
	float3 upVector = Up();
	mTarget = target;
	mEye = pos;
	ComputeViewMatrix(mEye, mTarget, upVector);

}

void Camera::SetPosition(const float3& pos , const float3& target, const float3& up)
{
 
	mTarget = target;
	mEye = pos;
	ComputeViewMatrix(mEye, mTarget, up);

}

Camera& Camera::operator=(const Camera& camera)
{
	float fieldWidth = HorizontalExtent();
	float fieldHeight = VerticalExtent();
	mViewMatrix  =    camera.mViewMatrix;
	mTarget      =    camera.mTarget;
	mEye         =    camera.mEye;
	mOrthoRange  =   camera.mOrthoRange;
	mbPerspective  = camera.mbPerspective ;
	mNearPlane = camera.mNearPlane;
	mFarPlane = camera.mFarPlane;
	ComputeProjectMatrix(fieldWidth,fieldHeight, mNearPlane, mFarPlane);
	return *this;

}

void Camera::SetProjectionMode(bool bPrespective)
{
	if (mbPerspective != bPrespective)
	{
		mbPerspective = bPrespective;
		ComputeProjectMatrix(HorizontalExtent(), VerticalExtent(), mNearPlane, mFarPlane);
	}
}

void  Camera::SetProjectionPlanes(float Near, float Far)
{
	if (Near != mNearPlane || Far != mFarPlane)
	{
		mNearPlane = Near;
		mFarPlane = Far;
		ComputeProjectMatrix(HorizontalExtent(), VerticalExtent(), mNearPlane, mFarPlane);	 
	}	
}


void Camera::SetExtends(float w, float h)
{
	if (w != HorizontalExtent() || h != VerticalExtent() )
		ComputeProjectMatrix(w, h, mNearPlane, mFarPlane);
}

void Camera::SetOrthoRange(float orthoRange)
{
	if (orthoRange != mOrthoRange)
	{
		mOrthoRange = orthoRange;
		ComputeProjectMatrix(HorizontalExtent(), VerticalExtent(), mNearPlane, mFarPlane);	 
	}
}

inline
void Camera::ComputeViewMatrix(const float3& pos, const float3& target, const float3& up)
{
	float3 ndir  = normalize(target - pos);
    float3 right = normalize(cross( up, ndir));
	float3 upVector =  cross(ndir, right);
	mViewMatrix = id4x4();
    mViewMatrix[0].x = right.x;
    mViewMatrix[1].x = right.y;
    mViewMatrix[2].x = right.z;
    mViewMatrix[0].y = up.x;
    mViewMatrix[1].y = up.y;
    mViewMatrix[2].y = up.z;
    mViewMatrix[0].z = ndir.x;
    mViewMatrix[1].z = ndir.y;
    mViewMatrix[2].z = ndir.z;
    mViewMatrix[0].w = 0;
    mViewMatrix[1].w = 0;
    mViewMatrix[2].w = 0;
    mViewMatrix[3].x =  - dot( right, pos);
    mViewMatrix[3].y = - dot( up, pos);
    mViewMatrix[3].z =  - dot( ndir, pos);
    mViewMatrix[3].w = 1;
}


inline 
void Camera::ComputeProjectMatrix(float W, float H, float Near, float Far)
{
	assert(W >0 && H >0);
	if (Near + MATRIX_EPSILON > Far)
            Far = Near + MATRIX_EPSILON;
	float  depth = Far - Near;
    float  oneOverDepth = 1/depth; 
    // Initialize the world matrix to the identity matrix.
	mProjectionMatrix = id4x4();
	//// Setup the projection matrix
	if (!mbPerspective)
	{
	//
    // General form of the orthographic Projection Matrix
    //
    // 
    //   2/w        0       0       0
    //    0        2/h      0       0
    //    0         0      1/(f-n)  0
    //    0         0     -n/(f-n)  1
    //
		mProjectionMatrix[0][0] = 2/ W;
        mProjectionMatrix[1][1] = 2 / H;
        mProjectionMatrix[2][2] = oneOverDepth;
        mProjectionMatrix[3][2] = -Near * oneOverDepth;


	}
	else
	{
		// General form of the perspective Projection Matrix
		// 
        //   2n/w       0       0       0
        //    0        2n/h     0       0
        //    0         0     f/(f-n)   1
        //    0         0    -fn/(f-n)  0
        //
		mProjectionMatrix[0][0] = (2 * Near) / W;
        mProjectionMatrix[1][1] = (2 * Near) / H;
        mProjectionMatrix[2][2] = Far * oneOverDepth;
        mProjectionMatrix[3][2] = (-Far * Near) * oneOverDepth;
        mProjectionMatrix[2][3] = 1;
        mProjectionMatrix[3][3] = 0;


	}

}


void Camera::ComputeFovProjectMatrix(float fov, float aspect, float Near, float Far )
{
    //
    // General form of the Projection Matrix
    //
    // (unit height) uh = Cot( fov/2 ) == 1/Tan(fov/2)
    // (unit width) uw / uh = aspect (same as in television industry, e.g. 4:3, 16:9)
    // 
    //   uw         0       0       0
    //    0        uh       0       0
    //    0         0      f/(f-n)  1
    //    0         0    -fn/(f-n)  0
    //
    mProjectionMatrix = id4x4();

    if (Near + MATRIX_EPSILON > Far)
        Far = Near + MATRIX_EPSILON;

    float  depth = Far - Near;
	float  oneOverDepth = 1/depth;   

    mProjectionMatrix[1][1] = 1 / tan(0.5f * fov);
    mProjectionMatrix[0][0] = mProjectionMatrix[1][1] / aspect;
    mProjectionMatrix[2][2] = Far * oneOverDepth;
    mProjectionMatrix[3][2] = (-Far * Near) * oneOverDepth;
    mProjectionMatrix[2][3] = 1;
    mProjectionMatrix[3][3] = 0;
}

inline 
float Camera::HorizontalExtent() const
{
        // avoid division by 0
        if ( mProjectionMatrix[0][0] == 0 ) return 0;
		if (mbPerspective )
			return fabs(2.0f * mNearPlane / mProjectionMatrix[0][0]);
        else
			return fabs(2.0f/ mProjectionMatrix[0][0]);
}

inline 
float Camera::VerticalExtent() const
{
    // avoid division by 0
    if ( mProjectionMatrix[1][1] == 0 ) return 0;

    if (IsPerspective() )
        return fabs(2.0f * mNearPlane / mProjectionMatrix[1][1]);
    else
        return fabs(2.0f/ mProjectionMatrix[1][1]);

}

void Camera::Orbit(const float3& centerPoint, double yawDelta,double pitchDelta)
{
	 // The new orbit constrains roll to zero
	float3 direction = Direction();
	double pitch = atan2(direction.z, length( float2(direction.x,direction.y)));
    if ((pitch + pitchDelta) > PITCH_MAX)
        pitchDelta = PITCH_MAX - pitch;
    if ((pitch + pitchDelta) < PITCH_MIN)
        pitchDelta = PITCH_MIN - pitch;
 
	float4x4 posInvMatrix = translate(-vector(centerPoint));
	float4x4 posMatrix = translate(vector(centerPoint));

	float4 q = quat_axis_rotate(pitchDelta, vector(Right()));
	float4x4 rotMat = quat_rotation_matrix(q);
	float4x4 t2= mul(rotMat, posInvMatrix);
	float4x4 pitchMatrix = mul(posMatrix, t2 ); 

    q = quat_axis_rotate(yawDelta, vector(Up()));
	rotMat = quat_rotation_matrix(q);
	t2= mul(rotMat, posInvMatrix);
	float4x4 yawMatrix = mul(posMatrix, t2 ); 

	float3 pos  = mul(pitchMatrix, vector(Eye()) ).xyz;
	pos = mul(yawMatrix, vector(pos)).xyz;

	float3 upVector = Up();
	upVector = mul(pitchMatrix, vector(upVector)).xyz;
	upVector = mul(yawMatrix, vector(upVector)).xyz;
	
	float3 target= mTarget;
	if (!vec3_equal(target, centerPoint))
	{
		target = mul(pitchMatrix, vector(target)).xyz;
		target = mul(yawMatrix, vector(target)).xyz;
	}
	SetPosition(pos, target, upVector);
}


bool Camera::SetOrthoRange (const Box3& bounds)
{
    bool valid = true;
	if (! IsPerspective() && !bounds.IsEmpty()) 
    {
        // update the camera if the range must be changed
        double      boundsMin, boundsMax;
        ComputeOrthoRange(bounds, boundsMin, boundsMax);
        
        double cameraMin, cameraMax;
        MinMax(cameraMin, cameraMax);
        
        const double boundsRange = boundsMax - boundsMin;
        const double cameraRange = cameraMax - cameraMin;
        const double maxBoundsRange = boundsRange * kOrthoRangeFactor * kOrthoRangeFactor;
        
        if (boundsMin < cameraMin || boundsMax > cameraMax) // geometry bounds exceed ortho range
        {
            valid = false;
            SetMinMax(kOrthoRangeFactor, boundsMin, boundsMax);;
        }
        else if ((boundsRange != 0.0) && (maxBoundsRange < cameraRange)) // ortho range too loose for bounds
        {
            valid = false;
            SetMinMax(kOrthoRangeFactor, boundsMin, boundsMax);
        }
    } 

    return valid;
}


void Camera::SetMinMax (const double inflateFactor, const double min, const double max)
{
    assert (inflateFactor >= 1.0);
	assert (!IsPerspective());
    assert (max >= min);

	double     distance    = Distance();
	const float3 direction = Direction();
    
    const double        kMinDistance = 0.001;
    float3 new_target,new_position;

    // shift camera target to middle of min and max
    const double targetShift = 0.5*(max + min) - distance;
    new_target   = mTarget + targetShift*direction;

    // modify the clip to match the target shiftting.
    mNearPlane -= targetShift;
    mFarPlane  -= targetShift;
    if (distance < kMinDistance)
    {
        // shift camera position by additional amount s.t. distance equals kMinDistance
        const double pos_shift = 0.5*(max + min) - (distance + kMinDistance);
        new_position = Eye() + pos_shift*direction;
        distance = kMinDistance;
    }
    else
        // shift camera position by same amount
        new_position = Eye() + targetShift*direction;
	
	SetPosition(new_position, new_target);

    const double kMinOrthoRange = 0.02;
    if (0.5*(max - min) < distance*kMinOrthoRange)
        SetOrthoRange (kMinOrthoRange);
    else
    {
        double new_ortho_range = 0.5*(max - min)/distance; // tightly bounds geometry 
        new_ortho_range *= inflateFactor; // inflate bounds by inflateFactor
        SetOrthoRange (new_ortho_range);
    }
}


void Camera::MinMax(double& min, double& max) const
{
	const float distance = Distance();

    min = (1.0 - mOrthoRange)*distance;
    max = (1.0 + mOrthoRange)*distance;
}

// Compute position of geometry front and rear bounds
void Camera::ComputeOrthoRange (const Box3& bounds, double& min, double& max) const
{
	if (!bounds.IsEmpty()) 
    {
        Box3 transformedBounds = bounds;   
		transformedBounds.TransformBy(ViewMatrix());
       
		float3  new_pos = mul(point(Eye()), ViewMatrix()).xyz;
		max  = transformedBounds.Max().z - new_pos.z;
		min  = transformedBounds.Min().z - new_pos.z;
    }
    else // no bounds: assume current ortho range bounds geometry
    {
        MinMax(min, max);
    }

    return;
}


}
