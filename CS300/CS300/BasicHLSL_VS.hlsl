cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix MVPMatrix;
};


struct VS_INPUT
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vColor : COLOR;
};

struct VS_OUTPUT
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float4 vColor : COLOR;
};


VS_OUTPUT VSMain( VS_INPUT Input )
{
    VS_OUTPUT Output;
    
    Output.vPosition = mul(float4(Input.vPosition, 1.0f), MVPMatrix);
    //Output.vPosition = mul(MVPMatrix, float4(Input.vPosition, 1.0f));
    //Output.vPosition = float4(Input.vPosition, 1.0f);
    Output.vNormal = Input.vNormal;
    Output.vColor = float4(Input.vColor.xyz, 1.0f);
    
    return Output;
}