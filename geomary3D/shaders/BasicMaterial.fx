////////////////////////////////////////////////////////////////////////////////
// Filename: color.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
	float padding;
};


cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor[2];
    float4 specularColor[2];
    float4 lightDirection[2];
	bool   lightActive[2];
	float  specularPower;
	float  padding1;
};
 

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
	float3 viewDirection : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType VS(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;
    
	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
	output.viewDirection = normalize(output.position.xyz);

	// Calculate the normal vector against the world matrix only.
	float4 normal2  = float4(input.normal, 0.0);
	normal2 = mul(normal2, worldMatrix);
	normal2 = mul(normal2, viewMatrix);
	output.normal = normalize(normal2.xyz);
    
    output.position = mul(output.position, projectionMatrix);
	//output.position.w = 1.0f;

	// Store the input color for the pixel shader to use.
    output.color = input.color;
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS(PixelInputType input) : SV_TARGET
{
	////return float4(1, 1, 1, 1);
    float4 color = ambientColor;
	//if (lightActive[0])
	{
		float lightCoef = saturate(-dot(input.normal, lightDirection[0].xyz));
		color += lightCoef * input.color * diffuseColor[0];

	    float3 lightVec = normalize(-lightDirection[0].xyz);

	    float3 halfVec = normalize(input.viewDirection+lightVec);
	    float nx = saturate(dot(input.normal, halfVec)) +0.01;
	    color += pow(nx, 2.0) * specularColor[0];
	}

	//if (lightActive[1])
	//{
	//	float lightCoef = saturate(-dot(input.normal, lightDirection[1]));
	//	color += lightCoef * input.color;  
 //     

	//	
	//}

	color.w = input.color.w;
    return color;
}


