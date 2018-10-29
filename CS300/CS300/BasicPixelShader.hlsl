/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: BasicPixelShader.hlsl
Purpose: Basic Pixel Shader
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 11
Operating System requirement: Windows
Project: allie.hammond_CS300_2
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/22/2018
End Header --------------------------------------------------------*/

struct PS_INPUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float3 vColor : COLOR;
};

float4 PSMain(PS_INPUT Input) : SV_TARGET
{
    return float4(Input.vColor, 1.0f);
}
