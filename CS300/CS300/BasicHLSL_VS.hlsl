//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
    matrix g_ModelViewProjection;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vColor : COLOR;
};

struct VS_OUTPUT
{
    float3 vNormal : NORMAL;
    float4 vPosition : SV_POSITION;
    float4 vColor : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
    VS_OUTPUT Output;
    
    Output.vPosition = mul( float4(Input.vPosition, 1.0f), g_ModelViewProjection );
    Output.vNormal = Input.vNormal;
    Output.vColor = float4(Input.vColor.xyz, 1.0f);
    
    return Output;
}