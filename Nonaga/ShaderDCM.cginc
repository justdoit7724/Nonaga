#ifndef _SHADER_DCM
#define _SHADER_DCM

#include "ShaderInfo.cginc"
#include "ShaderReg.cginc"
#include "ShaderSampPoint.cginc"
#include "ShaderSampLinear.cginc"

#define REFRACTION_INDEX_GLASS 1.2

TextureCube cmTex : SHADER_REG_SRV_CM;
TextureCube dcmTex : SHADER_REG_SRV_DCM;


float3 ComputeTransparency(float3 wPos, float3 normal, float3 look)
{
    float3 rtex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 1.01f));
    float3 gtex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 1));
    float3 btex3d = Lerp(look, normalize(look - normal), saturate(REFRACTION_INDEX_GLASS - 0.99));
    
    float3 cm = float3(
    cmTex.Sample(linearSamp, rtex3d).r,
    cmTex.Sample(linearSamp, gtex3d).g,
    cmTex.Sample(linearSamp, btex3d).b);
    
    float4 dcmr = dcmTex.Sample(linearSamp, rtex3d);
    float4 dcmg = dcmTex.Sample(linearSamp, gtex3d);
    float4 dcmb = dcmTex.Sample(linearSamp, btex3d);
    
    return float3(
    lerp(cm.r, dcmr.r, dcmr.w),
    lerp(cm.g, dcmg.g, dcmg.w),
    lerp(cm.b, dcmb.b, dcmb.w));
}
float3 ComputeReflect(float3 wNormal, float3 look)
{
    float3 reflDir = reflect(look, wNormal);
    
    float3 cm = cmTex.Sample(linearSamp, reflDir).xyz;
    float4 dcm = dcmTex.Sample(linearSamp, reflDir);
    return Lerp(cm, dcm.xyz, dcm.w);
}

#endif