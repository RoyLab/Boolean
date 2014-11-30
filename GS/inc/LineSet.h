#ifndef _LINE_SET_H_
#define  _LINE_SET_H_
#include <vector>
#include "configure.h"
#include "typedefs.h"
#include "Geometry.h"


namespace GS{

    class LineSet: public Geometry{
    struct  SegInfo{
        int Id0;
        int Id1;
        int ColorIdx;
    };
public:
     LineSet(bool bUseUniqueVertex = true);
     virtual ~LineSet();
     virtual int PrimitiveCount() const {return mSegs.size();}
     int AddLine(const double3 & s, const double3& t, const float4& color);
     void Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData);

protected:
    int  AddVertex(const double3& v);
    int  AddSegment(int vId1, int vId2, const float4& color);
    void FillD3DBuffer(ID3D11DeviceContext* dc,D3D11Buffer* mpVB, int VBOffset, 
                                        D3D11Buffer* mpIB, int IBOffet);
    void CheckPrimitiveCapacity();
    void CheckPointCapacity();
    int  GetVertexStride() const ;
private:
    ListOf3DPoints mPoints;
    std::vector<SegInfo> mSegs;
    std::vector<float4>  mSegColorTable;
    std::hash_map<Point3D, int, Vec3HashCompare<double> > mVertexDict;
    bool         mbUniqueVertex;
    D3D11Buffer       * mpVB;
    D3D11Buffer       * mpIB;
    int             mPrimitiveCapacity;
    int             mPointsCapacity;
};


}
#endif 