
#include "ShaderInfo.cginc"
#include "ShaderLight.cginc"
#include "ShaderShadow.cginc"
#include "ShaderReg.cginc"

#define REFRACTION_INDEX_GLASS 1.2

cbuffer EYE : SHADER_REG_PS_CB_EYE
{
    float4 eyePos;
};


TextureCube cmTex : SHADER_REG_PS_SRV_CM;
Texture2D diffuseTex : SHADER_REG_PS_SRV_DIFFUSE;
Texture2D normalTex : SHADER_REG_PS_SRV_NORMAL;
Texture2D ssaoTex : SHADER_REG_PS_SRV_SSAO;
//...

SamplerState cmSamp : SHADER_REG_PS_SAMP_CM;
SamplerState samp : SHADER_REG_PS_SAMP_TEX;

float3 GetBodyNormal(float2 tex)
{
    float3 ori_tex = normalTex.Sample(samp, tex).xyz;
    return (ori_tex * 2 - 1);
}
float3 Refract(float3 d, float3 n, float i)
{
    float3 newD = normalize(d - n);
    return Lerp(d, newD, saturate(i - 1));
}

float3 ComputeTransparency(float3 color, float3 normal, float3 look)
{
    float3 rtex3d = Refract(look, normal, REFRACTION_INDEX_GLASS-0.02f);
    float3 gtex3d = Refract(look, normal, REFRACTION_INDEX_GLASS);
    float3 btex3d = Refract(look, normal, REFRACTION_INDEX_GLASS+0.02f);
    float3 transpColor = float3(
        cmTex.SampleLevel(cmSamp, rtex3d, 0).r,
        cmTex.SampleLevel(cmSamp, gtex3d, 0).g,
        cmTex.SampleLevel(cmSamp, btex3d, 0).b);

    return Lerp(color, transpColor, 1 - mDiffuse.w);
}

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 pPos : TEXCOORD0;
    float3 wPos : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float2 tex : TEXCOORD3;
    float3 tangent : TEXCOORD4;
};
float4 main(PS_INPUT input) : SV_Target
{

    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    

    input.tangent = normalize(input.tangent - dot(input.normal, input.tangent)*input.normal);
    
    float3 bitangent = cross(input.normal, input.tangent);
    float3x3 tbn = float3x3(input.tangent, bitangent, input.normal);
    float3 tNormal = GetBodyNormal(input.tex);
    float3 wNormal = normalize(mul(tNormal, tbn));

    float3 look = normalize(input.wPos-eyePos.xyz);
    
    
    float3 ambient = 0;
    float3 diffuse = 0;
    float3 specular = 0;
    float3 reflection = 0;
    float3 A, D, S;
    ComputeDirectionalLight(wNormal, -look, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    
    ComputePointLight(input.wPos, wNormal, -look, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    
    ComputeSpotLight(input.wPos, wNormal, -look, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;

    float4x4 uvMat = float4x4(
        0.5, 0, 0, 0,
        0, -0.5, 0, 0,
        0, 0, 0, 0,
        0.5, 0.5, 0, 1);
    input.pPos = mul(input.pPos, uvMat);
    float2 viewUV = input.pPos.xy / input.pPos.w;
    float ssao = ssaoTex.SampleLevel(samp, viewUV, 0).r;
    ambient *= ssao;
    
    float3 light = specular + diffuse + ambient;
    
    float3 tex = diffuseTex.Sample(samp, input.tex).xyz;
    
    float shadowFactor = DirectionalLightOpaqueShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float shadowTranspFactor = DirectionalLightTranspShadowFactor(d_Dir[0].xyz, input.wPos);
    //debug--------------------------------------------------
    float4 pLightPos = mul(lightVPT, float4(input.wPos, 1));
    float3 lightPerspective = pLightPos.xyz / pLightPos.w;
    return float4(shadowTranspFactor.xxx, 1);

    float mapWidth, mapHeight;
    shadowTranspTex.GetDimensions(mapWidth, mapHeight);
    float dx = 1.0f / mapWidth;
    float dy = 1.0f / mapHeight;
    float4 centerSample = shadowTranspTex.SampleLevel(shadowTranspSamp, lightPerspective.xy, 0);
    float3 centerWDir = centerSample.xyz;
    float centerPDist = centerSample.w;
    
    /*
    float totalIntensity = dot(centerWDir, d_Dir[0].xyz);
    float totalWeight = 0.2f;
    
    float3 offsets[8] =
    {
        float3(-dx, -dy, 0.075f), float3(0, -dy, 0.125f), float3(dx, -dy, 0.075),
        float3(-dx, 0, 0.125), float3(dx, 0, 0.125),
       float3(-dx, dy, 0.075), float3(0, dy, 0.125), float3(dx, dy, 0.075)
    };
    
    for (int i = 0; i < 8; ++i)
    {
        float4 shadowSample = shadowTranspTex.SampleLevel(shadowTranspSamp, lightPerspective.xy + offsets[i].xy, 0);
        float3 pWDir = shadowSample.xyz;
        float pDepth = shadowSample.w;
        
        if (dot(pWDir, centerWDir) >= 0.8 &&
            abs(pDepth - centerPDist) <= 0.2)
        {
            totalIntensity += dot(pWDir, lightDir);
            totalWeight += offsets[i].z;
        }

    }
    
    return totalIntensity / totalWeight;
    */
    
    return float4(shadowTranspFactor.xxx, 1);
    //-------------------------------------------------------
    
    tex = ComputeTransparency(tex, wNormal, look);
    
    float3 color = 0;
    return float4(color, mDiffuse.w);
}