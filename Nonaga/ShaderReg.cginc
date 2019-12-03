
#ifndef _SHADER_REG
#define _SHADER_REG

#define SHADER_REG_CB_DIRLIGHT register(b0)
#define SHADER_REG_CB_POINTLIGHT register(b1)
#define SHADER_REG_CB_SPOTLIGHT register(b2)
#define SHADER_REG_CB_EYE register(b3)
#define SHADER_REG_CB_MATERIAL register(b4)
#define SHADER_REG_CB_LIGHTVP register(b5)

#define SHADER_REG_SRV_CM register(t0)
#define SHADER_REG_SRV_DIFFUSE register(t1)
#define SHADER_REG_SRV_NORMAL register(t2)
#define SHADER_REG_SRV_SSAO register(t3)
#define SHADER_REG_SRV_SHADOW register(t4)
#define SHADER_REG_SRV_SHADOW_TRANSP register(t5)

#define SHADER_REG_SAMP_POINT register(s0)
#define SHADER_REG_SAMP_LINEAR_POINT  register(s3)
#define SHADER_REG_SAMP_ANISOTROPIC register(s4)
#define SHADER_REG_SAMP_CMP_POINT register(s5)

#endif