
#include "ShaderReg.cginc"

cbuffer LIGHT_PERSPECTIVE : SHADER_REG_PS_CB_LIGHTVP
{
    float4x4 lightVPT;
};

Texture2D shadowTex : SHADER_REG_PS_SRV_SHADOW;
Texture2D shadowTranspTex : SHADER_REG_PS_SRV_SHADOW_TRANSP;

SamplerComparisonState shadowSamp : SHADER_REG_PS_SAMP_SHADOW;
SamplerState shadowTranspSamp : SHADER_REG_PS_SAMP_SHADOW_TRANSP;

float DirectionalLightOpaqueShadowFactor(float3 wNormal, float3 lightDir, float3 wPos)
{
    if (dot(wNormal, lightDir)>0)
    {
        return 0;
    }

    float4 pLightPos = mul(lightVPT, float4(wPos, 1));
    float3 lightPos = pLightPos.xyz / pLightPos.w;

    float mapWidth, mapHeight;
    shadowTex.GetDimensions(mapWidth, mapHeight);
    float dx = 1.0f / mapWidth;
    float dy = 1.0f / mapHeight;

    float2 offsets[25] =
    {
        float2(-dx * 2, -dy * 2), float2(-dx, -dy * 2), float2(0, -dy * 2), float2(dx, -dy * 2), float2(dx * 2, -dy * 2),
        float2(-dx * 2, -dy), float2(-dx, -dy), float2(0, -dy), float2(dx, -dy), float2(dx * 2, -dy),
        float2(-dx * 2, 0), float2(-dx, 0), float2(0, 0), float2(dx, 0), float2(dx * 2, 0),
        float2(-dx * 2, dy), float2(-dx, dy), float2(0, dy), float2(dx, dy), float2(dx * 2, dy),
        float2(-dx * 2, dy * 2), float2(-dx, dy * 2), float2(0, dy * 2), float2(dx, dy * 2), float2(dx * 2, dy * 2)
    };
    
    float percentLit = 0;
    [unroll]
    for (int i = 0; i < 25; ++i)
    {
        percentLit += shadowTex.SampleCmpLevelZero(shadowSamp, lightPos.xy + offsets[i], lightPos.z).r;
    }
    return percentLit * 0.04f;
}
float DirectionalLightTranspShadowFactor(float3 lightDir, float3 wPos)
{
    float4 pLightPos = mul(lightVPT, float4(wPos, 1));
    float3 lightPerspective = pLightPos.xyz / pLightPos.w;

    float mapWidth, mapHeight;
    shadowTranspTex.GetDimensions(mapWidth, mapHeight);
    float dx = 1.0f / mapWidth;
    float dy = 1.0f / mapHeight;
    
    // center first
    //debug chang
    float4 centerSample = shadowTranspTex.SampleLevel(shadowTranspSamp, lightPerspective.xy, 0);
    float3 centerWDir = centerSample.xyz;
    float centerPDist = centerSample.w;

    //debug remove
    if (abs(centerPDist - lightPerspective.z) <= 0.0075f)
        return 0;
    
    
    float totalIntensity = dot(centerWDir, lightDir);
    float totalWeight = 0.2f;
    
    return totalIntensity;
    
    // 3x3 except center
    float3 offsets[8] =
    {
        float3(-dx, -dy,0.075f), float3(0, -dy,0.125f), float3(dx, -dy, 0.075),
        float3(-dx, 0, 0.125), /* center */float3(dx, 0, 0.125),
       float3(-dx, dy, 0.075), float3(0, dy, 0.125), float3(dx, dy, 0.075)
    };
    
    for (int i = 0; i < 8; ++i)
    {
        float4 shadowSample = shadowTranspTex.SampleLevel(shadowTranspSamp, lightPerspective.xy + offsets[i].xy, 0);
        float3 pWDir = shadowSample.xyz;
        float pDepth = shadowSample.w;
        
        if(dot(pWDir, centerWDir)>=0.8 &&
            abs(pDepth - centerPDist)<=0.2)
        {
            totalIntensity += dot(-pWDir, lightDir);
            totalWeight += offsets[i].z;
        }

    }
    
    return totalIntensity / totalWeight;
}

float PointLightShadowFactor(float3 wNormal, float3 wPos, float3 lightPos, TextureCube map, SamplerState samp, float2 shadowPMatElem)
{
    float3 lPos = wPos - lightPos;
    
    if (dot(wNormal, lPos) > 0)
        return 1;
    
    float3 viewForward[6] =
    {
        float3(1, 0, 0),
        float3(-1, 0, 0),
        float3(0, 1, 0),
        float3(0, -1, 0),
        float3(0, 0, 1),
        float3(0, 0, -1)
    };
    float curDepth = 0;
    float3 curForward = 0;
    for (int i = 0; i < 6; ++i)
    {
        float newDepth = dot(viewForward[i], lPos);
        if (curDepth < newDepth)
        {
            curDepth = newDepth;
            curForward = viewForward[i];
        }
    }

    float vZ = curDepth;
    float pZ = shadowPMatElem.x + shadowPMatElem.y / vZ;
    
    return (map.Sample(samp, lPos).r>=pZ)?1:0;
}
