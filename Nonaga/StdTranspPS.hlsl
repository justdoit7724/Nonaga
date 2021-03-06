
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
TextureCube dcmTex : SHADER_REG_SRV_DCM;
Texture2D diffuseTex : SHADER_REG_SRV_DIFFUSE;
Texture2D normalTex : SHADER_REG_SRV_NORMAL;
Texture2D ssaoTex : SHADER_REG_SRV_SSAO;
//...

SamplerState anisotropicSamp : SHADER_REG_SAMP_ANISOTROPIC;
SamplerState linearPointSamp : SHADER_REG_SAMP_LINEAR_POINT;

float3 GetBodyNormal(float2 tex)
{
    float3 ori_tex = normalTex.Sample(linearPointSamp, tex).xyz;
    return (ori_tex * 2 - 1);
}
float3 Refract(float3 d, float3 n, float i)
{
    float3 newD = normalize(d - n);
    return Lerp(d, newD, saturate(i - 1));
}

float3 ComputeTransparency(float3 wPos, float3 normal, float3 look)
{
    float3 rtex3d = Refract(look, normal, REFRACTION_INDEX_GLASS - 0.02f);
    float3 gtex3d = Refract(look, normal, REFRACTION_INDEX_GLASS);
    float3 btex3d = Refract(look, normal, REFRACTION_INDEX_GLASS + 0.02f);
    return float3(
        cmTex.SampleLevel(pointSamp, rtex3d, 0).r,
        cmTex.SampleLevel(pointSamp, gtex3d, 0).g,
        cmTex.SampleLevel(pointSamp, btex3d, 0).b);
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
    float ssao = ssaoTex.SampleLevel(pointSamp, viewUV, 0).r;
    ssao = ssao * ssao;
    ambient *= ssao;
    
    return float4(cmTex.Sample(pointSamp, wNormal));
    float3 transp = ComputeTransparency(input.wPos, wNormal, look);
    return float4(transp, 1);
    
    float3 light = specular + diffuse + ambient;
    
    float3 tex = diffuseTex.Sample(anisotropicSamp, input.tex).xyz;
    
    float shadowFactor = max(DirectionalLightOpaqueShadowFactor(wNormal, d_Dir[0].xyz, input.wPos), DirectionalLightTranspShadowFactor(wNormal, d_Dir[0].xyz, input.wPos));
    
    float3 color = 0;
    
    //debug remove
    diffuse *= (1 - shadowFactor);
    specular *= (1 - shadowFactor);
    color = diffuse + specular + ambient;
    return float4(color, 1);
    
    return float4(color, mDiffuse.w);
}