/*+===================================================================
  File:      CUBES.H

  Summary:   Cubes header file contains declarations of EarthCube
			 class and MoonCube class for the lab04.

  Classes: EarthCube, MoonCube

  � 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "BaseCube.h"

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    EarthCube

  Summary:  Cube class for rotating cube around the y-axis 

  Methods:  Update
              Rotates around the origin.
			  Rotates around the y-axis each frame by angle of delta time
			EarthCube
			  Constructor
			~EarthCube
			  Destructor
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class EarthCube : public BaseCube
{
public:
	EarthCube() = default;
	~EarthCube() = default;
	void Update(_In_ FLOAT deltaTime) override;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    MoonCube

  Summary:  Cube class for orbiting cube around the origin

  Methods:  Update
			  Orbits around the origin with 4.0 as the orbit radius
			  Also rotates around the z-axis
			MoonCube
			  Constructor
			~MoonCube
			  Destructor
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class MoonCube : public BaseCube
{
public:
	MoonCube() = default;
	~MoonCube() = default;
	void Update(_In_ FLOAT deltaTime) override;
private:
	float elapsedTime = 0.0f;
};