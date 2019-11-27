
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
Texture2D metalicTex : register(t5);
Texture2D roughnessTex : register(t6);
//...

SamplerState cmSamp : SHADER_REG_PS_SAMP_CM;
SamplerState samp : SHADER_REG_PS_SAMP_TEX;

float3 GetBodyNormal(float2 tex)
{
    float3 ori_tex = normalTex.Sample(samp, tex).xyz;
    return (ori_tex * 2 - 1);
}
float3 ComputeMetalic(float3 color, float3 normal, float3 look, float2 uv)
{
    float3 cm = cmTex.Sample(cmSamp, reflect(look, normal)).xyz;
    float metalic = metalicTex.SampleLevel(samp, uv, 0).x;
    
    return Lerp(color, cm, metalic);
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
    
    //debug
    float3 cmDir = reflect(look, wNormal);
    return float4(cmTex.Sample(cmSamp, cmDir).xyz, 1);
    
    float4 ambient = 0;
    float4 diffuse = 0;
    float4 specular = 0;
    float4 reflection = 0;
    float4 A, D, S;
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

    float3 color = 0;
    float3 light = specular.xyz + diffuse.xyz + ambient.xyz;
    
    float3 tex = diffuseTex.Sample(samp, input.tex).xyz;
    
    float shadowFactor = DirectionalLightShadowFactor(wNormal, d_Dir[0].xyz, input.wPos).r;

    float4x4 uvMat = float4x4(
        0.5, 0, 0, 0,
        0, -0.5, 0, 0,
        0, 0, 0, 0,
        0.5, 0.5, 0, 1);
    input.pPos = mul(input.pPos, uvMat);
    float2 viewUV = input.pPos.xy / input.pPos.w;
    float ssao = ssaoTex.SampleLevel(samp, viewUV, 0).r;


    tex = ComputeTransparency(tex, wNormal, look);
    
    color = light * tex;
    
    

    tex = ComputeMetalic(tex, wNormal, look, input.tex);

    float smoothness = 1 - roughnessTex.SampleLevel(samp, input.tex, 0).x;
    specular *= smoothness;
    diffuse = Lerp(diffuse, diffuse * float4(tex, 1), smoothness);
    ambient = Lerp(ambient, ambient * float4(tex, 1), smoothness)*ssao;
    
    color = ambient + diffuse + specular;
    
    return float4(color, mDiffuse.w);
}