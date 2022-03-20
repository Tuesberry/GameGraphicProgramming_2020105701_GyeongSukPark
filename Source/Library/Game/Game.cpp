#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    /*--------------------------------------------------------------------
      TODO: Initialize global variables (remove the comment)
    --------------------------------------------------------------------*/
    #pragma comment( lib, "d3d11.lib" )
    
    HINSTANCE g_hInstance = nullptr;
    HWND g_hWnd = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Device> g_pd3dDevice = nullptr;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_pd3dDeviceContext = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain>g_pDXGISwapChain = nullptr;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>g_pRenderTargetView = nullptr;
    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/

    /*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      Function: WindowProc

      Summary:  Defines the behavior of the window—its appearance, how
                it interacts with the user, and so forth

      Args:     HWND hWnd
                  Handle to the window
                UINT uMsg
                  Message code
                WPARAM wParam
                  Additional data that pertains to the message
                LPARAM lParam
                  Additional data that pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    -----------------------------------------------------------------F-F*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    /*--------------------------------------------------------------------
      TODO: Function definitions (remove this comment)
    --------------------------------------------------------------------*/
    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        // Register the window class
        const wchar_t CLASS_NAME[] = L"KHU_ENGINE_WINCLASS";
        
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = CLASS_NAME;
        wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
        
        if (!RegisterClassEx(&wcex))
        {
            DWORD dwError = GetLastError();
            MessageBox(
                nullptr,
                L"Call to RegisterClassEx failed!",
                L"Game Graphics Programming",
                NULL
            );
            if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(dwError);
            }
            return E_FAIL;
        }

        // Create the window
        g_hInstance = hInstance;
        RECT rc = { 0, 0, 800, 600 };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        g_hWnd = CreateWindowEx(
            0,
            CLASS_NAME,
            L"Game Graphics Programming Lab 01: Direct3D 11 Basics",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top,
            NULL,
            NULL,
            hInstance,
            NULL
        );

        if (!g_hWnd)
        {
            DWORD dwError = GetLastError();
            MessageBox(
                nullptr,
                L"Call to CreateWindow failed!",
                L"Game Graphics Programming",
                NULL
            );
            if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(dwError);
            }
            return E_FAIL;
        }
        
        // show window
        ShowWindow(g_hWnd, nCmdShow);

        return S_OK;
    }

    HRESULT InitDevice()
    {
        RECT rc;
        GetClientRect(g_hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        // Create Direct3D 11 device and context
        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_11_1
        };

        DWORD createDeviceFlags = 0;

        #if defined(DEBUG) || defined(_DEBUG)
            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        Microsoft::WRL::ComPtr<ID3D11Device>        device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL f1;

        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            createDeviceFlags,
            levels,
            ARRAYSIZE(levels),
            D3D11_SDK_VERSION,
            &device,
            &f1,
            &context
        );

        if (FAILED(hr))
        {
            return hr;
        }

        device.As(&g_pd3dDevice);
        context.As(&g_pd3dDeviceContext);

        // Create Swap Chain
        DXGI_SWAP_CHAIN_DESC desc;
        ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
        desc.Windowed = TRUE; // Sets the initial state of full-screen mode.
        desc.BufferCount = 2;
        desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SampleDesc.Count = 1;      //multisampling setting
        desc.SampleDesc.Quality = 0;    //vendor-specific flag
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.OutputWindow = g_hWnd;

        // Create DXGI factory from device
        Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
        g_pd3dDevice.As(&dxgiDevice);

        // Create Swap Chain.
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;

        hr = dxgiDevice->GetAdapter(&adapter);
        if (SUCCEEDED(hr))
        {
            adapter->GetParent(IID_PPV_ARGS(&factory));
            hr = factory->CreateSwapChain(g_pd3dDevice.Get(), &desc, &g_pDXGISwapChain);
        }

        // Create render target view
        Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = g_pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, g_pRenderTargetView.GetAddressOf());
        pBackBuffer.Reset();

        if (FAILED(hr))
        {
            return hr;
        }

        g_pd3dDeviceContext->OMSetRenderTargets(1, g_pRenderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = (float)width;
        vp.Height = (float)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        g_pd3dDeviceContext->RSSetViewports(1, &vp);

        return S_OK;
    }

    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
            if (MessageBox(hWnd, L"Really quit?", L"Game Graphics Programming", MB_OKCANCEL) == IDOK)
            {
                DestroyWindow(hWnd);
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    void CleanupDevice()
    {
        if (g_pd3dDeviceContext)g_pd3dDeviceContext->ClearState();

        if (g_pRenderTargetView)g_pRenderTargetView.Reset();
        if (g_pDXGISwapChain)g_pDXGISwapChain.Reset();
        if (g_pd3dDevice)g_pd3dDevice.Reset();
        if (g_pd3dDeviceContext)g_pd3dDeviceContext.Reset();
    }

    void Render()
    {
        g_pd3dDeviceContext->ClearRenderTargetView(
            g_pRenderTargetView.Get(),
            DirectX::Colors::MidnightBlue
        );
        g_pDXGISwapChain->Present(0, 0);
    }
}