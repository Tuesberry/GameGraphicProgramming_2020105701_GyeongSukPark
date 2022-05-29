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
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection()
        , m_scenes()
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
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
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
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
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_camera.Initialize(m_d3dDevice.Get());

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::AddScene
     Summary:  Add scene to renderer
     Args:     PCWSTR pszSceneName
                 The name of the scene
               const std::shared_ptr<Scene>&
                 The shared pointer to Scene
     Modifies: [m_scenes].
     Returns:  HRESULT
                 Status code
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull
      Summary:  Return scene with the given name or null
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene
      Summary:  Set the main scene
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Modifies: [m_pszMainSceneName].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput
      Summary:  Handle user mouse input
      Args:     DirectionsInput& directions
                MouseRelativeMovement& mouseRelativeMovement
                FLOAT deltaTime
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
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
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

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
        // create camera constant buffer and update
        XMFLOAT4 camPosition;
        XMStoreFloat4(&camPosition, m_camera.GetEye());
        CBChangeOnCameraMovement cbView = {
                .View = XMMatrixTranspose(m_camera.GetView()),
                .CameraPosition = camPosition
        };
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cbView, 0, 0);
        // update the lights constant buffer
        CBLights cbLight = {
            .LightPositions = {},
            .LightColors = {}
        };
        for (int lightIndex = 0; lightIndex < NUM_LIGHTS; lightIndex++)
        {
            cbLight.LightPositions[lightIndex] = m_scenes[m_pszMainSceneName]->GetPointLight(lightIndex)->GetPosition();
            cbLight.LightColors[lightIndex] = m_scenes[m_pszMainSceneName]->GetPointLight(lightIndex)->GetColor();
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cbLight, 0, 0);
        // for each renderable
        for (auto iRenderable : m_scenes[m_pszMainSceneName]->GetRenderables())
        {
            // set the buffers
            // vertex buffer with normal data(tangent/bitangent)
            UINT aStrides[2] = {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(NormalData))
            };
            UINT aOffsets[2] = { 0u,0u };
            ComPtr<ID3D11Buffer> aBuffers[2]{
                iRenderable.second->GetVertexBuffer().Get(),
                iRenderable.second->GetNormalBuffer().Get()
            };
            m_immediateContext->IASetVertexBuffers(0, 2, aBuffers->GetAddressOf(), aStrides, aOffsets);
            // index buffer
            m_immediateContext->IASetIndexBuffer(iRenderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            // set the input layout
            m_immediateContext->IASetInputLayout(iRenderable.second->GetVertexLayout().Get());
            // update the constant buffers
            CBChangesEveryFrame cbWorld = {
                .World = XMMatrixTranspose(iRenderable.second->GetWorldMatrix()),
                .OutputColor = iRenderable.second->GetOutputColor(),
                .HasNormalMap = iRenderable.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(iRenderable.second->GetConstantBuffer().Get(), 0, nullptr, &cbWorld, 0, 0);
            // set the shaders and their input
            m_immediateContext->VSSetShader(iRenderable.second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, iRenderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, iRenderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetShader(iRenderable.second->GetPixelShader().Get(), nullptr, 0);
            
            if (iRenderable.second->HasTexture())
            {
                // for each mesh
                for (UINT i = 0u; i < iRenderable.second->GetNumMeshes(); ++i)
                {
                    UINT iMaterial = iRenderable.second->GetMesh(i).uMaterialIndex;
                    // sest pixel shader resources and sampler states
                    ComPtr<ID3D11ShaderResourceView> textures[2] = {
                        iRenderable.second->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().Get(),
                        iRenderable.second->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().Get()
                    };
                    ComPtr<ID3D11SamplerState> samplerStates[2] = {
                        iRenderable.second->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().Get(),
                        iRenderable.second->GetMaterial(iMaterial)->pNormal->GetSamplerState().Get()
                    };
                    m_immediateContext->PSSetShaderResources(0, 2, textures->GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                    // draw
                    m_immediateContext->DrawIndexed(iRenderable.second->GetMesh(i).uNumIndices, iRenderable.second->GetMesh(i).uBaseIndex, iRenderable.second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(iRenderable.second->GetNumIndices(), 0, 0);
            }
            
        }
        // for each voxel of main scene
        for (auto iVoxel : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            // set the buffers
            // vertex buffer with normal data and instance data
            UINT aStrides[3] = {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(NormalData)),
                static_cast<UINT>(sizeof(InstanceData))
            };
            UINT aOffsets[3] = { 0u,0u,0u };
            ComPtr<ID3D11Buffer> aBuffers[3]{
                iVoxel->GetVertexBuffer().Get(),
                iVoxel->GetNormalBuffer().Get(),
                iVoxel->GetInstanceBuffer().Get()
            };
            m_immediateContext->IASetVertexBuffers(0, 3, aBuffers->GetAddressOf(), aStrides, aOffsets);
            // index buffer
            m_immediateContext->IASetIndexBuffer(iVoxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            // set the input layout
            m_immediateContext->IASetInputLayout(iVoxel->GetVertexLayout().Get());
            // update the constant buffer
            CBChangesEveryFrame cbWorld = {
                .World = XMMatrixTranspose(iVoxel->GetWorldMatrix()),
                .OutputColor = iVoxel->GetOutputColor(),
                .HasNormalMap = iVoxel->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(iVoxel->GetConstantBuffer().Get(), 0, nullptr, &cbWorld, 0, 0);
            // set the shaders and their input
            m_immediateContext->VSSetShader(iVoxel->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, iVoxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, iVoxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetShader(iVoxel->GetPixelShader().Get(), nullptr, 0);
            // draw 
            if (iVoxel->HasTexture())
            {
                for (UINT i = 0u; i < iVoxel->GetNumMeshes(); ++i)
                {
                    UINT iMaterial = iVoxel->GetMesh(i).uMaterialIndex;
                    // sest pixel shader resources and sampler states
                    ComPtr<ID3D11ShaderResourceView> textures[2] = {
                        iVoxel->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().Get(),
                        iVoxel->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().Get()
                    };
                    ComPtr<ID3D11SamplerState> samplerStates[2] = {
                        iVoxel->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().Get(),
                        iVoxel->GetMaterial(iMaterial)->pNormal->GetSamplerState().Get()
                    };
                    m_immediateContext->PSSetShaderResources(0, 2, textures->GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                    // draw
                    m_immediateContext->DrawIndexedInstanced(iVoxel->GetMesh(i).uNumIndices, iVoxel->GetNumInstances(), 0, 0, 0);
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(iVoxel->GetNumIndices(), 0, 0);
            }
        }
        // for each model
        for (auto iModel : m_scenes[m_pszMainSceneName]->GetModels())
        {
            // set the buffers
            // vertex buffer with normal data(tangent/bitangent)
            UINT aStrides[2] = {
                static_cast<UINT>(sizeof(SimpleVertex)),
                static_cast<UINT>(sizeof(NormalData))
            };
            UINT aOffsets[2] = { 0u,0u };
            ComPtr<ID3D11Buffer> aBuffers[2]{
                iModel.second->GetVertexBuffer().Get(),
                iModel.second->GetNormalBuffer().Get()
            };
            m_immediateContext->IASetVertexBuffers(0, 2, aBuffers->GetAddressOf(), aStrides, aOffsets);
            // index buffer
            m_immediateContext->IASetIndexBuffer(iModel.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            // set the input layout
            m_immediateContext->IASetInputLayout(iModel.second->GetVertexLayout().Get());
            // update the constant buffers
            CBChangesEveryFrame cbWorld = {
                .World = XMMatrixTranspose(iModel.second->GetWorldMatrix()),
                .OutputColor = iModel.second->GetOutputColor(),
                .HasNormalMap = iModel.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(iModel.second->GetConstantBuffer().Get(), 0, nullptr, &cbWorld, 0, 0);
            // set the shaders and their input
            m_immediateContext->VSSetShader(iModel.second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, iModel.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, iModel.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());
            m_immediateContext->PSSetShader(iModel.second->GetPixelShader().Get(), nullptr, 0);
            // for each mesh
            if (iModel.second->HasTexture())
            {
                for (UINT i = 0u; i < iModel.second->GetNumMeshes(); ++i)
                {
                    UINT iMaterial = iModel.second->GetMesh(i).uMaterialIndex;
                    // sest pixel shader resources and sampler states
                    ComPtr<ID3D11ShaderResourceView> textures[2] = {
                        iModel.second->GetMaterial(iMaterial)->pDiffuse->GetTextureResourceView().Get(),
                        iModel.second->GetMaterial(iMaterial)->pNormal->GetTextureResourceView().Get()
                    };
                    ComPtr<ID3D11SamplerState> samplerStates[2] = {
                        iModel.second->GetMaterial(iMaterial)->pDiffuse->GetSamplerState().Get(),
                        iModel.second->GetMaterial(iMaterial)->pNormal->GetSamplerState().Get()
                    };
                    m_immediateContext->PSSetShaderResources(0, 2, textures->GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                    // draw
                    m_immediateContext->DrawIndexed(iModel.second->GetMesh(i).uNumIndices, iModel.second->GetMesh(i).uBaseIndex, iModel.second->GetMesh(i).uBaseVertex);
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(iModel.second->GetNumIndices(), 0, 0);
            }
        }
        // present the information rendered to the back buffer to the front buffer
        m_swapChain->Present(0, 0);
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::GetDriverType
     Summary:  Returns the Direct3D driver type
     Returns:  D3D_DRIVER_TYPE
                 The Direct3D driver type used
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}