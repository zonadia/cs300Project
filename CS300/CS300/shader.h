#pragma once

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
    ID3D11VertexShader *vShader;
    ID3D11PixelShader *pShader;
    
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

    ID3D11InputLayout *vsLayout;
};

HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);

int loadShaders();
