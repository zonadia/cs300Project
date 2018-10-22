/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: BasicHLSL_VS.hlsl
Purpose: Default Vertex Shader
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 10 or 11
Operating System requirement: Windows
Project: allie.hammond_CS300_1
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/12/2018
End Header --------------------------------------------------------*/

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix MVPMatrix;
    matrix Rotation;
    float4 lightDir;
    float4 Ia;
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
    float4 vNormal : NORMAL;
    float3 vColor : COLOR;
};


VS_OUTPUT VSMain( VS_INPUT Input )
{
    VS_OUTPUT Output;
    
    Output.vPosition = mul(float4(Input.vPosition, 1.0f), MVPMatrix);
    //Output.vPosition = mul(MVPMatrix, float4(Input.vPosition, 1.0f));
    //Output.vPosition = float4(Input.vPosition, 1.0f);
    Output.vNormal = mul(float4(Input.vNormal, 1.0f), Rotation);
    Output.vColor = Input.vColor.xyz;
    
    return Output;
}
