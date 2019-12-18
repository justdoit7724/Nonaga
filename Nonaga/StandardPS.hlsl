
#include "ShaderInfo.cginc"
#include "ShaderLight.cginc"
#include "ShaderShadow.cginc"
#include "ShaderReg.cginc"
#include "ShaderCM.cginc"
#include "ShaderNormal.cginc"
#include "ShaderSampPoint.cginc"
#include "ShaderSampAniso.cginc"
#include "ShaderSSAO.cginc"
#include "ShaderRghMetal.cginc"

cbuffer EYE : SHADER_REG_CB_EYE
{
    float4 eyePos;
};


Texture2D diffuseTex : SHADER_REG_SRV_DIFFUSE;
//...


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
    float3 wNormal = GetBodyNormal(anisotropicSamp, input.tex, input.normal, input.tangent);

    float3 look = normalize(input.wPos-eyePos.xyz);
    
    //surface property
    float roughness = ComputeRoughness(anisotropicSamp, input.tex);
    float metallic = ComputeMetallic(anisotropicSamp, input.tex);
    
    //lights
    float3 ambient = 0;
    float3 diffuse = 0;
    float3 specular = 0;
    ComputeDirectionalLight(wNormal, -look, roughness, ambient, diffuse, specular);

    //surface colors
    float3 tex = diffuseTex.Sample(anisotropicSamp, input.tex).xyz;
    float3 transp = ComputeTransparency(input.wPos, wNormal, look);
    float3 reflec = ComputeReflect(wNormal, look);
    
    //shadow
    float oShadowFactor = DirectionalLightOpaqueShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float2 tShadowFactor = DirectionalLightTranspShadowFactor(wNormal, d_Dir[0].xyz, input.wPos);
    float shadowFactor = saturate(1 - max(oShadowFactor, tShadowFactor.x));
 
    // let's combine all variables 
    specular = specular * (1 - roughness) * shadowFactor + (saturate(0.05f - tShadowFactor.x) * tShadowFactor.y);
    ambient *= tex * ComputeSSAO(pointSamp, input.pPos);
    diffuse *= tex * shadowFactor;
    
    // first lerp between tex light - transparent color
    float3 surface = Lerp(ambient + diffuse, transp, mDiffuse.a * (1 - roughness));
    // second lerp with reflect color
    surface = Lerp(surface, reflec, metallic * (1 - roughness));
    
    return float4(specular + surface, 1);
}