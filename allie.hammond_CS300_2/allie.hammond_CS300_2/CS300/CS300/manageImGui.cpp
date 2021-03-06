/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: manageImGui.cpp
Purpose: Configure all the ImGUI options
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 11
Operating System requirement: Windows
Project: allie.hammond_CS300_2
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/18/2018
End Header --------------------------------------------------------*/

#include <Windows.h>
#include <string>

#include "manageImGui.h"

#include "ImGui/imgui.h"
#include "ImGUI\imgui_impl_win32.h"
#include "ImGUI\imgui_impl_dx11.h"

namespace WinData
{
    extern HINSTANCE *hInst;
    extern WNDCLASS wndClass;
    extern HWND windowHandle;
    extern int windowHeight, windowWidth;
}

namespace ImGuiData
{
    float clearCol[4] = {1.0f, 0.298f, 0.561f, 1.0f};
    int numLights = 4;
    float lightColor[16][3] {1.0f};
    int lightType[16] {0};
    float lightDirection[16][3]{1.0f};
    float lightPos[16][3]{1.0f};
    float theta[16]{0.5f};
    float phi[16]{1.0f};
    float cameraZoom = 18.0f;
    float Ka = 0.0f;
    float globalAmbient[3] = {0.6f, 0.6f, 0.6f};
    float Ns = 16.0f;
    int shader = 1; //0 - VSPhong 1 - PSPhong 2 - PSBlinn
}

void initImGuiValues()
{
    
    for(int i = 0;i < 16; ++i)
    {
        ImGuiData::lightColor[i][0] = 0.0f;
        ImGuiData::lightColor[i][1] = 0.0f;
        ImGuiData::lightColor[i][2] = 1.0f;
        ImGuiData::lightDirection[i][0] = -1.0f;
        ImGuiData::lightDirection[i][1] = -1.0f;
        ImGuiData::lightDirection[i][2] = -1.0f;
        ImGuiData::lightType[i] = 1;
    }
    ImGuiData::numLights = 3;
    ImGuiData::lightColor[0][0] = 1.0f;
    ImGuiData::lightColor[0][2] = 1.0f;
    ImGuiData::lightColor[1][1] = 1.0f;
    ImGuiData::lightColor[1][2] = 0.0f;
}

void initImGui(ID3D11Device *device, ID3D11DeviceContext *context)
{
    initImGuiValues();
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    ImGui_ImplWin32_Init(WinData::windowHandle);
    ImGui_ImplDX11_Init(device, context);

    // Setup style
    ImGui::StyleColorsDark();
}

void cleanupImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void renderImGuiFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("CS300 Project!");                          // Create a window called "Hello, world!" and append into it.

        //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

        ImGui::ColorEdit3("clear color", (float*)&ImGuiData::clearCol); // Edit 3 floats representing a color

        ImGui::SliderFloat("Camera Zoom", &ImGuiData::cameraZoom, 2.0f, 50.0f);

        ImGui::ColorEdit3("Global Ambient", (float*)&ImGuiData::globalAmbient);

        ImGui::SliderFloat("Model ambient intensity (Ka): ", &ImGuiData::Ka, 0.0f, 1.0f);

        ImGui::SliderFloat("Specular constant (ns): ", &ImGuiData::Ns, 1.0f, 256.0f);

        //Shader selection
        const char *lightOptions[] = { "Phong Lighting (VS)", "Phong Shading (PS)", "Blinn Shading (PS)" };
        ImGui::ListBox("Shader type", &ImGuiData::shader, lightOptions, 3);

        ImGui::SliderInt("Number of lights: ", &ImGuiData::numLights, 1, 16, "%d");

        for(int i = 0;i < 16; ++i)
        {
            //Dont allow light direction to be 0
            if (ImGuiData::lightDirection[i][0] == 0.0f && ImGuiData::lightDirection[i][1] == 0.0f && ImGuiData::lightDirection[i][2] == 0.0f)
                ImGuiData::lightDirection[i][1] = 0.1f;
        }


        if(ImGui::CollapsingHeader("Light Settings"))
        {
            for(int i = 0;i < ImGuiData::numLights; ++i)
            {
                //Edit the number of lights
                std::string lightName = "Light ";
                lightName.append(std::to_string(i));
                if(ImGui::CollapsingHeader(lightName.c_str()))
                {
                    //Light color selection
                    ImGui::ColorEdit3(lightName.c_str(), (float*)ImGuiData::lightColor[i]);
                    //Light type selection
                    const char *lightOptions[] = {"Directional", "Point", "Spotlight"};
                    std::string lightTypeName = "Light Type: ";
                    lightTypeName.append(std::to_string(i));
                    ImGui::ListBox(lightTypeName.c_str(), &ImGuiData::lightType[i], lightOptions, 3);

                    //Directional light options
                    if(ImGuiData::lightType[i] == 0 || ImGuiData::lightType[i] == 2)
                    {
                        std::string lightDirName = "Direction: ";
                        lightDirName.append(std::to_string(i));
                        ImGui::SliderFloat3(lightDirName.c_str(), ImGuiData::lightDirection[i], -1.0f, 1.0f);
                    }
                    //Spotlight specific light options
                    if (ImGuiData::lightType[i] == 2)
                    {
                        std::string innerAngleName = "Theta(Inner Angle): ";
                        innerAngleName.append(std::to_string(i));
                        ImGui::InputFloat(innerAngleName.c_str(), &ImGuiData::theta[i]);
                        std::string outerAngleName = "Phi(Outer Angle): ";
                        outerAngleName.append(std::to_string(i));
                        ImGui::InputFloat(outerAngleName.c_str(), &ImGuiData::phi[i]);
                    }
                }
            }
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }


    //Rendering
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
