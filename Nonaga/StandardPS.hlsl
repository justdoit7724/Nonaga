
#include "ShaderInfo.cginc"
#include "ShaderLight.cginc"
#include "ShaderShadow.cginc"
#include "ShaderReg.cginc"

#define REFRACTION_INDEX_GLASS 1.2

cbuffer EYE : SHADER_REG_CB_EYE
{
    float4 eyePos;
};


TextureCube cmTex : SHADER_REG_SRV_CM;
Texture2D diffuseTex : SHADER_REG_SRV_DIFFUSE;
Texture2D normalTex : SHADER_REG_SRV_NORMAL;
Texture2D ssaoTex : SHADER_REG_SRV_SSAO;
//...

SamplerState anisotropicSamp : SHADER_REG_SAMP_ANISOTROPIC;
SamplerState linearPointSamp : SHADER_REG_SAMP_LINEAR_POINT;
SamplerState linearSamp : SHADER_REG_SAMP_LINEAR;

float3 GetBodyNormal(float2 tex)
{
    float3 ori_tex = normalTex.Sample(anisotropicSamp, tex).xyz;
    return (ori_tex * 2 - 1);
}

float3 ComputeTransparency(float3 wPos, float3 normal, float3 look)
{
    float3 rtex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 1.01f));
    float3 gtex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 1));
    float3 btex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 0.99));
    return float3(
        cmTex.SampleLevel(pointSamp, rtex3d, 0).r,
        cmTex.SampleLevel(pointSamp, gtex3d, 0).g,
        cmTex.SampleLevel(pointSamp, btex3d, 0).b);
}
float3 ComputeReflect(float3 wNormal, float3 look)
{
    return cmTex.SampleLevel(pointSamp, reflect(look, wNormal), 0).xyz;
}
float ComputeSSAO(float4 pPos)
{
    float2 viewUV = pPos.xy / pPos.w;
    viewUV.x = viewUV.x * 0.5f + 0.5f;
    viewUV.y = -viewUV.y * 0.5f + 0.5f;
    float ssao = ssaoTex.SampleLevel(pointSamp, viewUV, 0).r;
    return pow(ssao, 1.8f);
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
    ComputeDirectionalLight(wNormal, -look, ambient, diffuse, specular);

    ambient *= ComputeSSAO(input.pPos);
    
    float3 transp = ComputeTransparency(input.wPos, wNormal, look);
    float3 tex = diffuseTex.Sample(anisotropicSamp, input.tex).xyz;
    float3 reflec = ComputeReflect(wNormal, look);
    
    float oShadowFactor = DirectionalLightOpaqueShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float2 tShadowFactor = DirectionalLightTranspShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float shadowFactor = max(oShadowFactor, tShadowFactor.x);
    
    ambient *= tex;
    diffuse *= tex * saturate(1 - shadowFactor);
    float3 surface = Lerp(ambient + diffuse, transp, mInfo.x);
    surface = Lerp(surface, reflec, mInfo.y);
    
    specular += (0 + saturate(0.06f - tShadowFactor.x) * tShadowFactor.y);
    
    return float4(specular + surface, 1);
}