
#include "ShaderInfo.cginc"
#include "ShaderLight.cginc"
#include "ShaderReg.cginc"

#define REFRACTION_INDEX_GLASS 1.2

cbuffer EYE : SHADER_REG_CB_EYE
{
    float4 eyePos;
};
cbuffer LIGHT_PERSPECTIVE : SHADER_REG_CB_LIGHTVP
{
    float4x4 lightVPT;
};

TextureCube cmTex : SHADER_REG_SRV_CM;
Texture2D diffuseTex : SHADER_REG_SRV_DIFFUSE;
Texture2D normalTex : SHADER_REG_SRV_NORMAL;
Texture2D shadowTex : SHADER_REG_SRV_SHADOW;
Texture2D shadowTranspTex : SHADER_REG_SRV_SHADOW_TRANSP;
//...

SamplerComparisonState shadowSamp : SHADER_REG_SAMP_CMP_POINT;
SamplerState pointSamp : SHADER_REG_SAMP_POINT;

float3 GetBodyNormal(float2 tex)
{
    float3 ori_tex = normalTex.Sample(pointSamp, tex).xyz;
    return (ori_tex * 2 - 1);
}

float3 ComputeTransparency(float3 wPos, float3 normal, float3 look)
{
    float3 tex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 1));
    return cmTex.SampleLevel(pointSamp, tex3d, 0).rgb;
}
float3 ComputeReflect(float3 wNormal, float3 look)
{
    return cmTex.SampleLevel(pointSamp, reflect(look, wNormal), 0).xyz;
}

float DirectionalLightOpaqueShadowFactor(float3 wNormal, float3 lightDir, float3 wPos)
{
    if (dot(wNormal, lightDir) > 0)
        return 0;
    
    float4 pLightPos = mul(lightVPT, float4(wPos, 1));
    float3 lightPos = pLightPos.xyz / pLightPos.w;

    float mapWidth, mapHeight;
    shadowTex.GetDimensions(mapWidth, mapHeight);
    float dx = 1.0f / mapWidth;
    float dy = 1.0f / mapHeight;
    
    return saturate(1-shadowTex.SampleCmpLevelZero(shadowSamp, lightPos.xy, lightPos.z).r);
}
float2 DirectionalLightTranspShadowFactor(float3 wDir, float3 lightDir, float3 wPos)
{
    float4 pLightPos = mul(lightVPT, float4(wPos, 1));
    float3 lightPerspective = pLightPos.xyz / pLightPos.w;

    float4 centerSample = shadowTranspTex.SampleLevel(pointSamp, lightPerspective.xy, 0);
    float3 centerWDir = centerSample.xyz;
    float centerPDist = centerSample.w;

    float power = saturate(pow(3 * dot(-wDir, lightDir), 3));
    float overlap = saturate((lightPerspective.z - centerPDist) * 100);
    
    float totalIntensity = 1 - dot(-centerWDir, lightDir);
    
    
    float area = saturate(ceil(lightPerspective.z - centerPDist));
    return float2(power * overlap * totalIntensity, area);
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
    input.tangent = normalize(input.tangent - dot(input.normal, input.tangent) * input.normal);
    
    float3 bitangent = cross(input.normal, input.tangent);
    float3x3 tbn = float3x3(input.tangent, bitangent, input.normal);
    float3 tNormal = GetBodyNormal(input.tex);
    float3 wNormal = normalize(mul(tNormal, tbn));

    float3 look = normalize(input.wPos - eyePos.xyz);
    
    float3 ambient = 0;
    float3 diffuse = 0;
    float3 specular = 0;
    ComputeDirectionalLight(wNormal, -look, ambient, diffuse, specular);

    float3 transp = ComputeTransparency(input.wPos, wNormal, look);
    float3 tex = diffuseTex.Sample(pointSamp, input.tex).xyz;
    float3 reflec = ComputeReflect(wNormal, look);
    
    float oShadowFactor = DirectionalLightOpaqueShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float2 tShadowFactor = DirectionalLightTranspShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float shadowFactor = max(oShadowFactor, tShadowFactor.x);
    
    ambient *= tex;
    diffuse *= tex * saturate(1 - shadowFactor);
    float3 surface = Lerp(ambient + diffuse, transp, mInfo.x);
    surface = Lerp(surface, reflec, mInfo.y);
    
    specular *= (1 + saturate(0.2f - tShadowFactor.x) * tShadowFactor.y);
    
    return float4(specular + surface, 1);
}