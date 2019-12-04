#include "Skybox.h"
#include "Transform.h"
#include "Shader.h"
#include "Sphere.h"
#include "RasterizerState.h"
#include "Camera.h"
#include "DepthStencilState.h"
#include "ShaderFormat.h"
#include "CameraMgr.h"
#include "TextureMgr.h"
#include "ShaderReg.h"

Skybox::Skybox(ID3D11ShaderResourceView* srv)
	:Object("Skybox", std::make_shared<Sphere>(0),nullptr,
		"CMVS.cso", Std_ILayouts,ARRAYSIZE(Std_ILayouts),
		"","","",
		"CMPS.cso",Z_ORDER_STANDARD)
{ 
	transform->SetScale(300, 300, 300);
	
	vs->AddCB(0, 1, sizeof(SHADER_STD_TRANSF));
	delete rsState;
	D3D11_RASTERIZER_DESC rs_desc;
	ZeroMemory(&rs_desc, sizeof(D3D11_RASTERIZER_DESC));
	rs_desc.CullMode = D3D11_CULL_FRONT;
	rs_desc.FillMode = D3D11_FILL_SOLID;
	rs_desc.FrontCounterClockwise = false;
	rsState = new RasterizerState(&rs_desc);

	ID3D11ShaderResourceView* cmSRV = TextureMgr::Instance()->Get("cm");
	DX_DContext->PSSetShaderResources(SHADER_REG_SRV_CM, 1, &cmSRV);
}

void Skybox::Update()
{
	transform->SetTranslation(CameraMgr::Instance()->Main()->transform->GetPos());
}


void Skybox::Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth) const
{
	if (sceneDepth != 0)
		return;

	Object::Render(vp,frustum, sceneDepth);
}


