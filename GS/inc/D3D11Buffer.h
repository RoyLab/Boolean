#ifndef _D3D11_BUFFER_H
#define _D3D11_BUFFER_H
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

enum EBufferType{
	Index,
	Vertex, 
	Const
};

class D3D11Buffer{

public: 
	D3D11Buffer(EResourceUsage eUsageType, EBufferType eBufferType);
	 ~D3D11Buffer();
    bool  CreateDeviceBuffer(ID3D11Device* device, void* srcBuffer, int size); 
	void* MapDeviceBuffer(ID3D11DeviceContext* dc, EAccessFlags access, unsigned int offset, unsigned int size);
	void  UnMapDeviceBuffer(ID3D11DeviceContext* deviceContext);
	bool  Update(ID3D11DeviceContext* dc, int offsetInFloats, int countInFloats, void* pData, bool discardOnPartialUpdate = false );
    void  ReleaseDeviceBuffer(); 
    ID3D11Buffer* GetDeviceBuffer() const {return mpDeviceBuffer;} 
	bool  IsMappable(EAccessFlags access) const ;
	bool  IsBufferAvailable() const {return mpDeviceBuffer != NULL;}
	EBufferType    BufferType()  const {return meBufferType;}
	EResourceUsage BufferUsage() const {return meBufferUsage;}
protected:
	static D3D11_USAGE MapToDeviceUsageFlag(EResourceUsage eType);
	static UINT        MapToCPUAccessFlag(EResourceUsage eType);
	static UINT        MapToBufferBindingFlag(EBufferType eType);
protected:
	ID3D11Buffer*   mpDeviceBuffer;	
    EResourceUsage  meBufferUsage;
	EBufferType     meBufferType;
};


};




#endif 