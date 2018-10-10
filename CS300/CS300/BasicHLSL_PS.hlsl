struct PS_INPUT
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float4 vColor : COLOR;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
    return Input.vColor;
}