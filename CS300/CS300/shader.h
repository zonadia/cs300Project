/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shader.h
Purpose: Create and manage shaders
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 11
Operating System requirement: Windows
Project: allie.hammond_CS300_2
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/12/2018
End Header --------------------------------------------------------*/


#pragma once

#include <wrl.h>

//Forward declarations
struct ID3D11VertexShader;
struct ID3D11PixelShader;

struct DefaultVertex
{
    //Position
    float x, y, z;
    //Normal
    float nx, ny, nz;
    //Color
    float r, g, b;
};

//A combination of a vertex and pixel shader
struct shaderProgram
{
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

    ID3D11InputLayout *vsLayout;
};

HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);

int loadShaders();
