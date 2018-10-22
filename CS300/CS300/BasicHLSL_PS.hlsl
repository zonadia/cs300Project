/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: BasicHLSL_PS.hlsl
Purpose: Default Pixel Shader
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
    float4 lightDir[16];
    float4 Ia[16];
    int numDirLights;
};

struct PS_INPUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float3 vColor : COLOR;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET
{
    float4 Ka = float4(0.05f, 0.05f, 0.05f, 1.0f);
    float4 Id = float4(1.0f, 1.0f, 1.0f, 1.0f);

    float4 ambient = Ia[0] * Ka;
    float4 diffuse = Id * float4(Input.vColor, 1.0f) * dot(Input.vNormal, lightDir[0]);

    return ambient + diffuse;
}
