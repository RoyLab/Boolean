#include "IndexBuffer.h"

namespace GS{

IndexBuffer::IndexBuffer(EResourceUsage eUsage)
	:ID3DBuffer(eUsage)
{
}

IndexBuffer::~IndexBuffer()
{
}


bool IndexBuffer::CreateDeviceBuffer(ID3D11Device* device, void* srcBuffer, int size)
{
	D3D11_BUFFER_DESC BufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData;
	BufferDesc.Usage = MapToDeviceUsageFlag(meBufferType);
	BufferDesc.CPUAccessFlags = MapToCPUAccessFlag(meBufferType);
	BufferDesc.StructureByteStride =0 ;
	BufferDesc.MiscFlags  = 0;
	BufferDesc.ByteWidth = size ; 
	BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// set up initialize data
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = srcBuffer;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
    HRESULT result = device->CreateBuffer(&BufferDesc, &initData, &mpDeviceBuffer);
	if(FAILED(result))
	{
		return false;
	}
	return true;
}


}