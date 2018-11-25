/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: BlinnShadingPS.hlsl
Purpose: Blinn shading pixel shader
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 11
Operating System requirement: Windows
Project: allie.hammond_CS300_2
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/12/2018
End Header --------------------------------------------------------*/

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix worldTransMatrix;
    matrix MVPMatrix;
    matrix Rotation;
    float4 globalAmbient;
    float4 lightDir[16]; //Light direction (for directional/spotlights)
    float4 Ia[16]; //Light ambient intensity (light color)
    float4 lightPos[16]; // Light position
    float4 theta[16]; //For spotlights
    float4 phi[16]; // For spotlights
    int4 numLights; //Number of lights
    int4 lightType[16]; //Type of light 0 - dir 1 - point 2 - spotlight
    float4 camPos;
    float4 Ka;
    float4 Ns;
    float4 meshSize; //xMin, xMax, yMin, yMax
    float4 meshSizeZ; //zMin, zMax
};

struct PS_INPUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float3 vColor : COLOR;
    float3 worldPos : POSITION;
};

float4 PSMain(PS_INPUT Input) : SV_TARGET
{
    float3 outColor = globalAmbient.xyz * Ka[0];
    float Kd = 1.0f;
    float Ks = 1.0f;
    float KaLight = 0.1f;

    //Loop through all the lights
    for (int i = 0; i < numLights[0]; ++i)
    {
        //Constants
        float c1 = 1.0f;
        float c2 = 0.1f;
        float c3 = 0.1f;


        //Calculate attenuation
        float dL = distance(Input.worldPos, lightPos[i].xyz);
        float att = 1.0f;


        //L = light direction
        float3 L;

        if (lightType[i][0] == 1)//Point light
        {
            //Use vector from surface to light as L
            L = normalize(lightPos[i].xyz - Input.worldPos);
        }
        else//Not a point light
        {
            L = normalize(lightDir[i].xyz);
        }

        if (lightType[i][0] != 0)//For spotlights or point lights
        {
            att = min(1.0f / (c1 + c2*dL + c3 * dL *dL), 1.0f);
        }

        //Ambient
        outColor += Ia[i].xyz * att * KaLight;
        //Specular and diffuse
        float spotlight = 1.0f;

        if (lightType[i][0] == 2)//Spotlight
        {
            //D = unit vector from light source to vertex
            float3 D = normalize(Input.worldPos - lightPos[i].xyz);
            float cosPhi = cos(phi[i][0]);
            float cosTheta = cos(theta[i][0]);
            float cosAlpha = dot(L, D);

            if (cosAlpha < cosPhi)
            {
                spotlight = 0.0f;
            }
            else
                if (cosAlpha > cosTheta)
                {
                    spotlight = 1.0f;
                }
                else
                {
                    float p = 0.5f;
                    spotlight = pow(((cosAlpha - cosPhi) / (cosTheta - cosPhi)), p);
                }
        }

        //Diffuse and specular
        float3 Idiffuse = Ia[i].xyz * Kd * dot(L.xyz, normalize(Input.vNormal.xyz));

        float3 V = normalize(camPos.xyz - Input.worldPos);
        float3 N = normalize(Input.vNormal.xyz);
        float3 H = normalize(L + V);

        float3 Ispecular = Ia[i].xyz * Ks * pow(max(dot(N, H), 0.0f), Ns[0]);
        outColor += att * spotlight * (Idiffuse + Ispecular);
    }

    outColor.x = min(outColor.x, 1.0f);
    outColor.y = min(outColor.y, 1.0f);
    outColor.z = min(outColor.z, 1.0f);


    return float4(outColor, 1.0f);
}
