struct VS_INPUT
{
    float3 pos : POSITION;
    float noUse : FAR_PLANE_IDX;
    float2 tex : TEXCOORD;
};
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = float4(input.pos, 1);
    output.tex = input.tex;
    return output;
}