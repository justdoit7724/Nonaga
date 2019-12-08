
#include "ShaderInfo.cginc"
#include "ShaderLight.cginc"
#include "ShaderShadow.cginc"
#include "ShaderReg.cginc"

#define REFRACTION_INDEX_GLASS 1.2
#define ROUGHNESS_MAX 16

cbuffer EYE : SHADER_REG_CB_EYE
{
    float4 eyePos;
};


TextureCube cmTex : SHADER_REG_SRV_CM;
TextureCube dcmTex : SHADER_REG_SRV_DCM;
Texture2D diffuseTex : SHADER_REG_SRV_DIFFUSE;
Texture2D normalTex : SHADER_REG_SRV_NORMAL;
Texture2D ssaoTex : SHADER_REG_SRV_SSAO;
Texture2D roughnessTex : SHADER_REG_SRV_ROUGHNESS;
//...

SamplerState anisotropicSamp : SHADER_REG_SAMP_ANISOTROPIC;
SamplerState linearPointSamp : SHADER_REG_SAMP_LINEAR_POINT;

float3 GetBodyNormal(float2 tex)
{
    float3 ori_tex = normalTex.Sample(linearPointSamp, tex).xyz;
    return (ori_tex * 2 - 1);
}
float3 ComputeTransparency(float3 wPos, float3 normal, float3 look)
{
    float3 rtex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 1.01f));
    float3 gtex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 1));
    float3 btex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 0.99));
    
    float3 cm = float3(
    cmTex.SampleLevel(pointSamp, rtex3d, 0).r,
    cmTex.SampleLevel(pointSamp, gtex3d, 0).g,
    cmTex.SampleLevel(pointSamp, btex3d, 0).b);
    
    float4 dcmr = dcmTex.SampleLevel(pointSamp, rtex3d, 0);
    float4 dcmg = dcmTex.SampleLevel(pointSamp, gtex3d, 0);
    float4 dcmb = dcmTex.SampleLevel(pointSamp, btex3d, 0);
    
    return float3(
    lerp(cm.r, dcmr.r, dcmr.w),
    lerp(cm.g, dcmg.g, dcmg.w),
    lerp(cm.b, dcmb.b, dcmb.w));
}
float3 ComputeReflect(float3 wNormal, float3 look)
{
    float3 reflDir = reflect(look, wNormal);
    
    float3 cm = cmTex.SampleLevel(pointSamp, reflDir, 0).xyz;
    float4 dcm = dcmTex.SampleLevel(pointSamp, reflDir, 0);
    return Lerp(cm, dcm.xyz, dcm.w);
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
    input.tangent = normalize(input.tangent - dot(input.normal, input.tangent) * input.normal);
    
    float3 bitangent = cross(input.normal, input.tangent);
    float3x3 tbn = float3x3(input.tangent, bitangent, input.normal);
    float3 tNormal = GetBodyNormal(input.tex);
    float3 wNormal = normalize(mul(tNormal, tbn));

    float3 look = normalize(input.wPos - eyePos.xyz);
    
    //debug 
    // 2.brighten p2_rgh texture bit more
    float3 ambient = 0;
    float3 diffuse = 0;
    float3 specular = 0;
    float roughness = roughnessTex.Sample(pointSamp, input.tex).r;
    ComputeDirectionalLight(wNormal, -look, max(1, (1 - roughness) * ROUGHNESS_MAX), ambient, diffuse, specular);
    specular *= (1 - roughness);
    float transpT = mInfo.x * (1 - roughness);
    float refractT = mInfo.y * (1 - roughness);

    ambient *= ComputeSSAO(input.pPos);
    
    float3 transp = ComputeTransparency(input.wPos, wNormal, look);
    float3 tex = diffuseTex.Sample(anisotropicSamp, input.tex).xyz;
    float3 reflec = ComputeReflect(wNormal, look);
    
    float oShadowFactor = DirectionalLightOpaqueShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float2 tShadowFactor = DirectionalLightTranspShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float shadowFactor = saturate(1 - max(oShadowFactor, tShadowFactor.x));
    specular = specular * shadowFactor + (saturate(0.05f - tShadowFactor.x) * tShadowFactor.y);
    ambient *= tex;
    diffuse *= tex * shadowFactor;
    
    float3 surface = Lerp(ambient + diffuse, transp, transpT);
    surface = Lerp(surface, reflec, refractT);
    
    return float4(surface + specular, 1);

}