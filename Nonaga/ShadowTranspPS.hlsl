
#include "ShaderReg.cginc"

struct PS_INPUT
{
    float4 pos : SV_Position;
    float3 wNormal : TEXCOORD0;
    float pDist : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float2 ui : TEXCOORD3;
};

Texture2D normalTex : SHADER_REG_SRV_NORMAL;
SamplerState normalSamp : SHADER_REG_SAMP_POINT;

float4 main(PS_INPUT input) : SV_Target
{
    float3 normal = normalize(input.wNormal);
    float3 tangent = normalize(input.tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    float3 bitangent = cross(normal, tangent);
    float3x3 tbn = float3x3(tangent, bitangent, normal);
    
    float3 tNormal = normalTex.Sample(normalSamp, input.ui);
    float3 finalNormal = mul(tNormal * 2 - 1, tbn);
    
    return float4(finalNormal, input.pDist);
}