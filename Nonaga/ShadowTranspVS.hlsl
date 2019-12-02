#include "ShaderVertex.cginc"

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 wNormal : TEXCOORD0;
    float pDist : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float2 ui : TEXCOORD3;
};

cbuffer CB_TRANSFORMATION : register(b0)
{
    float4x4 wvpMat;
    float4x4 normalMat;
}

VS_OUTPUT main(STD_VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(wvpMat, float4(input.pos, 1));
    output.pDist = output.pos.z / output.pos.w;
    output.wNormal = mul((float3x3)normalMat, input.normal);
    output.tangent = input.tangent;
    output.ui = input.tex;
    return output;
}