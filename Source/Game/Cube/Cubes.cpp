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
	if (elapsedTime >= XM_2PI)
	{
		elapsedTime -= XM_2PI;
	}
	XMMATRIX mSpin = XMMatrixRotationZ(elapsedTime * -1.0f);
	XMMATRIX mOrbit = XMMatrixRotationY(elapsedTime * -2.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	m_world = mScale * mSpin * mTranslate * mOrbit;
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   CustomCube::Update

  Summary:  Orbits around the origin with 2.0 as the orbit radius
			and rotates around the z-axis and changes scale
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void CustomCube::Update(_In_ FLOAT deltaTime)
{
	elapsedTime += deltaTime;

	if (elapsedTime >= XM_2PI)
	{
		elapsedTime -= XM_2PI;
		
		if (bIncreaseSize)
		{
			bIncreaseSize = false;
		}
		else
		{
			bIncreaseSize = true;
		}
	}

	XMMATRIX mSpin = XMMatrixRotationZ(elapsedTime * 1.0f);
	XMMATRIX mOrbit = XMMatrixRotationY(elapsedTime * 2.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(-2.0f, 0.0f, 0.0f);

	float scaleFactor = 0.0f;
	if (bIncreaseSize)
	{
		scaleFactor = elapsedTime / XM_2PI / 2.0f;
		
	}
	else
	{
		scaleFactor = (1.0f - elapsedTime / XM_2PI)/2.0f;
	}
	XMMATRIX mScale = XMMatrixScaling(scaleFactor, scaleFactor, scaleFactor);

	m_world = mScale * mSpin * mTranslate * mOrbit;
}