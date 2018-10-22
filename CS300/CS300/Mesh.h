/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Mesh.h
Purpose: Load and draw meshes
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 10 or 11
Operating System requirement: Windows
Project: allie.hammond_CS300_1
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/12/2018
End Header --------------------------------------------------------*/

#pragma once

class Mesh
{
public:
    Mesh(ID3D11VertexShader *verShader, ID3D11PixelShader *pixShader, ID3D11InputLayout *iLayout);
    ~Mesh();
    virtual void loadMesh(std::string meshName, ID3D11Device *device, ID3D11DeviceContext *context);
    virtual void drawMesh(ID3D11Device *device, ID3D11DeviceContext *context);



    ID3D11VertexShader *vShader;
    ID3D11PixelShader *pShader;
    ID3D11InputLayout *inLayout;

    ID3D11Buffer *vertexBuffer;
    ID3D11Buffer *indexBuffer;

    int vertexCount, faceCount;

    float scaleX, scaleY, scaleZ;
    float transX, transY, transZ;
    float rotX, rotY, rotZ;
    float r, g, b;
};
