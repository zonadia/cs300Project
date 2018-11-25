/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: manageImGui.h
Purpose: Configure all the ImGUI options
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 11
Operating System requirement: Windows
Project: allie.hammond_CS300_2
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/18/2018
End Header --------------------------------------------------------*/

#pragma once

#include <d3d11.h>

//Light types:
//0 - Directional
//1 - Point
//2 - Spotlight

void initImGui(ID3D11Device *device, ID3D11DeviceContext *context);
void cleanupImGui();
void renderImGuiFrame();

