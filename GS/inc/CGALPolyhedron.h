#ifndef _CGAL_POLYHEDRON_H
#define _CGAL_POLYHEDRON_H
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include "typedefs.h"
#include "D3D11Buffer.h"
namespace GS{

// A vertex type with a color member variable.
template <class Refs, class Point>
struct  VertexType
    : public CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, Point>
{
   float4 color;
   float4 normal;
    VertexType() {} // repeat the required constructors
    VertexType( const Point& p)
        : CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, Point>(p) {}
};

// An items type using my vertex.
struct ItemsType : public CGAL::Polyhedron_items_3 {
    template <class Refs, class Traits>
    struct Vertex_wrapper {
        typedef typename Traits::Point_3 Point_3;
        typedef VertexType<Refs,Point_3> Vertex;
    };
};

typedef CGAL::Simple_cartesian<double>        Kernel;

class PolyHedron: public  CGAL::Polyhedron_3<Kernel, ItemsType>{

public : 
    void            Render(ID3D11Device* device,ID3D11DeviceContext* dc, bool bForceFillData);
    void            FillD3DBuffer(ID3D11DeviceContext* dc,D3D11Buffer* mpVB, int VBOffset, 
                                        D3D11Buffer* mpIB, int IBOffet);

};

template <class HDS>
class PolyhedronBuilder :public CGAL::Modifier_base<HDS> {

public: 
    void operator() ( HDS& hds) ;
 

};

}

#endif 