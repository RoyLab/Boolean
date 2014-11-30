#ifndef _VERTEX_BUFFER_H
#define _VERTEX_BUFFER_H
#include "ID3DResource.h"
namespace GS{

	
      
    enum EVertexField
    {
        /// <description>
        /// position pre-define values
        /// </description>
        PositionVal  = 0,
        /// <description>
        /// normal pre-define values
        /// </description>
        NormalVal    = 3,
        /// <description>
        /// texture pre-define values
        /// </description>
        TextureVal   = 5,
        /// <description>
        /// tangent pre-define values
        /// </description>
        TangentVal   = 6,
        /// <description>
        /// bitangent pre-define values
        /// </description>
        BitangentVal = 7,
        /// <description>
        /// z value used to draw in draw order
        /// </description>
        DrawOrderZVal = 8,
        /// <description>
        /// object id
        /// </description>
        ObjectIDVal = 9,
        /// <description>
        /// color pre-define values
        /// </description>
        ColorDWVal     = 10,
        /// <description>
        /// color pre-define values
        /// </description>
        ColorFVal     = ColorDWVal+1,
        /// <description>
        /// point size scale factor can be attached per vertex
        /// </description>
        PointSizeVal   = 12,
        /// <description>
        /// indices into the array of matrices in the constant buffer, used for skinning
        /// </description>
        BlendIndicesVal   = 13,
       /// <description>
        /// weights applied to the indexed matrix multiplies, used for skinning
        /// </description>
        BlendWeightsVal   = 14,
        /// <description>
        /// unknown pre-define values
        /// </description>
        UnknownVal   = 15
    };
        
    enum EVertexFormat
    {
        /// <description>
        /// no format
        /// </description>
        NoFormat = 0,
        /// <description>
        /// position
        /// </description>
        P = 0x1<<PositionVal, //d3d.VertexFormats.Position,
        /// <description>
        /// normal
        /// </description>
        N = 0x1<<NormalVal, //d3d.VertexFormats.Normal,
        /// <description>
        /// texture
        /// </description>
        TEX = 0x1<<TextureVal, 
        /// <description>
        /// tangent
        /// </description>
        TA = 0x1<<TangentVal,
        /// <description>
        /// bitangent (a.k.a. binormal)
        /// </description>
        BT = 0x1<<BitangentVal,
        /// <description>
        /// z value used to draw in draw order
        /// </description>
        DZ = 0x1<<DrawOrderZVal,
        /// <description>
        /// object id
        /// </description>
        O_ID = 0x1<<ObjectIDVal,
        /// <description>
        /// color 0 channel as a dword rgba (8-bits each)
        /// </description>
        C_DW = 0x1<<ColorDWVal, //d3d.VertexFormats.Color,
        /// <description>
        /// color 0 channel as 4 floats rgba (32-bits each)
        /// </description>
        C_4F = 0x1<<ColorFVal, //d3d.VertexFormats.Color,
        /// <description>
        /// point size
        /// </description>
        PS = 0x1<<PointSizeVal,

        /// <description>
        /// This format uses a custom field type.
        /// </description>
        CustomFormat = 0x1<<UnknownVal
	};

class VertexBuffer: public ID3DBuffer{

public:
	VertexBuffer(EResourceUsage eUsage);
	virtual ~VertexBuffer();
	bool CreateDeviceBuffer(ID3D11Device* device, void* data, int size); 

protected:
 
	int           mnVertexFormat;
	int           mnStride;

};


};


#endif 