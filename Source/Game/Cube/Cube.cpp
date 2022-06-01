#include "Cube/Cube.h"


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Cube
  Summary:  Constructor
  Args:     const XMFLOAT4& outputColor
              Default color of the cube
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
Cube::Cube(_In_ const XMFLOAT4& outputColor)
    : BaseCube(outputColor)
{
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Update

  Summary:  Updates the cube every frame

  Args:     FLOAT deltaTime
              Elapsed time

  Modifies: [m_world].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void Cube::Update(_In_ FLOAT deltaTime)
{
    static FLOAT s_totalTime = 0.0f;
    s_totalTime += deltaTime;

    //m_world = XMMatrixTranslation(0.0f, XMScalarSin(s_totalTime), 0.0f) * XMMatrixRotationY(s_totalTime);
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Initialize

  Summary:  Initialize the cube

  Args:     ID3D11Device* pDevice
              ID3D11Device device
            ID3D11DeviceContext* pImmediateContext
              Immediate context

  Modifies: [m_aMeshes].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HRESULT Cube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
    BasicMeshEntry basicMeshEntry;
    basicMeshEntry.uNumIndices = NUM_INDICES;

    m_aMeshes.push_back(basicMeshEntry);

    if (HasTexture())
    {
        SetMaterialOfMesh(0, 0);
    }

    return initialize(pDevice, pImmediateContext);
}
