/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: BasicHLSL_VS.hlsl
Purpose: Default Vertex Shader
Language: Visual Studio 2017 C++
Platform: Compiler : Visual Studio C++ 14.0
Hardware must support DirectX 10 or 11
Operating System requirement: Windows
Project: allie.hammond_CS300_1
Author: Allie Hammond (allie.hammond) (180009414)
Creation date: 10/12/2018
End Header --------------------------------------------------------*/

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
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
    float4 Ka;
};


struct VS_INPUT
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vColor : COLOR;
};

struct VS_OUTPUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float3 vColor : COLOR;
};


VS_OUTPUT VSMain(VS_INPUT Input)
{
    VS_OUTPUT Output;

    Output.vPosition = mul(float4(Input.vPosition, 1.0f), MVPMatrix);
    Output.vNormal = mul(float4(Input.vNormal, 1.0f), Rotation);

    
    float4 outColor = globalAmbient * Ka[0];
    float Kd = 0.7f;
    float Ks = 0.7f;

    //Loop through all the lights
    for(int i = 0;i < numLights[0]; ++i)
    {
        //Constants
        float c1 = 1.0f;
        float c2 = 0.1f;
        float c3 = 0.1f;

        float ns = 0.3f;

        //Calculate attenuation
        float dL = distance(Output.vPosition.xyz, lightPos[i].xyz);
        float att = 1.0f;

        if(lightType[i][0] != 0)//For spotlights or point lights
        {
            att = min(1.0f / (c1 + c2*dL + c3 * dL *dL), 1.0f);
        }

        //Ambient
        Output.vColor += Ia[i] * att;
        //Specular and diffuse
        float spotlight = 1.0f;
        //L = light direction
        float3 L = normalize(lightDir[i].xyz);
        /*if(lightType[i][0] == 2)//Spotlight
        {
            //D = unit vector from light source to vertex
            float3 D = normalize(Output.vPosition.xyz - lightPos[i].xyz);
            float cosPhi = cos(phi[i][0]);
            float cosTheta = cos(theta[i][0]);
            float cosAlpha = dot(L, D);

            if(cosAlpha < cosPhi)
            {
                spotlight = 0.0f;
            }
            else
            if(cosAlpha > cosTheta)
            {
                spotlight = 1.0f;
            }
            else
            {
                float p = 0.5f;
                spotlight = pow(((cosAlpha - cosPhi) / (cosTheta - cosPhi)), p);
            }
        }*/

        //Diffuse and specular
        float3 Idiffuse = Ia[i] * Kd * dot(L, normalize(Output.vNormal));
        float3 Idiffuse = Ia[i] * Ks * pow(dot(R, V), ns)
        Output.vColor += att * spotlight * (Idiffuse + Ispecular);
    }
    
    Output.vColor = outColor.xyz;

    return Output;
}
