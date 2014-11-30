#ifndef _ID3D_RESOURCE_H
#define _ID3D_RESOURCE_H
#include <d3d11.h>
namespace GS{

 enum EResourceUsage{
	 Default,
	 Immutable,
	 Dynamic,
	 Staging
 };

 enum EAccessFlags
 {      
	    AccessNone,
        ReadOnly ,
        WriteOnly,
        ReadWrite,

        /// A new device buffer may be provided to the client if the existing device buffer is
        /// currently being used by the GPU (i.e. from a draw call). 
        WriteDiscard,

        /// The client must ensure that any data in the buffer currently being used by the GPU
        /// (i.e. from a draw call) is not modified; other data can be modified.  
        WriteNoOverwrite,
 };

class ID3DBuffer{

public: 
	ID3DBuffer(EResourceUsage eUsageType);
	virtual ~ID3DBuffer() {} 
	virtual bool  CreateDeviceBuffer(ID3D11Device* device, void* srcBuffer, int size)= 0; 
	virtual void  ReleaseDeviceBuffer() ; 
			bool  IsMappable(EAccessFlags access) const ;         
protected:
	static D3D11_USAGE MapToDeviceUsageFlag(EResourceUsage eType);
	static UINT        MapToCPUAccessFlag(EResourceUsage eType);
protected:
	ID3D11Buffer*   mpDeviceBuffer;	
    EResourceUsage  meBufferType;
};

inline 
ID3DBuffer::ID3DBuffer(EResourceUsage eUsageType)
	:mpDeviceBuffer(NULL)
	,meBufferType(eUsageType)
{
}

inline 
void ID3DBuffer::ReleaseDeviceBuffer()  
{
	if(mpDeviceBuffer)
	{
		mpDeviceBuffer->Release();
		mpDeviceBuffer = NULL;
	}
}

inline 
D3D11_USAGE ID3DBuffer::MapToDeviceUsageFlag(EResourceUsage eType)
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
UINT ID3DBuffer::MapToCPUAccessFlag(EResourceUsage eType)
{
	if (eType == Dynamic)
		return D3D11_CPU_ACCESS_WRITE;
	if (eType == Staging)
		return (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
    return 0;
}

inline 
bool ID3DBuffer::IsMappable(EAccessFlags access) const  
{

        switch(meBufferType)
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

#endif 