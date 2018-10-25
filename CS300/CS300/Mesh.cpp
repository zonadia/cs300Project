/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Mesh.cpp
Purpose: Load and draw meshes
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0 
Hardware must support DirectX 10 or 11
Operating System requirement: Windows
Project: allie.hammond_CS300_1
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/12/2018
End Header --------------------------------------------------------*/

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <DirectXMath.h>
#include <algorithm>

#include "shader.h"
#include "Mesh.h"

namespace WinData
{
    extern int windowWidth;
    extern int windowHeight;
};

namespace ImGuiData
{
    extern float clearCol[4];
    extern int numLights;
    extern float lightColor[16][3];
    extern int lightType[16];
    extern float lightDirection[16][3];
    extern float lightPos[16][3];
    extern float theta[16];
    extern float phi[16];
    extern float cameraZoom;
    extern float Ka;
    extern float globalAmbient[3];
};

Mesh::Mesh(ID3D11VertexShader *verShader, ID3D11PixelShader *pixShader, ID3D11InputLayout *iLayout)
    : vShader(verShader), pShader(pixShader), inLayout(iLayout), vertexCount(0), faceCount(0),
      scaleX(1.0f), scaleY(1.0f), scaleZ(1.0f), transX(0.0f), transY(0.0f), transZ(0.0f),
      rotX(0.0f), rotY(0.0f), rotZ(0.0f), r(0.612f), g(0.0f), b(1.0f)
{ }

//Cleanup buffers
Mesh::~Mesh()
{
    vertexBuffer->Release();
    indexBuffer->Release();
    delete[] meshVertices;
}

void Mesh::loadMesh(std::string meshName, ID3D11Device *device, ID3D11DeviceContext *context)
{
    //Clear the vertex and index buffer if they already exist
    if(vertexCount > 0)
    {
        vertexBuffer->Release();
        indexBuffer->Release();
        delete[] meshVertices;
    }

    vertexCount = faceCount = 0;
    //Load mesh from obj file
    std::vector<DefaultVertex> vertices;
    std::vector<unsigned int> indices;

    //Load the model from the file
    std::ifstream modelFile;
    std::string modelName = "models/";
    modelName.append(meshName);
    modelFile.open(modelName, std::ifstream::binary);
    std::cout << "Loading model from " << modelName << std::endl;
    char *buffer;
    int length = 0;

    if(modelFile)
    {
        //Get length of file
        modelFile.seekg(0, modelFile.end);
        length = static_cast<int>(modelFile.tellg());
        modelFile.seekg(0, modelFile.beg);

        //Read data into buffer
        buffer = new char[length]{0};
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
            DefaultVertex newVert = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b};

            //Skip space at the front
            ++it;
            //Make string out of char line
            int subIt = 0;
            while(it + subIt + 1 < length && buffer[it + ++subIt] != '\n');
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

            it += subIt;
            continue;
        }
        //If line starts with a f, load face
        if (buffer[it] == 'f')
        {
            ++it;
            //Skip space at the front
            ++it;
            //Make string out of char line
            int subIt = 0;
            while (it + subIt + 1 < length && buffer[it + ++subIt] != '\n')
            {
                char test = buffer[it + subIt];
                test = test;
            }
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

            it += subIt;
            continue;
        }
        ++it;
    }

    //Update mesh transform values based on max and min values
    scaleX = 2.0f / (maxX - minX);
    scaleY = 2.0f / (maxY - minY);
    scaleZ = 2.0f / (maxZ - minZ);

    //Set all scales equal to the greatest scale
    scaleX = scaleY = scaleZ = (std::max)((std::max)(scaleX, scaleY), scaleZ);

    transX = -((maxX + minX) / 2.0f) * scaleX;
    transY = -((maxY + minY) / 2.0f) * scaleY;
    transZ = -((maxZ + minZ) / 2.0f) * scaleZ;

    //Delete file buffer
    delete[] buffer;

    struct Normal
    {
        float x, y, z;
    };

    //Process all the face normals
    std::vector<Normal> normals(faceCount);
    //Vector of face index normals for vertices
    std::vector<std::vector<Normal>> adjacents(vertexCount);

    for(int i = 0;i < faceCount; ++i)
    {
        DefaultVertex &v1 = vertices[indices[i * 3]];
        DefaultVertex &v2 = vertices[indices[i * 3 + 1]];
        DefaultVertex &v3 = vertices[indices[i * 3 + 2]];

        //Calculate vectors U and V
        Normal vecU, vecV;
        vecU.x = v2.x - v1.x;
        vecU.y = v2.y - v1.y;
        vecU.z = v2.z - v1.z;
        vecV.x = v3.x - v1.x;
        vecV.y = v3.y - v1.y;
        vecV.z = v3.z - v1.z;

        //Calculate normals with dotproducts
        normals[i].x = vecU.y * vecV.z - vecU.z * vecV.y;
        normals[i].y = vecU.z * vecV.x - vecU.x * vecV.z;
        normals[i].z = vecU.x * vecV.y - vecU.y * vecV.x;
        //Normalize the normals
        float mag = sqrt(normals[i].x * normals[i].x + normals[i].y * normals[i].y + normals[i].z * normals[i].z);

        normals[i].x /= mag;
        normals[i].y /= mag;
        normals[i].z /= mag;
        //Add this face to all adjacent vertices in the adjaceny list
        adjacents[indices[i * 3]].push_back(normals[i]);
        adjacents[indices[i * 3 + 1]].push_back(normals[i]);
        adjacents[indices[i * 3 + 2]].push_back(normals[i]);
    }

    //Use adjaceny list to create vertex normals
    
    for(int i = 0;i < adjacents.size(); ++i)
    {
        //First remove duplicate face normals from each vertex
        for(int j = 0;j < adjacents[i].size(); ++j)
        {
            for(int k = j + 1;k < adjacents[i].size(); ++k)
            {
                if(adjacents[i][j].x == adjacents[i][k].x && 
                    adjacents[i][j].y == adjacents[i][k].y &&
                    adjacents[i][j].z == adjacents[i][k].z)
                {
                    //Normal is a duplicate
                    adjacents[i].erase(adjacents[i].begin() + k);
                    j = 0;
                    break;
                }
            }
        }

        //Average up the remaining normals to get the resulting normal
        for(int j = 0;j < adjacents[i].size(); ++j)
        {
            vertices[i].nx += adjacents[i][j].x;
            vertices[i].ny += adjacents[i][j].y;
            vertices[i].nz += adjacents[i][j].z;
        }
        vertices[i].nx /= adjacents[i].size();
        vertices[i].ny /= adjacents[i].size();
        vertices[i].nz /= adjacents[i].size();
    }
    
    //Copy all the vertices
    meshVertices = new DefaultVertex[vertexCount];
    for(int i = 0;i < vertexCount; ++i)
    {
        meshVertices[i] = vertices[i];
    }
    

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
        XMMATRIX Rotation;
        XMFLOAT4 globalAmbient;
        XMFLOAT4 lightDir[16]; //Light direction (for directional/spotlights)
        XMFLOAT4 Ia[16]; //Light ambient intensity (light color)
        XMFLOAT4 lightPos[16]; // Light position
        XMFLOAT4 theta[16]; //For spotlights
        XMFLOAT4 phi[16]; // For spotlights
        XMINT4 numLights; //Number of lights
        XMINT4 lightType[16]; //Type of light 0 - dir 1 - point 2 - spotlight
        float xx[16];
        XMFLOAT4 Ka; // Object ambient intensity
    };

    //Set the MVP matrix
    XMFLOAT3 eye(0.0f, (1.3f / 3.0f) * ImGuiData::cameraZoom, ImGuiData::cameraZoom);
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
    cBuf.Rotation = rot;
    cBuf.numLights = XMINT4(ImGuiData::numLights, 0, 0, 0);
    cBuf.globalAmbient = XMFLOAT4(ImGuiData::globalAmbient[0], ImGuiData::globalAmbient[1], ImGuiData::globalAmbient[2], 1.0f);
    cBuf.Ka = XMFLOAT4(ImGuiData::Ka, 0.0f, 0.0f, 0.0f);

    for(int i = 0;i < 16; ++i)
    {
        //Fill in constant buffer for light information
        cBuf.lightDir[i] = XMFLOAT4(-6.0f, 2.0f, 3.0f, 1.0f);
        XMVECTOR normL = XMVector3Normalize(XMLoadFloat4(&cBuf.lightDir[i]));
        XMStoreFloat4(&cBuf.lightDir[i], normL);
        cBuf.Ia[i] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        cBuf.lightPos[i] = XMFLOAT4(ImGuiData::lightPos[i][0], ImGuiData::lightPos[i][1], ImGuiData::lightPos[i][2], 1.0f);
        cBuf.theta[i] = XMFLOAT4(ImGuiData::theta[i], 0.0f, 0.0f, 0.0f);
        cBuf.phi[i] = XMFLOAT4(ImGuiData::phi[i], 0.0f, 0.0f, 0.0f);
    }

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
        auto reason = device->GetDeviceRemovedReason();
        std::cout << "aaaaaaaaaaa\n";
    }

    // Set the buffer.
    context->VSSetConstantBuffers(0, 1, &constBuffer);
    context->PSSetConstantBuffers(0, 1, &constBuffer);

    //Update the color in the vertex buffer
    for(int i = 0;i < vertexCount; ++i)
    {
        meshVertices[i].r = r;
        meshVertices[i].g = g;
        meshVertices[i].b = b;
    }

    D3D11_MAPPED_SUBRESOURCE ms;
    context->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, meshVertices, sizeof(DefaultVertex) * vertexCount);
    context->Unmap(vertexBuffer, NULL);


    //Set up active things the GPU needs to know
    UINT stride = sizeof(DefaultVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    //Also set the input layout
    context->IASetInputLayout(inLayout);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    context->DrawIndexed(static_cast<unsigned int>(faceCount * 3), 0, 0);

    constBuffer->Release();
}
