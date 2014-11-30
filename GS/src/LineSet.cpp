#define _USE_MATH_DEFINES 
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <assert.h>
#include "arithmetic.h"
#include "LineSet.h"


namespace GS{

LineSet::LineSet(bool bUseUniqueVertex )
    : mbUniqueVertex(bUseUniqueVertex)
    , mPrimitiveCapacity(INIT_PRIMITIVE_CAPACITY)
    , mPointsCapacity(INIT_VERTEX_CAPACITY)
{
    mPoints.reserve(mPointsCapacity);
    mSegs.reserve(INIT_PRIMITIVE_CAPACITY);
}

LineSet::~LineSet()
{

	mPoints.clear();
    mSegColorTable.clear();
	mSegs.clear();
    mVertexDict.clear();
	if (mpVB)
	{
		delete mpVB;
		mpVB = NULL;
	}
	if (mpIB)
	{
		delete mpIB;
		mpIB = NULL;
	}

}

int LineSet::AddLine(const double3 & s, const double3& t, const float4& color)
{
    int vId1 = AddVertex(s);
    int vId2 = AddVertex(t);
    return AddSegment(vId1, vId2,  color);
}

int  LineSet::AddSegment(int vId1, int vId2, const float4& color)
{
	if (vId1 == vId2)
		return -1 ;
	assert(vId1 >=0 && vId2 >=0 );
    CheckPrimitiveCapacity();
	SegInfo info;
    info.Id0 = vId1;
    info.Id1 = vId2;
	int idx = -1; 
    for (int i = 0; i < mSegColorTable.size(); i++)
    {
        if (vec4_equal(mSegColorTable[i], color))
        {
            idx = i; 
            break;
        }
    }
    if (idx == -1)
    {
		mSegColorTable.push_back(color);
        idx= mSegColorTable.size() -1;
    }
    info.ColorIdx = idx;
	mSegs.push_back(info);
    return mSegs.size()-1;
}
int	LineSet::AddVertex(const double3& v)
{
	int vertexIdx = -1; 
	std::hash_map<Point3D, int,Vec3HashCompare<double> >::iterator it  = mVertexDict.find (v);
    if (mbUniqueVertex && it != mVertexDict.end())
		vertexIdx = mVertexDict[v];
	if(!mbUniqueVertex || vertexIdx == -1)
	{
        vertexIdx = mPoints.size();
		CheckPointCapacity();
		mPoints.push_back(v);
		if (mbUniqueVertex)
			mVertexDict[v] = vertexIdx;
	}
	return vertexIdx;
}


void LineSet::Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData)
{
    
   if (PrimitiveCount() ==0)
		return ; 
	if (!mpVB)
		mpVB = new D3D11Buffer(GS::Dynamic, GS::Vertex);
	if (!mpIB)
		mpIB = new D3D11Buffer(GS::Dynamic, GS::Index);
	if (bForceFillData)
	{
		mpVB->ReleaseDeviceBuffer();
		mpIB->ReleaseDeviceBuffer();
	}

	if (!mpVB->IsBufferAvailable())
	{
		mpVB->CreateDeviceBuffer(device, NULL, PrimitiveCount()*2* GetVertexStride() );
		if (!mpIB->IsBufferAvailable())
			mpIB->CreateDeviceBuffer(device, NULL, PrimitiveCount()*2* sizeof (unsigned int));
		FillD3DBuffer(dc,mpVB, 0, mpIB, 0);
	}
	unsigned int stride;
	unsigned int offset;
	ID3D11Buffer* pVB = mpVB->GetDeviceBuffer();

	// Set vertex buffer stride and offset.
	stride = GetVertexStride(); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	dc->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	dc->IASetIndexBuffer(mpIB->GetDeviceBuffer(), DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	// Render the triangle.
	dc->DrawIndexed(PrimitiveCount()*2, 0, 0);
}

void LineSet::FillD3DBuffer(ID3D11DeviceContext* dc,D3D11Buffer* mpVB, int VBOffset, 
                                        D3D11Buffer* mpIB, int IBOffet)
{
    
      struct VertexType{
		float3 vertex;
		float4 color;
		float3 normal;
	};
      int VBSize = mPoints.size()*GetVertexStride();
	unsigned char * pVBStart = (unsigned char *)mpVB->MapDeviceBuffer(dc, GS::WriteDiscard, VBOffset, VBSize);
	assert(pVBStart!= NULL);
	VertexType* pVBData =(VertexType*)(pVBStart+ VBOffset);

	int n = PrimitiveCount();
	for(int i = 0 ; i< n; i++)
	{
       
        pVBData->vertex = mPoints[mSegs[i].Id0];
        pVBData->color =  mSegColorTable[mSegs[i].ColorIdx];
        pVBData->normal = 0.0f;
		pVBData++;

		pVBData->vertex =  mPoints[mSegs[i].Id1];
		pVBData->color =  mSegColorTable[mSegs[i].ColorIdx];
		pVBData->normal = 0.0f;
		pVBData++;

	
	}
	struct IndexType {
		unsigned int v[2];

	};
	int IBSize = PrimitiveCount()*2* sizeof (unsigned int);
	unsigned char * pIBStart = (unsigned char *)mpIB->MapDeviceBuffer(dc, GS::WriteDiscard, IBOffet, IBSize);
		assert(pIBStart!= NULL);
    IndexType* pIBData =(IndexType*)(pIBStart + IBOffet);
	for (int i = 0 ; i < PrimitiveCount(); i++)
	{
		pIBData->v[0] = 2*i;
		pIBData->v[1] = 2*i + 1;
		pIBData++;
	}
	mpVB->UnMapDeviceBuffer(dc);
	mpIB->UnMapDeviceBuffer(dc);
}
    
   

int LineSet::GetVertexStride() const 
{
	return sizeof(float3)+ sizeof(float4)+sizeof(float3);
	
}


void LineSet::CheckPrimitiveCapacity()
{
	if (PrimitiveCount() >=  mPrimitiveCapacity)
	{
		mPrimitiveCapacity*=2;
		mSegs.reserve(mPrimitiveCapacity);
	}
}

void LineSet::CheckPointCapacity()
{
 
    if ( mPoints.size() >= mPointsCapacity)
	{
		mPointsCapacity *=2;
		mPoints.reserve(mPointsCapacity);
	}
}



}

