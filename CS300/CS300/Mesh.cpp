#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <string>
#include <iostream>
#include <fstream>
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
    : vShader(verShader), pShader(pixShader), inLayout(iLayout), vertexCount(0), faceCount(0),
      scaleX(1.0f), scaleY(1.0f), scaleZ(1.0f), transX(0.0f), transY(0.0f), transZ(0.0f),
      rotX(0.0f), rotY(0.0f), rotZ(0.0f)
{ }

//Cleanup buffers
Mesh::~Mesh()
{
    vertexBuffer->Release();
    indexBuffer->Release();
}

void Mesh::loadMesh(std::string meshName, ID3D11Device *device, ID3D11DeviceContext *context)
{
    //Clear the vertex and index buffer if they already exist
    if(vertexCount > 0)
    {
        vertexBuffer->Release();
        indexBuffer->Release();
    }

    vertexCount = faceCount = 0;
    //Load mesh from obj file
    std::vector<DefaultVertex> vertices;
    std::vector<unsigned int> indices;

    //Load the model from the file
    std::ifstream modelFile;
    std::string modelName = "models/";
    modelName.append(meshName);
    modelFile.open(modelName);
    std::cout << "Loading model from " << modelName << std::endl;
    char *buffer;
    int length = 0;

    if(modelFile)
    {
        //Get length of file
        modelFile.seekg(0, modelFile.end);
        length = modelFile.tellg();
        modelFile.seekg(0, modelFile.beg);

        //Read data into buffer
        buffer = new char[length];
        modelFile.read(buffer, length);

        //Close the file
        modelFile.close();
    }
    else
    {
        std::cout << "Failed to read model file!\n";
        return;
    }

    //Process file into mesh
    
    //Store maximum and minimum x, y, z positions
    float minX = 0.0f, maxX = 0.0f;
    float minY = 0.0f, maxY = 0.0f;
    float minZ = 0.0f, maxZ = 0.0f;

    int it = 0;
    while(it < length)
    {
        //Skip lines if they don't start with v or f
        if(buffer[it] == '\n')
        {
            ++it;
            continue;
        }
        if(buffer[it] == '#')
        {
            //Skip until end of line
            ++it;
            while(buffer[it++] != '\n' && it < length);
            continue;
        }
        //If line starts with a v, load vertex
        if(buffer[it] == 'v')
        {
            ++it;
            //Read 3 floats into new vertex
            DefaultVertex newVert = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.8f};

            //Skip space at the front
            ++it;
            //Make string out of char line
            int lineLength;
            int subIt = 0;
            while(buffer[it + ++subIt] != '\n');
            std::string line(&(buffer[it]), subIt);
            std::string::size_type sz;
            std::string::size_type sz2;
            float x = std::stof(line, &sz);
            float y = std::stof(line.substr(sz + 1), &sz2);
            float z = std::stof(line.substr(sz + sz2 + 1), &sz);

            //Update max and min values
            if(x < minX)minX = x;
            if(y < minY)minY = y;
            if(z < minZ)minZ = z;

            if(x > maxX)maxX = x;
            if(y > maxY)maxY = y;
            if(z > maxZ)maxZ = z;

            newVert.x = x;
            newVert.y = y;
            newVert.z = z;
            vertices.push_back(newVert);
            vertexCount++;
            continue;
        }
        //If line starts with a f, load face
        if (buffer[it] == 'f')
        {
            ++it;
            //Skip space at the front
            ++it;
            //Make string out of char line
            int lineLength;
            int subIt = 0;
            while (buffer[it + ++subIt] != '\n');
            std::string line(&(buffer[it]), subIt);
            std::string::size_type sz;
            std::string::size_type sz2;
            int f1 = std::stoi(line, &sz);
            int f2 = std::stoi(line.substr(sz + 1), &sz2);
            int f3 = std::stoi(line.substr(sz + sz2 + 1), &sz);
            indices.push_back(static_cast<unsigned int>(f1 - 1));
            indices.push_back(static_cast<unsigned int>(f2 - 1));
            indices.push_back(static_cast<unsigned int>(f3 - 1));
            faceCount++;
            continue;
        }
        ++it;
    }

    //Update mesh transform values based on max and min values
    scaleX = 2.0f / (maxX - minX);
    scaleY = 2.0f / (maxY - minY);
    scaleZ = 2.0f / (maxZ - minZ);

    transX = -((maxX + minX) / 2.0f) * scaleX;
    transY = -((maxY + minY) / 2.0f) * scaleY;
    transZ = -((maxZ + minZ) / 2.0f) * scaleZ;

    //Delete file buffer
    delete[] buffer;


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
    XMFLOAT3 eye(0.0f, 0.0f, 7.0f);
    XMFLOAT3 focusPosition(0.0f, 0.0f, 0.0f);
    XMFLOAT3 upVector(0.0f, 1.0f, 0.0f);

    XMMATRIX trans = XMMatrixTranspose(XMMatrixTranslation(transX, transY, transZ));
    XMMATRIX scale = XMMatrixTranspose(XMMatrixScaling(scaleX, scaleY, scaleZ));
    XMMATRIX rot = XMMatrixTranspose(XMMatrixRotationRollPitchYaw(rotX, rotY, rotZ));

    XMMATRIX model = XMMatrixMultiply(XMMatrixMultiply(rot, trans), scale);

    XMMATRIX view = XMMatrixLookAtRH(XMLoadFloat3(&eye), XMLoadFloat3(&focusPosition), XMLoadFloat3(&upVector));
    view = XMMatrixTranspose(view);
    view = XMMatrixMultiply(view, model);

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
