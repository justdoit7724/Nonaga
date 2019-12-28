#include "ShaderReg.cginc"

TextureCube cubeMap : SHADER_REG_SRV_CM;
sampler samp : SHADER_REG_SAMP_LINEAR;

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 lPos : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_Target
{
    uint noUse1, noUse2;
    uint mipmapLevel;
    cubeMap.GetDimensions(0, noUse1, noUse2, mipmapLevel);
    //if(mipmapLevel==)
    
    return cubeMap.SampleLevel(samp, normalize(input.lPos), 4);
}