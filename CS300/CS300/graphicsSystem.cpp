#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>

#include "graphicsSystem.h"
#include "shader.h"

#pragma comment(lib,"d3d11.lib")

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

    D3D11_VIEWPORT viewport;
}

namespace WinData
{
    extern HWND windowHandle;
}

void initDirectX()
{
    D3D_FEATURE_LEVEL levels[] = {
    D3D_FEATURE_LEVEL_9_1,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_11_1
    };

    // This flag adds support for surfaces with a color-channel ordering different
    // from the API default. It is required for compatibility with Direct2D.
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;


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
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        static_cast<UINT> (DXData::bbDesc.Width),
        static_cast<UINT> (DXData::bbDesc.Height),
        1, // This depth stencil view has only one texture.
        1, // Use a single mipmap level.
        D3D11_BIND_DEPTH_STENCIL
    );

    DXData::DXdevice->CreateTexture2D(
        &depthStencilDesc,
        nullptr,
        &DXData::depthStencil
    );

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

    DXData::DXdevice->CreateDepthStencilView(
        DXData::depthStencil,
        &depthStencilViewDesc,
        &DXData::depthStencilView
    );

    //Create a viewport
    ZeroMemory(&DXData::viewport, sizeof(D3D11_VIEWPORT));
    DXData::viewport.Height = (float)DXData::bbDesc.Height;
    DXData::viewport.Width = (float)DXData::bbDesc.Width;
    DXData::viewport.MinDepth = 0;
    DXData::viewport.MaxDepth = 1;

    DXData::DXcontext->RSSetViewports(1, &DXData::viewport);
    
    loadShaders();
}

void graphicsMainLoop()
{
    initDirectX();

    bool bGotMsg;
    MSG  msg;
    msg.message = WM_NULL;
    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

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
            // Update the scene.
            //renderer->Update();

            // Render frames during idle time (when no messages are waiting).
            //renderer->Render();

            // Present the frame to the screen.
            //deviceResources->Present();
        }
    }
}
