/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: PhongShadingVS.hlsl
Purpose: Phong shading vertex shader
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 11
Operating System requirement: Windows
Project: allie.hammond_CS300_2
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/24/2018
End Header --------------------------------------------------------*/

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix worldTransMatrix;
    matrix MVPMatrix;
    matrix Rotation;
    float4 globalAmbient;
    float4 lightDir[16]; //Light direction (for directional/spotlights)
    float4 Ia[16]; //Light ambient intensity (light color)
    float4 lightPos[16]; // Light position
    float4 theta[16]; //For spotlights
    float4 phi[16]; // For spotlights
    int4 numLights; //Number of lights
    int4 lightType[16]; //Type of light 0 - dir 1 - point 2 - spotlight
    float4 camPos;
    float4 Ka;
    float4 Ns;
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
    float3 worldPos : POSITION;
};


VS_OUTPUT VSMain(VS_INPUT Input)
{
    VS_OUTPUT Output;

    Output.vPosition = mul(float4(Input.vPosition, 1.0f), MVPMatrix);
    Output.worldPos = mul(float4(Input.vPosition, 1.0f), worldTransMatrix).xyz;
    Output.vNormal = mul(float4(Input.vNormal, 1.0f), Rotation);
    Output.vColor = Input.vColor.xyz;

    return Output;
}
