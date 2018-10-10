#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <string>
#include <iostream>
#include <vector>
#include <DirectXMath.h>

#include "shader.h"
#include "Mesh.h"

namespace WinData
{
    extern int windowWidth;
    extern int windowHeight;
};

Mesh::Mesh(ID3D11VertexShader *verShader, ID3D11PixelShader *pixShader, ID3D11InputLayout *iLayout)
    : vShader(verShader), pShader(pixShader), inLayout(iLayout), vertexCount(0), faceCount(0)
{ }

//Cleanup buffers
Mesh::~Mesh()
{
    vertexBuffer->Release();
    indexBuffer->Release();
}

void Mesh::loadMesh(std::string meshName, ID3D11Device *device, ID3D11DeviceContext *context)
{
    vertexCount = faceCount = 0;
    //Load mesh from obj file
    std::vector<DefaultVertex> vertices;
    std::vector<unsigned int> indices;

    DefaultVertex TriVertices[] = { {-0.50f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f},
                                   { 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
                                   { 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f } };

    vertices.push_back(TriVertices[0]);
    vertices.push_back(TriVertices[1]);
    vertices.push_back(TriVertices[2]);
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    vertexCount = 3;
    faceCount = 1;


    //Setup vertex buffer
    //Create buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(DefaultVertex) * vertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateBuffer(&bd, NULL, &vertexBuffer);
    if (FAILED(hr)) {
        std::cout << "Failed to create vertex buffer!\n" << std::endl;
        return;
    }
    //Fill the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    context->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, vertices.data(), sizeof(DefaultVertex) * vertexCount);
    context->Unmap(vertexBuffer, NULL);

    //Setup index buffer
    //Create buffer
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = static_cast<unsigned int>(sizeof(unsigned int) * faceCount * 3);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = sizeof(unsigned int);

    // Define the resource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = indices.data();
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&bd, &InitData, &indexBuffer);
    if(FAILED(hr))
    {
        std::cout << "Could not create index buffer!\n";
    }
}

void Mesh::drawMesh(ID3D11Device *device, ID3D11DeviceContext *context)
{
    //For DirectXMath
    using namespace DirectX;

    //Set shaders as active
    context->VSSetShader(vShader, 0, 0);
    context->PSSetShader(pShader, 0, 0);

    //Do the actual rendering

    struct VS_CONSTANT_BUFFER
    {
        XMMATRIX MVPMatrix;
    };

    //Set the MVP matrix
    XMFLOAT3 eye(0.0f, 0.0f, 10.0f);
    XMFLOAT3 focusPosition(0.0f, 0.0f, 0.0f);
    XMFLOAT3 upVector(0.0f, 1.0f, 0.0f);


    XMMATRIX view = XMMatrixLookAtRH(XMLoadFloat3(&eye), XMLoadFloat3(&focusPosition), XMLoadFloat3(&upVector));
    view = XMMatrixTranspose(view);
    XMMATRIX projection = XMMatrixPerspectiveRH(3.14f / 4.0f, (3.14f / 4.0f) * (WinData::windowWidth / WinData::windowHeight), 1.0f, 100.0f);
    projection = XMMatrixTranspose(projection);

    //Create constant buffer for MVP Matrix
    ID3D11Buffer *constBuffer;
    VS_CONSTANT_BUFFER cBuf;
    cBuf.MVPMatrix = XMMatrixMultiply(projection, view);

    // Fill in a buffer description.
    D3D11_BUFFER_DESC cbDesc;
    cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &cBuf;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    // Create the buffer.
    HRESULT hr = device->CreateBuffer(&cbDesc, &InitData, &constBuffer);

    if (FAILED(hr))
    {
        std::cout << "Failed to create constant buffer\n";
    }

    // Set the buffer.
    context->VSSetConstantBuffers(0, 1, &constBuffer);


    //Set up active things the GPU needs to know
    UINT stride = sizeof(DefaultVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    //Also set the input layout
    context->IASetInputLayout(inLayout);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    context->DrawIndexed(static_cast<unsigned int>(faceCount * 3), 0, 0);
}
