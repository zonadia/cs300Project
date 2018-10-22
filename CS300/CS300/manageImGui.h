#pragma once

#include <d3d11.h>

void initImGui(ID3D11Device *device, ID3D11DeviceContext *context);
void cleanupImGui();
void renderImGuiFrame();
void imGuiDraw();

