#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_
#include <D3DX11.h>

namespace GS{
class Geometry{

public:
    Geometry() {}
    virtual ~Geometry() {}
    virtual int PrimitiveCount() const =0;
    virtual void  Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData)= 0;
};
}

#endif 
