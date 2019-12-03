
#include "ShaderReg.cginc"

cbuffer LIGHT_PERSPECTIVE : SHADER_REG_CB_LIGHTVP
{
    float4x4 lightVPT;
};

Texture2D shadowTex : SHADER_REG_SRV_SHADOW;
Texture2D shadowTranspTex : SHADER_REG_SRV_SHADOW_TRANSP;

SamplerComparisonState shadowSamp : SHADER_REG_SAMP_CMP_POINT;
SamplerState pointSamp : SHADER_REG_SAMP_POINT;

float DirectionalLightOpaqueShadowFactor(float3 wNormal, float3 lightDir, float3 wPos)
{
    if (dot(wNormal, lightDir)>0)
        return 1;

    float4 pLightPos = mul(lightVPT, float4(wPos, 1));
    float3 lightPos = pLightPos.xyz / pLightPos.w;

    float mapWidth, mapHeight;
    shadowTex.GetDimensions(mapWidth, mapHeight);
    float dx = 1.0f / mapWidth;
    float dy = 1.0f / mapHeight;

    float3 offsets[9] =
    {
        float3(-dx, -dy, 0.05), float3(0, -dy, 0.15), float3(dx, -dy, 0.05),
        float3(-dx, 0, 0.15), float3(0, 0, 0.2), float3(dx, 0, 0.15),
        float3(-dx, dy, 0.05), float3(0, dy, 0.15), float3(dx, dy, 0.05)
    };
    
    float totalFactor = 0;
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        totalFactor += shadowTex.SampleCmpLevelZero(shadowSamp, lightPos.xy + offsets[i].xy, lightPos.z).r * offsets[i].z;
    }
    return (1-totalFactor);
}
float DirectionalLightTranspShadowFactor(float3 lightDir, float3 wPos)
{
    float4 pLightPos = mul(lightVPT, float4(wPos, 1));
    float3 lightPerspective = pLightPos.xyz / pLightPos.w;

    
    // center first
    float4 centerSample = shadowTranspTex.SampleLevel(pointSamp, lightPerspective.xy, 0);
    float3 centerWDir = centerSample.xyz;
    float centerPDist = centerSample.w;

    // front surface = no shadow factor
    if ((lightPerspective.z-centerPDist) <= 0.001f)
        return 0;

    
    
    
    float totalIntensity = dot(-centerWDir, lightDir)*0.2f;
    float totalWeight = 0.2f;
    
    // 3x3 except center
    float mapWidth, mapHeight;
    shadowTranspTex.GetDimensions(mapWidth, mapHeight);
    float dx = 1.0f / mapWidth;
    float dy = 1.0f / mapHeight;
    float3 offsets[4] =
    {
        float3(0, -dy, 0.2f), float3(0, dy, 0.2f),
        float3(-dx, 0, 0.2f), float3(dx, 0, 0.2f)
    };
    
    for (int i = 0; i < 4; ++i)
    {
        float4 shadowSample = shadowTranspTex.SampleLevel(pointSamp, lightPerspective.xy + offsets[i].xy, 0);
        float3 pWDir = shadowSample.xyz;
        float pDepth = shadowSample.w;
        
        if(abs(pDepth - centerPDist)<=0.1)
        {
            totalIntensity += dot(-pWDir, lightDir) * offsets[i].z;
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
