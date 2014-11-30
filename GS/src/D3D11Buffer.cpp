#include "D3D11Buffer.h"

namespace GS{


D3D11Buffer::D3D11Buffer(EResourceUsage eUsageType, EBufferType eBufferType)
	: mpDeviceBuffer(NULL)
	, meBufferUsage(eUsageType)
	, meBufferType(eBufferType)
{
	
}

D3D11Buffer::~D3D11Buffer()
{
	ReleaseDeviceBuffer();
}

bool  D3D11Buffer::CreateDeviceBuffer(ID3D11Device* device, void* srcBuffer, int size)
{
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.Usage = MapToDeviceUsageFlag(meBufferUsage);
	BufferDesc.CPUAccessFlags = MapToCPUAccessFlag(meBufferUsage);
	BufferDesc.StructureByteStride =0 ;
	BufferDesc.MiscFlags  = 0;
	BufferDesc.ByteWidth = size ; 
	BufferDesc.BindFlags = MapToBufferBindingFlag(meBufferType);
	
	HRESULT result ;
	// set up initialize data
	if (srcBuffer)
	{
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = srcBuffer;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;
		// Now create the vertex buffer.
	    result = device->CreateBuffer(&BufferDesc, &initData, &mpDeviceBuffer);
	}
	else 
	   result = device->CreateBuffer(&BufferDesc, NULL, &mpDeviceBuffer);
	if(FAILED(result))
	{
		return false;
	}
	return true;
}

void* D3D11Buffer::MapDeviceBuffer(ID3D11DeviceContext* dc,EAccessFlags access, unsigned int offset, unsigned int size)
{
	if (!IsMappable(access))
		return NULL;
     D3D11_MAP map;
	 switch (access)
     {
        case ReadOnly:
            {
                map = D3D11_MAP_READ;
                break;
            }
        case ReadWrite:
            {
                map = D3D11_MAP_READ_WRITE;
                break;
            }
        case WriteOnly:  
			{
				map = D3D11_MAP_WRITE;
                break;
            }
        case WriteDiscard:
            {
                map = D3D11_MAP_WRITE_DISCARD;
                break;
            }
        case WriteNoOverwrite: 
            {
                map = D3D11_MAP_WRITE_NO_OVERWRITE;
                break;
            }
        default: return NULL;
    }
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedSub;
	result = dc->Map(mpDeviceBuffer, 0, map, 0, &mappedSub);
	if(FAILED(result))
	{
		return NULL;
	}
	return mappedSub.pData;
}

inline
void  D3D11Buffer::UnMapDeviceBuffer(ID3D11DeviceContext* deviceContext)
{
	deviceContext->Unmap(mpDeviceBuffer, 0);
}

void  D3D11Buffer::ReleaseDeviceBuffer()
{
	if(mpDeviceBuffer)
	{
		mpDeviceBuffer->Release();
		mpDeviceBuffer = NULL;
	}
}


bool  D3D11Buffer::Update(ID3D11DeviceContext* dc, int offsetInFloats, int countInFloats, void* pData, bool discardOnPartialUpdate)
{ 

	switch (meBufferUsage)
    {
		case Default:
        {
                // D3D11 spec is totally wrong in saying UpdateSubresource() uses CPU to copy data. This method
                // uses GPU to copy data. The resource has to be created with D3D11_USAGE_DEFAULT, which is
                // allowed for GPU read/write and disallowed for CPU read/write.
                D3D11_BOX box;
                box.front = 0; box.back = 1; 
                box.top = 0; box.bottom = 1; 
                box.left = offsetInFloats * sizeof(float);  // offset in bytes
                box.right = (offsetInFloats+countInFloats) * sizeof(float); // "right end" in bytes
                //{
      
                //    const void* pAdjustedSrcData = pData;
                //    if(!mVD->Caps()->SupportHWCmdList())
                //    {
                //        // Offset the source location - this must be done because of a bug in DirectX 11 itself.
                //        // See http://msdn.microsoft.com/en-us/library/windows/desktop/ff476486(v=vs.85).aspx
                //        // "If your application calls UpdateSubresource on a deferred context with a destination
                //        // box ?to which pDstBox points ?that has a non-(0,0,0) offset, and if the driver does
                //        // not support command lists, UpdateSubresource inappropriately applies that destination-box
                //        // offset to the pSrcData parameter. To work around this behavior, use the following code"
                //        //
                //        // In other words, they add an offset of box.left to the pData source pointer, from what I can tell.
                //        // They give a fix of
                //        //     - (alignedBox.left * srcBytesPerElement);
                //        // I believe that we're saying the bytes per element is 1, since box.left is in terms of bytes, so:
                //        pAdjustedSrcData = ((const BYTE*)pData) - box.left;

                //        // comment from http://blogs.msdn.com/b/chuckw/archive/2010/07/28/known-issue-direct3d-11-updatesubresource-and-deferred-contexts.aspx
                //        // "The workaround works for most of the cases. But the pointer pAdjustedSrcData is unsafe.
                //        // UpdateSubResource will check if the fourth parameter is a valid read pointer. So if
                //        // unfortunately this pointer points to a buffer which is not for read, UpdateSubresource will fail."

                //        // Yet more info here: http://www.gamedev.net/topic/618854-questions-about-mapupdatesubresource/ - deep waters
                //    }

      
                    dc->UpdateSubresource( 
                        mpDeviceBuffer,            // destination resource
                        0,                  // destination subresource
                        &box,               // portion of destination to copy to 
                        pData,   // the source data
                        0,   // size of one row of the source data - "row pitch"
                        0);  // size of one depth slice of source data - examples seem to set to zero, I guess it's not used?
				return true;
			}
              
           
        case Dynamic:
            {
                // Dynamic buffers are allowed for GPU read and CPU write. We have to map for CPU write and then call memcpy.
                EAccessFlags mapFlag;
                if (offsetInFloats == 0/* && countInFloats == mCapacityInFloats*/)
                {
                    mapFlag = WriteDiscard;
                }
                else
                {
                    mapFlag = discardOnPartialUpdate? WriteDiscard : WriteNoOverwrite;
                }
                float* pDst = (float*)MapDeviceBuffer(dc, mapFlag, 0, 0);
				if(pDst == NULL) return false ;
                memcpy_s( &pDst[offsetInFloats], countInFloats * sizeof(float), pData, countInFloats * sizeof(float) );
                UnMapDeviceBuffer(dc);
                return true;
            }
        default:
          return false ;
        }// End switch
	return false ;

}


inline 
D3D11_USAGE D3D11Buffer::MapToDeviceUsageFlag(EResourceUsage eType)
{
	switch (eType)
	{
		
		case Dynamic:
			return D3D11_USAGE_DYNAMIC;
		case Immutable:
			return D3D11_USAGE_IMMUTABLE;
		case Staging:
		    return D3D11_USAGE_STAGING;
	   default:
			return D3D11_USAGE_DEFAULT;
	}

}

inline 
UINT D3D11Buffer::MapToCPUAccessFlag(EResourceUsage eType)
{
	if (eType == Dynamic)
		return D3D11_CPU_ACCESS_WRITE;
	if (eType == Staging)
		return (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
    return 0;
}

inline 
UINT D3D11Buffer::MapToBufferBindingFlag(EBufferType eType)
{
	switch (eType)
	{
	case Vertex: return D3D11_BIND_VERTEX_BUFFER;
	case Index:  return D3D11_BIND_INDEX_BUFFER;
	case Const:  return D3D11_BIND_CONSTANT_BUFFER;
	}

}

inline 
bool D3D11Buffer::IsMappable(EAccessFlags access) const  
{

        switch(meBufferUsage)
        {
        case Dynamic:
            {
                // WriteDiscard and WriteNoOverwrite should only be used with Dynamic usage.
                if( (access == ReadWrite) || (access == WriteOnly)
                    || (access == WriteDiscard) || (access == WriteNoOverwrite) )
                    return true;
            }
            break;
        case Staging:
            {
                if( (access == ReadOnly) || (access == ReadWrite) || (access == WriteOnly) )
                    return true;
            }
            break;
        default:
            break;
        }
        return false;
}

}