//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
    matrix        g_ModelViewProjection    : packoffset( c0 );
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 vPosition    : POSITION;
    float3 vNormal        : NORMAL;
	float3 vColor : COLOR;
};

struct VS_OUTPUT
{
    float3 vNormal        : NORMAL;
    float4 vPosition    : SV_POSITION;
	float3 vColor : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
    VS_OUTPUT Output;
    
    Output.vPosition = mul( Input.vPosition, g_mWorldViewProjection );
    Output.vNormal = mul( Input.vNormal, (float3x3)g_mWorld );
    Output.vTexcoord = Input.vTexcoord;
	Output.vColor = Input.vColor;
    
    return Output;
}