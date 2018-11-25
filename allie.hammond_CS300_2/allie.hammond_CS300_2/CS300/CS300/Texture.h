#pragma once
#include <d3d11.h>

class Texture
{
public:
    Texture(const char *fileName);

    void BindAsTexture();
    void BindAsRenderTarget();
    
    ID3D11Texture2D *texture;
    ID3D11ShaderResourceView *shaderResourceView;
    ID3D11RenderTargetView *renderTargetView;
};
