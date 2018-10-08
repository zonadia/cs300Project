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
};
