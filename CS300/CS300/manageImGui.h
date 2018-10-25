#pragma once

#include <d3d11.h>

//Light types:
//0 - Directional
//1 - Point
//2 - Spotlight

void initImGui(ID3D11Device *device, ID3D11DeviceContext *context);
void cleanupImGui();
void renderImGuiFrame();

