
#include "ShaderInfo.cginc"
#include "ShaderReg.cginc"

Texture2D diffuseTex : SHADER_REG_SRV_DIFFUSE;
SamplerState pointSamp : SHADER_REG_SAMP_POINT;

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 ui : TEXCOORD0;
};
float4 main(PS_INPUT input) : SV_Target
{
    float3 surface = diffuseTex.SampleLevel(pointSamp, input.ui, 0);
    return float4(surface, 0.4f);
}