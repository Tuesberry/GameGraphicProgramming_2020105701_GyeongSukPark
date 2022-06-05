#include "Light/RotatingPointLight.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   RotatingPointLight::RotatingPointLight
  Summary:  Constructor
  Args:     const XMFLOAT4& position
              Position of the light
            const XMFLOAT4& color
              Position of the color
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
/*--------------------------------------------------------------------
  TODO: RotatingPointLight::RotatingPointLight definition (remove the comment)
--------------------------------------------------------------------*/
RotatingPointLight::RotatingPointLight(_In_ const XMFLOAT4& position, _In_ const XMFLOAT4& color, _In_ FLOAT attenuationDistance)
    : PointLight(position, color, attenuationDistance)
{
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   RotatingPointLight::Update
  Summary:  Update every frame
  Args:     FLOAT deltaTime
  Modifies: [m_position, m_eye, m_eye, m_at,
            m_view].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
/*--------------------------------------------------------------------
  TODO: RotatingPointLight::Update definition (remove the comment)
--------------------------------------------------------------------*/
void RotatingPointLight::Update(_In_ FLOAT deltaTime)
{
    // rotate around the origin
    // the light rotates around the y-axis each frame by angle of -2.0f * deltaTime
    XMMATRIX rotate = XMMatrixRotationY(-2.0f * deltaTime);
    XMVECTOR position = XMLoadFloat4(&m_position);
    position = XMVector3Transform(position, rotate);
    XMStoreFloat4(&m_position, position);
    // eye: current position of the light
    m_eye = position;
    // at: (0.0, 0.0, 0.0, 1.0)
    m_at = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    // up: default_up
    m_up = DEFAULT_UP;
    // create the view matrix
    m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);
}