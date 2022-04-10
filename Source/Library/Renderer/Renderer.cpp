#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer
      Summary:  Constructor
      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_vertexShader,
                  m_pixelShader, m_vertexLayout, m_vertexBuffer].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Renderer definition (remove the comment)
    --------------------------------------------------------------------*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_depthStencil(nullptr)
        , m_depthStencilView(nullptr)
        , m_projection(XMMatrixIdentity())
        , m_camera(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f))
        , m_renderables(std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>())
        , m_vertexShaders(std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>())
        , m_pixelShaders(std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>())
    {
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize
      Summary:  Creates Direct3D device and swap chain
      Args:     HWND hWnd
                  Handle to the window
      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader,
                  m_vertexLayout, m_pixelShader, m_vertexBuffer].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Initialize definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        DWORD createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = S_OK;

        // Create Direct3D 11 device and context
        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }


        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
        {
            Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice = nullptr;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                Microsoft::WRL::ComPtr<IDXGIAdapter> adapter = nullptr;
                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory);
                    adapter.Reset();
                }
                dxgiDevice.Reset();
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
        hr = dxgiFactory.As(&dxgiFactory2);

        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);

            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }

            dxgiFactory2.Reset();
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = {
                .BufferDesc = {
                    .Width = width,
                    .Height = height,
                    .RefreshRate = {
                        .Numerator = 60,
                        .Denominator = 1
                    },
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                },
                .SampleDesc = {
                    .Count = 1,
                    .Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        dxgiFactory.Reset();

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);

        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        pBackBuffer.Reset();

        if (FAILED(hr))
        {
            return hr;
        }

        // create depth stencil texture and the depth stencil view
        // create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth = {
            .Width = width,
            .Height = height,
            .MipLevels = 1,     // 1 for a multisampled texture
            .ArraySize = 1,     // Number of textures in the texture array
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {
                .Count = 1,
                .Quality = 0
            },
            .Usage = D3D11_USAGE_DEFAULT,   // A resource that requires read and write access by the GPU
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0
        };

        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {
                .MipSlice = 0
            }
        };

        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // setup the viewport
        D3D11_VIEWPORT vp = {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = (float)width,
            .Height = (float)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // initalize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (float)width / (float)height, 0.01f, 100.0f);
                                              
        // initalize the shaders, then the renderables
        for (auto iVShader = m_vertexShaders.begin(); iVShader != m_vertexShaders.end(); iVShader++)
        {
            hr = iVShader->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto iPShader = m_pixelShaders.begin(); iPShader != m_pixelShaders.end(); iPShader++)
        {
            hr = iPShader->second->Initialize(m_d3dDevice.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto iRenderable = m_renderables.begin(); iRenderable != m_renderables.end(); iRenderable++)
        {
            hr = iRenderable->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable
      Summary:  Add a renderable object and initialize the object
      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Unique pointer to the renderable object
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        // checks if the key already exists in the renderable hash map
        if (m_renderables.find(pszRenderableName) != m_renderables.end())
        {
            // key already exists
            return E_FAIL;
        }

        // add the renderable
        m_renderables.insert(std::make_pair(pszRenderableName, renderable));

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader
      Summary:  Add the vertex shader into the renderer
      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add
      Modifies: [m_vertexShaders].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddVertexShader definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        // checks if the key already exists in the renderable hash map
        if (m_vertexShaders.find(pszVertexShaderName) != m_vertexShaders.end())
        {
            // key already exists
            return E_FAIL;
        }

        // add the vertex shader
        m_vertexShaders.insert(std::make_pair(pszVertexShaderName, vertexShader));

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader
      Summary:  Add the pixel shader into the renderer 
      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add
      Modifies: [m_pixelShaders].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddPixelShader definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        // checks if the key already exists in the renderable hash map
        if (m_pixelShaders.find(pszPixelShaderName) != m_pixelShaders.end())
        {
            // key already exists
            return E_FAIL;
        }

        // add the pixel shader
        m_pixelShaders.insert(std::make_pair(pszPixelShaderName, pixelShader));

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput
      Summary:  Add the pixel shader into the renderer and initialize it
      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data
      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::HandleInput definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update
      Summary:  Update the renderables each frame
      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Update definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        // update renderable
        for (auto iRenderable = m_renderables.begin(); iRenderable != m_renderables.end(); iRenderable++)
        {
            iRenderable->second->Update(deltaTime);
        }

        // update camera
        m_camera.Update(deltaTime);
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render
      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Render definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Render()
    {
        // clear the back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);
        // clear the depth buffer to 1.0 (maximum depth)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        // for each rendererables
        for (auto iRenderable = m_renderables.begin(); iRenderable != m_renderables.end(); iRenderable++)
        {
            // set vertex buffer
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;
            m_immediateContext->IASetVertexBuffers(0u, 1u, iRenderable->second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);

            // set index buffer
            m_immediateContext->IASetIndexBuffer(iRenderable->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            
            // set input layout
            m_immediateContext->IASetInputLayout(iRenderable->second->GetVertexLayout().Get());
        
            // update constant buffer
            ConstantBuffer cb = {
                .World = XMMatrixTranspose(iRenderable->second->GetWorldMatrix()),
                .View = XMMatrixTranspose(m_camera.GetView()),
                .Projection = XMMatrixTranspose(m_projection)
            };
            m_immediateContext->UpdateSubresource(iRenderable->second->GetConstantBuffer().Get(), 0, nullptr, &cb, 0, 0);

            // render the triangles
            m_immediateContext->VSSetShader(iRenderable->second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, iRenderable->second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetShader(iRenderable->second->GetPixelShader().Get(), nullptr, 0);
            m_immediateContext->DrawIndexed(iRenderable->second->GetNumIndices(), 0, 0);
        }
        // present the information rendered to the back buffer to the front buffer
        m_swapChain->Present(0, 0);
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable
      Summary:  Sets the vertex shader for a renderable
      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetVertexShaderOfRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::const_iterator iRenderable = m_renderables.find(pszRenderableName);
        std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>::const_iterator iVertexShader = m_vertexShaders.find(pszVertexShaderName);

        if (iRenderable == m_renderables.end() || iVertexShader == m_vertexShaders.end())
        {
            return E_FAIL;
        }

        iRenderable->second->SetVertexShader(iVertexShader->second);

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable
      Summary:  Sets the pixel shader for a renderable
      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader
      Modifies: [m_renderables].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetPixelShaderOfRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::const_iterator iRenderable = m_renderables.find(pszRenderableName);
        std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>::const_iterator iPixelShader = m_pixelShaders.find(pszPixelShaderName);
        
        if (iRenderable == m_renderables.end() || iPixelShader == m_pixelShaders.end())
        {
            return E_FAIL;
        }

        iRenderable->second->SetPixelShader(iPixelShader->second);

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType
      Summary:  Returns the Direct3D driver type
      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::GetDriverType definition (remove the comment)
    --------------------------------------------------------------------*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}