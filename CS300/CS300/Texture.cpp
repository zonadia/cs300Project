#include "Texture.h"
#include "DirectXTex.h"

#include <Windows.h>
#include <iostream>
#include <wrl.h>

namespace DXData
{
    extern Microsoft::WRL::ComPtr<ID3D11Device> DXdevice;
}

const wchar_t *GetWC(const char *c)
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);

    return wc;
}

Texture::Texture(const char *fileName)
{
    DirectX::TexMetadata metadata;
    DirectX::ScratchImage image;

    const wchar_t *name = GetWC(fileName);

    HRESULT hr = DirectX::LoadFromTGAFile(name, &metadata, image);
    if (FAILED(hr))
    {
        std::cout << "Unable to load texture! " << fileName << "\n";
        return;
    }
    delete[] name;

    D3D11_TEXTURE2D_DESC desc;
    D3D11_SUBRESOURCE_DATA data;

    desc.Width = metadata.width;
    desc.Height = metadata.height;
    desc.MipLevels = metadata.mipLevels;
    desc.ArraySize = metadata.arraySize;
    desc.Format = metadata.format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    data.pSysMem = image.GetImages()->pixels;
    data.SysMemPitch = image.GetImages()->rowPitch;
    data.SysMemSlicePitch = image.GetImages()->slicePitch;

    hr = DXData::DXdevice->CreateTexture2D(&desc, &data, &texture);
    if(FAILED(hr))
    {
        std::cout << "Failed to create texture : " << fileName << std::endl;
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
    shaderDesc.Format = metadata.format;
    shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderDesc.Texture2D.MipLevels = 1;
    shaderDesc.Texture2D.MostDetailedMip = 0;

    hr = DXData::DXdevice->CreateShaderResourceView(texture, &shaderDesc, &shaderResourceView);
    if (FAILED(hr))
    {
        std::cout << "Failed to create texture(shader resource view) : " << fileName << std::endl;
        return;
    }
}

void Texture::BindAsTexture()
{

}

void Texture::BindAsRenderTarget()
{
    return;
}
