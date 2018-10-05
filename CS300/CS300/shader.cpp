#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <comdef.h>

#include "shader.h"

namespace DXData
{
    extern Microsoft::WRL::ComPtr<ID3D11Device> DXdevice;
    extern Microsoft::WRL::ComPtr<ID3D11DeviceContext> DXcontext;
    extern Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;

    shaderProgram mainShaderProgram;
}

HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
    if (!srcFile || !entryPoint || !profile || !blob)
        return E_INVALIDARG;

    *blob = nullptr;

    const D3D_SHADER_MACRO defines[] =
    {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint, profile,
        0, 0, &shaderBlob, &errorBlob);
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return hr;
    }

    *blob = shaderBlob;

    return hr;
}

//Return -1 on fail
int loadShaders()
{
    // Compile vertex shader
    ID3DBlob *vsBlob = nullptr;
    HRESULT hr = CompileShader(L"BasicHLSL_VS.hlsl", "VSMain", "vs_4_0_level_9_1", &vsBlob);
    if (FAILED(hr))
    {
        std::cout << "Failed compiling vertex shader!" << std::endl;
        return -1;
    }

    std::cout << "Compiled vertex shader!\n";

    D3D11_INPUT_ELEMENT_DESC iaDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
        0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,
        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = DXData::DXdevice->CreateInputLayout(iaDesc, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &DXData::mainShaderProgram.vsLayout);

    if(FAILED(hr))
    {
        std::cout << "Failed to create vertex shader input layout!\n";
        return -1;
    }

    // Compile pixel shader
    ID3DBlob *psBlob = nullptr;
    hr = CompileShader(L"BasicHLSL_PS.hlsl", "PSMain", "ps_4_0_level_9_1", &psBlob);
    if (FAILED(hr))
    {
        vsBlob->Release();
        std::cout << "Failed compiling pixel shader!" << std::endl;
        return -1;
    }

    std::cout << "Compiled pixel shader!\n";


    //Create shaders
    hr = DXData::DXdevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &DXData::mainShaderProgram.vertexShader);
    if(FAILED(hr))
    {
        std::cout << "Failed to create vertex shader!\n";
    }
    hr = DXData::DXdevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &DXData::mainShaderProgram.pixelShader);
    if (FAILED(hr))
    {
        std::cout << "Failed to create pixel shader!\n";
    }

    // Clean up shader blobs
    vsBlob->Release();
    psBlob->Release();



    return 0;
}