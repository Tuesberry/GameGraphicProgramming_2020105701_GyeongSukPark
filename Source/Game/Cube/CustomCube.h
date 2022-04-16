/*+===================================================================
  File:      CUSTOMCUBE.H

  Summary:   Custom Cube header file contains declarations of Custom
             Cube class
  
  Classes: CustomCube

  © 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    Cube

  Summary:  A renderable 3d cube object

  Methods:  Update
              Overriden function that updates the cube every frame
            Cube
              Constructor.
            ~Cube
              Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class CustomCube : public BaseCube
{
public:
    CustomCube(const std::filesystem::path& textureFilePath);
    CustomCube(const CustomCube& other) = delete;
    CustomCube(CustomCube&& other) = delete;
    CustomCube& operator=(const CustomCube& other) = delete;
    CustomCube& operator=(CustomCube&& other) = delete;
    ~CustomCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};