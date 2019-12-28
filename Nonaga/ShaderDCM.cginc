#ifndef _SHADER_DCM
#define _SHADER_DCM

#include "ShaderInfo.cginc"
#include "ShaderReg.cginc"
#include "ShaderSampPoint.cginc"
#include "ShaderSampLinear.cginc"


//for optimizing reason, 
//separate static cm/ dynamic cm, so i don't have to draw skybox everytime when mapping dcm
TextureCube cmTex : SHADER_REG_SRV_CM;
TextureCube dcmTex : SHADER_REG_SRV_DCM;


float3 ComputeTransparency(float3 wPos, float3 normal, float3 look)
{ 
    /*
    exp_transparency
    get color info from dynamic cube map using look dir.
    give a tweak depends on world normal of current pixel, so that more reality.
    */
    float3 mLook = Lerp(look, normalize(look - normal), 0.20f);
    
    /*
    exp_chromatic_aberration
    divide look dir into 3(Red,Green,Blue) and give slight constant sweak.
    */
    float3 tex3d_red = Lerp(look, normalize(look - normal), 0.19f);
    float3 tex3d_green = mLook;
    float3 tex3d_blue = Lerp(look, normalize(look - normal), 0.21f);
    
    // static cube map
    float3 cm = float3(
    cmTex.Sample(linearSamp, tex3d_red).r,
    cmTex.Sample(linearSamp, tex3d_green).g,
    cmTex.Sample(linearSamp, tex3d_blue).b);
    
    // dynamic cube map - w(1 if drawn)
    float4 dcmr = dcmTex.Sample(linearSamp, tex3d_red);
    float4 dcmg = dcmTex.Sample(linearSamp, tex3d_green);
    float4 dcmb = dcmTex.Sample(linearSamp, tex3d_blue);
    
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