#include "VertexBuffer.h"

namespace GS {

VertexBuffer::VertexBuffer(EResourceUsage eUsage)
	:ID3DBuffer(eUsage)
{

}

VertexBuffer::~VertexBuffer()
{
}

bool VertexBuffer::CreateDeviceBuffer(ID3D11Device* device, void* srcBuffer, int size)
{
	D3D11_BUFFER_DESC BufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData;
	BufferDesc.Usage = MapToDeviceUsageFlag(meBufferType);
	BufferDesc.CPUAccessFlags = MapToCPUAccessFlag(meBufferType);
	BufferDesc.StructureByteStride =0 ;
	BufferDesc.MiscFlags  = 0;
	BufferDesc.ByteWidth = size ; 
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

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

 void* VertexBuffer::MapDeviceBuffer(EAccessFlags access, unsigned int offset, unsigned int size)
    {
        OGSAssertReturnPtr((offset+size) <= (unsigned int)mOwnerBuffer->CapacityInFloats()); 
        OGSAssertReturnPtr(mMapAccess == EAccessNone); // must not already be mapped

        OGSAssertReturnPtr(mBuffer != NULL);

        //get d3d11 access flag and map flag, also check if it need a copy operation
        D3D11_MAP map;
        unsigned int cpu_access = 0;
        bool needcopy = false;

        // This is the only assertion we can make on the mAccess, because we do not follow the DX11 spec.
        // For example, to map a buffer with D3D11_MAP_READ map flag, DX11 spec requires D3D11_CPU_ACCESS_READ 
        // CPU access flag and D3D11_USAGE_STAGING usage flag in creation time, but we do not.
        if (mOwnerBuffer->Access() == EAccessNone)
        {
            Sys::Warning(L"DX11: Cannot map a buffer created with no CPU access.");
            return NULL;
        }

        D3D11_BUFFER_DESC desc;
        mBuffer->GetDesc(&desc);

        switch (access)
        {
        case ReadOnly:
            {
                map = D3D11_MAP_READ;
                cpu_access = D3D11_CPU_ACCESS_READ;

                //only staging usage can be mapped by CPU read
                if (desc.Usage != D3D11_USAGE_STAGING) 
                    needcopy = true;
                break;
            }
        case ReadWrite:
            {
                map = D3D11_MAP_READ_WRITE;
                cpu_access = D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;

                //only staging usage can be mapped by CPU read
                if (desc.Usage != D3D11_USAGE_STAGING) 
                    needcopy = true;
                break;
            }
        case WriteOnly:  
            {
                map = D3D11_MAP_WRITE;
                cpu_access = D3D11_CPU_ACCESS_WRITE;

                //only dynamic/staging usage can be mapped by CPU write
                if (desc.Usage != D3D11_USAGE_STAGING)
                    needcopy = true;
                break;
            }
        case WriteDiscard:
            {
                map = D3D11_MAP_WRITE_DISCARD;
                cpu_access = D3D11_CPU_ACCESS_WRITE;

                //only dynamic usage can be mapped by CPU write discard
                if (desc.Usage != D3D11_USAGE_DYNAMIC)
                {
                    Sys::Warning(L"Can't map this map with access flag WriteDiscard.");
                    return NULL;
                }
                break;
            }
        case WriteNoOverwrite: 
            {
                map = D3D11_MAP_WRITE_NO_OVERWRITE;
                cpu_access = D3D11_CPU_ACCESS_WRITE;

                //only dynamic usage can be mapped by CPU write no overwrite
                if (desc.Usage != D3D11_USAGE_DYNAMIC)
                {
                    Sys::Warning(L"Can't map this map with access flag WriteNoOverwrite.");
                    return NULL;
                }
                break;
            }
        default: ErrorReturnPtr();
        }


        // set up the buffer to map
        ID3D11Buffer* buffer = mBuffer;

        //get dx11 device and context
        ID3D11Device* pDevice = (ID3D11Device*)(mVD->GPUDevice());

        if (needcopy)
        {
            //warning information
            // It turns out this condition is fine for DX10 (or at least no worse that DX9),
            // i.e. it's normal to do this sort of mapping, according to Adam. Warning removed.
            //Sys::Warning(L"Mapping flag doesn't match resource usage,the map will be a low performance operation");

            //check if staging buffer in NULL, or the vertex buffer is in map now
            OGSAssertReturnPtr(mStagingBuffer == NULL);

            //create staging buffer description
            D3D11_BUFFER_DESC bufferDesc;

            bufferDesc.ByteWidth = sizeof(float) * mOwnerBuffer->CapacityInFloats();
            bufferDesc.BindFlags = 0; // can't bind a staging resource
            bufferDesc.Usage     = D3D11_USAGE_STAGING;
            bufferDesc.CPUAccessFlags = cpu_access;
            bufferDesc.MiscFlags = 0;

            //create a staging buffer for data-copy
            HRESULT hr;
            OGS_DEVICECALLDX11(pDevice->CreateBuffer( &bufferDesc, NULL, &buffer ), return NULL);

            if ( FAILED(hr)|| buffer == NULL )
            {
                OGS_INTERPRET_ERROR_CODE_DX11( L"CreateBuffer() staging buffer", hr );
                return NULL;
            }

            {
                // Use lock in virtual device to protect device context operation
                VD_TRYTOLOCK(mVD);

                // copy the vb to the staging buffer
                DX_IMMEDIATECONTEXT(mVD)->CopyResource( buffer, mBuffer );
            }
            mStagingBuffer = buffer;

            // add the size of staging buffer to memory monitor
            OGS::Objects::ObjModel::MemoryMonitor().AddToDeviceMemory(mOwnerBuffer->CapacityInFloats() * sizeof(float));
        }

        HRESULT hr;
        // now map the buffer or the staging buffer
        D3D11_MAPPED_SUBRESOURCE mappedSub;
        {
            //TODO DX11 MULTITHREADING
            // Buffers with DYNAMIC_USAGE flags can be Mapped/Unmapped on a Deferred Context.
            // But there are some restrictions on how WRITE_DISCARD and WRITE_NO_OVERWRITE flags should be used.
            // We currently don't support all workflows for using WRITE_DISCARD and WRITE_NO_OVERWRITE in Deferred Context.
            // Once this support is enabled we can do Map/Unmap on Deferred Context.
            {
                // Use lock in virtual device to protect device context operation
                VD_TRYTOLOCK(mVD);
                hr=DX_IMMEDIATECONTEXT(mVD)->Map( buffer, 0, map, 0, &mappedSub );
            }
        }

        if( FAILED(hr) ) { 
            OGS_INTERPRET_ERROR_CODE_DX11( L"Map() vertex", hr );

            //release created staging buffer
            if (mStagingBuffer)
            {
                buffer->Release();
                mStagingBuffer = NULL;
            }

            if(hr == DXGI_ERROR_DEVICE_REMOVED)
            {
                // The video card has been physically removed from the system, a driver upgrade for the
                // GPU has occurred, or the GPU driver was disabled.
                // At this point we need to release this and the device will have to be recreated by
                // the client application using the DeviceRemovedCB() callback.
                // See http://msdn.microsoft.com/en-us/library/windows/desktop/bb509553(v=vs.85).aspx
                OGSAssert(mVD != NULL);
                ID3D11Device * const pDevice = static_cast<ID3D11Device * const>(mVD->GPUDevice());

                static_cast<DeviceContextDx11*>(mVD)->OnDeviceRemoved(pDevice);
            }

            return NULL;
        }
        float* dstBuffer = (float*)mappedSub.pData;

        mMapAccess = access;
        mOwnerBuffer->MappedBuffer(dstBuffer);

        return dstBuffer + offset;
    }

    /// <description>
    /// unlock the vertex buffer
    /// </description>
    void VertexDeviceBufferDx11::UnMapDeviceBuffer()
    {
        OGSAssert( mMapAccess != EAccessNone ); // must be locked to unlock
        // is it read or read/write access via a staging buffer?


        if( mStagingBuffer != NULL )
        {
            // Use lock in virtual device to protect device context operation
            VD_TRYTOLOCK(mVD);

            DX_IMMEDIATECONTEXT(mVD)->Unmap( mStagingBuffer, 0 );
            // do we need to update the vb from the staging buffer
            if( mMapAccess != ReadOnly )
            {
                // copy the staging buffer to the vb
                DX_IMMEDIATECONTEXT(mVD)->CopyResource( mBuffer, mStagingBuffer );
            }

            // release the staging buffer for now
            mStagingBuffer->Release();
            mStagingBuffer = NULL;

            // reduce the memory size
            OGS::Objects::ObjModel::MemoryMonitor().ReduceFromDeviceMemory(mOwnerBuffer->CapacityInFloats() * sizeof(float));
        }
        else if (mBuffer != NULL)
        {
            // The map flag will always be WriteDiscard or WriteNoOverwrite here. So we can use
            // deferred context.

            //TODO DX11 MULTITHREADING
            // Buffers with DYNAMIC_USAGE flags can be Mapped/Unmapped on a Deferred Context.
            // But there are some restrictions on how WRITE_DISCARD and WRITE_NO_OVERWRITE flags should be used.
            // We currently don't support all workflows for using WRITE_DISCARD and WRITE_NO_OVERWRITE in Deferred Context.
            // Once this support is enabled we can do Map/Unmap on Deferred Context.

            DX_IMMEDIATECONTEXT(mVD)->Unmap(mBuffer, 0);
        }

        mMapAccess = EAccessNone;
    }




}

