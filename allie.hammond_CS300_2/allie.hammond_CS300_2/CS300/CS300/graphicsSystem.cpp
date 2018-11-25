/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: graphicsSystem.cpp
Purpose: Init and run graphics main loop
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 11
Operating System requirement: Windows
Project: allie.hammond_CS300_2
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/12/2018
End Header --------------------------------------------------------*/

#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <vector>
#include <wrl.h>

#include "graphicsSystem.h"
#include "manageImGui.h"
#include "shader.h"
#include "Mesh.h"
#include "DirectXTex.h"
#include "Texture.h"

#pragma comment(lib,"d3d11.lib")

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
    extern int shader;
}

namespace DXData
{
    ID3D11Texture2D *backBuffer;
    ID3D11Texture2D *depthStencil;
    D3D11_TEXTURE2D_DESC bbDesc;

    Microsoft::WRL::ComPtr<ID3D11Device> DXdevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> DXcontext;
    Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;

    //Used for swap chain creation
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory> factory;

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

    D3D11_VIEWPORT viewport;

    extern shaderProgram mainShaderProgram;
    extern shaderProgram phongLighting;
    extern shaderProgram phongShading;
    extern shaderProgram blinnShading;
    extern shaderProgram planeTexture;
}

namespace WinData
{
    extern HWND windowHandle;
}


void createRasterizerState()
{
    D3D11_RASTERIZER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.AntialiasedLineEnable = false;
    desc.CullMode = D3D11_CULL_BACK;
    desc.DepthBias = 0;
    desc.DepthBiasClamp = 0.0f;
    desc.DepthClipEnable = true;
    desc.FillMode = D3D11_FILL_SOLID;
    desc.FrontCounterClockwise = true; // RH Coordinate System
    desc.MultisampleEnable = false;
    desc.ScissorEnable = false;
    desc.SlopeScaledDepthBias = 0.0f;

    HRESULT hr = DXData::DXdevice->CreateRasterizerState(&desc, &DXData::rasterizerState);
    if(FAILED(hr))std::cout << "Failed to create rasterizer state! " << GetLastError() << std::endl;

    DXData::DXcontext->RSSetState(DXData::rasterizerState.Get());
}

void createDepthStencilState()
{
    D3D11_DEPTH_STENCIL_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.DepthEnable = true;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS;

    desc.StencilEnable = true;
    desc.StencilReadMask = 0xFF;
    desc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    HRESULT hr = DXData::DXdevice->CreateDepthStencilState(&desc, &DXData::depthStencilState);
    if(FAILED(hr))std::cout << "Failed to create depth stencil state!\n";

    DXData::DXcontext->OMSetDepthStencilState(DXData::depthStencilState.Get(), 1);
}

void initDirectX()
{
    /*
    D3D_FEATURE_LEVEL levels[] = {
    D3D_FEATURE_LEVEL_9_1,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_11_1
    };*/

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1
    };

    // This flag adds support for surfaces with a color-channel ordering different
    // from the API default. It is required for compatibility with Direct2D.
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;


    // Create the Direct3D 11 API device object and a corresponding context.
    //Microsoft::WRL::ComPtr<ID3D11Device>        device;
    //Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
        0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        deviceFlags,                // Set debug and Direct2D compatibility flags.
        levels,                     // List of feature levels this app can support.
        ARRAYSIZE(levels),          // Size of the list above.
        D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
        &DXData::DXdevice,          // Returns the Direct3D device created.
        NULL,                       // Returns feature level of device created.
        &DXData::DXcontext          // Returns the device immediate context.
    );

    if (FAILED(hr))
    {
        // Handle device interface creation failure if it occurs.
        // For example, reduce the feature level requirement, or fail over 
        // to WARP rendering.
        std::cout << "Failed to create Direct3D Device! :'(\n";
    }

    //Create device and swap chain
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.Windowed = TRUE; // Sets the initial state of full-screen mode to not full screen
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;      //multisampling setting
    desc.SampleDesc.Quality = 0;    //vendor-specific flag
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.OutputWindow = WinData::windowHandle;

    // Create the DXGI device object to use in other factories, such as Direct2D.
    DXData::DXdevice.As(&DXData::dxgiDevice);

    // Create swap chain.
    hr = DXData::dxgiDevice->GetAdapter(&DXData::adapter);

    if (SUCCEEDED(hr))
    {
        DXData::adapter->GetParent(IID_PPV_ARGS(&DXData::factory));

        hr = DXData::factory->CreateSwapChain(DXData::DXdevice.Get(), &desc, &DXData::swapChain);
    }

    //Create a render target
    hr = DXData::swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&DXData::backBuffer);

    hr = DXData::DXdevice->CreateRenderTargetView(DXData::backBuffer, nullptr, DXData::renderTargetView.GetAddressOf());

    DXData::backBuffer->GetDesc(&DXData::bbDesc);

    //Create a depth stencil
    createDepthStencilState();
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        static_cast<UINT> (DXData::bbDesc.Width),
        static_cast<UINT> (DXData::bbDesc.Height),
        1, // This depth stencil view has only one texture.
        1, // Use a single mipmap level.
        D3D11_BIND_DEPTH_STENCIL
    );

    hr = DXData::DXdevice->CreateTexture2D(
        &depthStencilDesc,
        nullptr,
        &DXData::depthStencil);
    if (FAILED(hr)) {
        std::cout << GetLastError() << std::endl;
        return;
    }
    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

    hr = DXData::DXdevice->CreateDepthStencilView(
            DXData::depthStencil,
            &depthStencilViewDesc,
            &DXData::depthStencilView);
    if (FAILED(hr)) {
        std::cout << GetLastError() << std::endl;
        return;
    }
    createRasterizerState();

    //Create a viewport
    ZeroMemory(&DXData::viewport, sizeof(D3D11_VIEWPORT));
    DXData::viewport.Height = (float)DXData::bbDesc.Height;
    DXData::viewport.Width = (float)DXData::bbDesc.Width;
    DXData::viewport.MinDepth = 0;
    DXData::viewport.MaxDepth = 1;

    DXData::DXcontext->RSSetViewports(1, &DXData::viewport);
    
    loadShaders();
}

void renderTriangle()
{
    //Set shaders as active
    DXData::DXcontext->VSSetShader(DXData::mainShaderProgram.vertexShader.Get(), 0, 0);
    DXData::DXcontext->PSSetShader(DXData::mainShaderProgram.pixelShader.Get(), 0, 0);
    


    //Set up vertex buffers
    /*DefaultVertex TriVertices[] = {{0.0f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f}, 
                                   { -0.45f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f },
                                   { -0.45f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f }};*/

    DefaultVertex TriVertices[] = { {-0.50f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f},
                                   { 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
                                   { 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f } };

    //Put buffer into mesh later
    //Create buffer
    ID3D11Buffer *vertBuffer;
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(DefaultVertex) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = DXData::DXdevice->CreateBuffer(&bd, NULL, &vertBuffer);
    if (FAILED(hr)) {
        std::cout << GetLastError() << std::endl;
        return;
    }
    //Fill the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    DXData::DXcontext->Map(vertBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, TriVertices, sizeof(TriVertices));
    DXData::DXcontext->Unmap(vertBuffer, NULL);



    //Do the actual rendering
    //Set up active things the GPU needs to know
    UINT stride = sizeof(DefaultVertex);
    UINT offset = 0;
    DXData::DXcontext->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
    //Also set the input layout
    DXData::DXcontext->IASetInputLayout(DXData::mainShaderProgram.vsLayout);

    DXData::DXcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DXData::DXcontext->Draw(3, 0);

    vertBuffer->Release();
}

void graphicsMainLoop(std::string modelName)
{
    initDirectX();
    initImGui(DXData::DXdevice.Get(), DXData::DXcontext.Get());

    //Load textures for skybox
    Texture skyboxFront("textures/badomen_ft.tga");

    //Load test model
    Mesh mainModel(DXData::blinnShading.vertexShader.Get(), DXData::blinnShading.pixelShader.Get(), DXData::blinnShading.vsLayout);
    mainModel.loadMesh(modelName, DXData::DXdevice.Get(), DXData::DXcontext.Get());

    std::vector<Mesh *> spheres(16);

    //Load 16 orbs around in a circle
    for(int i = 0;i < 16; ++i)
    {
        Mesh *orbModel = new Mesh(DXData::mainShaderProgram.vertexShader.Get(), DXData::mainShaderProgram.pixelShader.Get(), DXData::mainShaderProgram.vsLayout);
        orbModel->r = orbModel->g = 0.5f;
        orbModel->loadMesh("sphere_mid_poly.obj", DXData::DXdevice.Get(), DXData::DXcontext.Get());
        spheres[i] = orbModel;
    }

    //Load plane
    Mesh *planeMesh = new Mesh(DXData::blinnShading.vertexShader.Get(), DXData::blinnShading.pixelShader.Get(), DXData::blinnShading.vsLayout);
    planeMesh->loadMesh("plane_low_poly.obj", DXData::DXdevice.Get(), DXData::DXcontext.Get());
    planeMesh->transY = -5.0f;
    planeMesh->r = planeMesh->g = planeMesh->b = 1.0f;
    planeMesh->scaleX = 0.5f;
    planeMesh->scaleZ = 0.5f;

    bool bGotMsg;
    MSG  msg;
    msg.message = WM_NULL;
    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

    float sphereTheta = 0.0f;

    //Create sampler state
    D3D11_SAMPLER_DESC samplerDesc;
    ID3D11SamplerState *samplerState;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MinLOD = -FLT_MAX;
    samplerDesc.MaxLOD = FLT_MAX;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;


    HRESULT hr = DXData::DXdevice->CreateSamplerState(&samplerDesc, &samplerState);
    if(FAILED(hr))
    {
        std::cout << "Failed to create sampler state!\n";
        return;
    }

    while (WM_QUIT != msg.message)
    {
        // Process window events.
        // Use PeekMessage() so we can use idle time to render the scene. 
        bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);

        if (bGotMsg)
        {
            // Translate and dispatch the message
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            DXData::DXcontext->OMSetRenderTargets(1, DXData::renderTargetView.GetAddressOf(), DXData::depthStencilView.Get());

            // Update the scene.
            //renderer->Update();

            // Render frames during idle time (when no messages are waiting).
            //Clear the back buffer
            
            DXData::DXcontext->ClearRenderTargetView(DXData::renderTargetView.Get(), ImGuiData::clearCol);
            DXData::DXcontext->ClearDepthStencilView(DXData::depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            //Update Mesh
            mainModel.rotY -= 0.0002f;
            mainModel.rotZ += 0.00011f;

            sphereTheta += 0.00023f;

            //Set the active shader
            if(ImGuiData::shader == 0)//Phong lighting
            {
                /*
                planeMesh->pShader = DXData::phongLighting.pixelShader.Get();
                planeMesh->vShader = DXData::phongLighting.vertexShader.Get();
                planeMesh->inLayout = DXData::phongLighting.vsLayout;
                */
                mainModel.pShader = DXData::phongLighting.pixelShader.Get();
                mainModel.vShader = DXData::phongLighting.vertexShader.Get();
                mainModel.inLayout = DXData::phongLighting.vsLayout;
            }
            else
            if(ImGuiData::shader == 1)//Phong shading
            {
                /*
                planeMesh->pShader = DXData::phongShading.pixelShader.Get();
                planeMesh->vShader = DXData::phongShading.vertexShader.Get();
                planeMesh->inLayout = DXData::phongShading.vsLayout;
                */

                mainModel.pShader = DXData::phongShading.pixelShader.Get();
                mainModel.vShader = DXData::phongShading.vertexShader.Get();
                mainModel.inLayout = DXData::phongShading.vsLayout;
            }
            else
            if(ImGuiData::shader == 2)//Blinn shading
            {
                /*
                planeMesh->pShader = DXData::blinnShading.pixelShader.Get();
                planeMesh->vShader = DXData::blinnShading.vertexShader.Get();
                planeMesh->inLayout = DXData::blinnShading.vsLayout;
                */

                mainModel.pShader = DXData::blinnShading.pixelShader.Get();
                mainModel.vShader = DXData::blinnShading.vertexShader.Get();
                mainModel.inLayout = DXData::blinnShading.vsLayout;
            }

            //Draw mesh
            mainModel.drawMesh(DXData::DXdevice.Get(), DXData::DXcontext.Get());
            //Draw plane

            //Set plane shader to be texture shader
            planeMesh->pShader = DXData::planeTexture.pixelShader.Get();
            planeMesh->vShader = DXData::planeTexture.vertexShader.Get();
            planeMesh->inLayout = DXData::planeTexture.vsLayout;

            //Set sampler and texture
            DXData::DXcontext->PSSetSamplers(0, 1, &samplerState);
            DXData::DXcontext->PSSetShaderResources(0, 1, &skyboxFront.shaderResourceView);

            planeMesh->drawMesh(DXData::DXdevice.Get(), DXData::DXcontext.Get());
            //planeMesh->drawMesh(DXData::DXdevice.Get(), DXData::DXcontext.Get());

            //Draw spheres
            for(int i = 0;i < ImGuiData::numLights; ++i)
            {
                //Update spheres
                spheres[i]->transX = 6.0f * sin(2 * 3.14159265f * (i / (float)ImGuiData::numLights) + sphereTheta);
                spheres[i]->transZ = 6.0f * cos(2 * 3.14159265f * (i / (float)ImGuiData::numLights) + sphereTheta);

                //Update sphere colors
                spheres[i]->r = ImGuiData::lightColor[i][0];
                spheres[i]->g = ImGuiData::lightColor[i][1];
                spheres[i]->b = ImGuiData::lightColor[i][2];

                //Update sphere position to be passed to GPU
                ImGuiData::lightPos[i][0] = spheres[i]->transX;
                ImGuiData::lightPos[i][1] = spheres[i]->transY;
                ImGuiData::lightPos[i][2] = spheres[i]->transZ;

                spheres[i]->drawMesh(DXData::DXdevice.Get(), DXData::DXcontext.Get());
            }

            renderImGuiFrame();


            // Present the frame to the screen.
            HRESULT hr = DXData::swapChain->Present(0, 0);
        }
    }

    //Cleanup spheres
    for(int i = 0;i < 16; ++i)
    {
        delete spheres[i];
    }

    delete planeMesh;
    cleanupImGui();
    cleanupDirectX();
}

void cleanupDirectX()
{
    //Release shaders
    DXData::mainShaderProgram.pixelShader.ReleaseAndGetAddressOf();
    DXData::mainShaderProgram.vertexShader.ReleaseAndGetAddressOf();
    //Relase other stuff
    DXData::DXdevice.ReleaseAndGetAddressOf();
    DXData::DXcontext.ReleaseAndGetAddressOf();
    DXData::dxgiDevice.ReleaseAndGetAddressOf();
    DXData::adapter.ReleaseAndGetAddressOf();
    DXData::factory.ReleaseAndGetAddressOf();
    DXData::swapChain.ReleaseAndGetAddressOf();
    DXData::renderTargetView.ReleaseAndGetAddressOf();
    DXData::depthStencilView.ReleaseAndGetAddressOf();
    DXData::depthStencilState.ReleaseAndGetAddressOf();
    DXData::rasterizerState.ReleaseAndGetAddressOf();
}
