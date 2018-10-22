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
    int numLights = 1;
    int numDirLights = 1;
    int numPointLights = 0;
    int numSpotLights = 0;
    float lightColor[16][3] {0};
}

void initImGui(ID3D11Device *device, ID3D11DeviceContext *context)
{
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

        ImGui::SliderInt("Number of lights: ", &ImGuiData::numLights, 1, 16, "%d");

        for(int i = 1;i <= ImGuiData::numLights; ++i)
        {
            //Edit the number of lights
            std::string lightName = "Light ";
            lightName.append(std::to_string(i));
            if(ImGui::CollapsingHeader(lightName.c_str()))
            {
                ImGui::ColorEdit3(lightName.c_str(), (float*)ImGuiData::lightColor[i - 1]);
            }
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }


    //Rendering
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

//I think this function is unneeded? Maybe??
/*
void imGuiDraw()
{
    ImDrawData *drawData = ImGui::GetDrawData();

    if(drawData->Valid)
    {
        int drawListIt = 0;
        //Loop through draw lists
        while(drawListIt < drawData->CmdListsCount)
        {
            ImDrawList *drawList = drawData->CmdLists[drawListIt];
            int commandIt = 0;
            int indexIt = 0;
            //Iterate through draw commands
            while(commandIt < drawList->CmdBuffer.size())
            {
                //Draw all the things



                indexIt += drawList->CmdBuffer[commandIt].ElemCount;
                ++commandIt;
            }
            
            ++drawListIt;
        }

    }
}*/
