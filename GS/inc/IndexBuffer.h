#ifndef _INDEX_BUFFER_H_
#define _INDEX_BUFFER_H_
#include "ID3DResource.h"

namespace GS{

class IndexBuffer: public ID3DBuffer{

public:	
	IndexBuffer(EResourceUsage eUsage);
	virtual ~IndexBuffer(); 
	bool CreateDeviceBuffer(ID3D11Device* device, void* data, int size); 
	
};

}


#endif 
