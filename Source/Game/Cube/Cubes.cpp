#include "Cubes.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   EarthCube::Update

  Summary:  Rotates around the origin.
			Rotates around the y-axis each frame by angle of delta time
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void EarthCube::Update(_In_ FLOAT deltaTime)
{
	RotateY(deltaTime);
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   MoonCube::Update

  Summary:  Orbits around the origin with 4.0 as the orbit radius
			and rotates around the z-axis
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void MoonCube::Update(_In_ FLOAT deltaTime)
{
	elapsedTime += deltaTime;
	XMMATRIX mSpin = XMMatrixRotationZ(elapsedTime * -1.0f);
	XMMATRIX mOrbit = XMMatrixRotationY(elapsedTime * -2.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	m_world = mScale * mSpin * mTranslate * mOrbit;
}