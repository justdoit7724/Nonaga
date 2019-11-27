#include "Object.h"
#include "ShaderFormat.h"
#include "Camera.h"
#include "ShaderReg.h"
#include "TextureMgr.h"
#include "Transform.h"
#include "Shader.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "Shape.h"
#include "CameraMgr.h"

#include "Debugging.h"

//fundamental elements
Object::Object(Shape* shape, std::string sVS, const D3D11_INPUT_ELEMENT_DESC* iLayouts, UINT layoutCount, std::string sHS, std::string sDS, std::string sGS, std::string sPS,int zOrder)
	:shape(shape), zOrder(zOrder)
{
	transform = new Transform();
	vs = new VShader(sVS, iLayouts, layoutCount);
	hs = new HShader(sHS);
	ds = new DShader(sDS);
	gs = new GShader(sGS);
	ps = new PShader(sPS);

	blendState = new BlendState(nullptr);
	dsState = new DepthStencilState(nullptr);
	rsState = new RasterizerState(nullptr);
}

//standard elements
Object::Object(Shape* shape, ID3D11ShaderResourceView* diffSRV, ID3D11ShaderResourceView* normalSRV)
	:zOrder(Z_ORDER_STANDARD), shape(shape)
{
	transform = new Transform();
	vs = new VShader("StandardVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts));
	hs = new HShader();
	ds = new DShader();
	gs = new GShader();
	ps = new PShader("StandardPS.cso");

	vs->AddCB(0, 1, sizeof(SHADER_STD_TRANSF));
	ps->AddCB(SHADER_REG_PS_CB_MATERIAL, 1, sizeof(SHADER_MATERIAL));
	ps->WriteCB(SHADER_REG_PS_CB_MATERIAL,&SHADER_MATERIAL(XMFLOAT3(0.7,0.7,0.7), 0.2, XMFLOAT3(0.2, 0.2, 0.2), XMFLOAT3(0.8, 0.8, 0.8), 32));
	
	D3D11_SAMPLER_DESC body_desc;
	ZeroMemory(&body_desc, sizeof(D3D11_SAMPLER_DESC));
	body_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	body_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	body_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	body_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	body_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	body_desc.MinLOD = 0;
	body_desc.MaxLOD = D3D11_FLOAT32_MAX;
	ps->AddSamp(SHADER_REG_PS_SAMP_TEX, 1, &body_desc);
	ps->AddSRV(SHADER_REG_PS_SRV_DIFFUSE, 1);
	ps->AddSRV(SHADER_REG_PS_SRV_NORMAL, 1);
	ps->WriteSRV(SHADER_REG_PS_SRV_DIFFUSE, diffSRV);
	ps->WriteSRV(SHADER_REG_PS_SRV_NORMAL, normalSRV);

	blendState = new BlendState(nullptr);
	dsState = new DepthStencilState(nullptr);
	rsState = new RasterizerState(nullptr);
}

Object::~Object()
{
	delete transform;
	delete shape;
	delete vs;
	delete hs;
	delete ds;
	delete gs;
	delete ps;

	delete dsState;
	delete blendState;
	delete rsState;
}

void Object::Update(const XMMATRIX& parentWorld)
{
	XMMATRIX curWorld = transform->WorldMatrix() * parentWorld;
	for (auto child : children)
		child->Update(curWorld);

	if (!enabled)
		return;

	UpdateBound(parentWorld);

}

void Object::UpdateBound(const XMMATRIX& parentWorld)
{
	XMFLOAT3 boundlMinPt;
	XMFLOAT3 boundlMaxPt;
	shape->GetLBound(&boundlMinPt, &boundlMaxPt);
	XMMATRIX world = transform->WorldMatrix()*parentWorld;
	XMFLOAT3 wMinPt = Multiply(boundlMinPt, world);
	XMFLOAT3 wMaxPt = Multiply(boundlMaxPt, world);
	bound.p = Multiply(transform->GetPos(), parentWorld);
	bound.rad = Length(wMinPt - wMaxPt) * 0.5f;
}

Object::Object()
	:zOrder(Z_ORDER_STANDARD)
{
}

void Object::Render()const
{
	vs->Apply();
	hs->Apply();
	ds->Apply();
	gs->Apply();
	ps->Apply();

	dsState->Apply();
	blendState->Apply();
	rsState->Apply();

	shape->Apply();
}
void Object::Render(const XMMATRIX& parentWorld, const XMMATRIX& vp, const Frustum* frustum, UINT sceneDepth) const
{
	if (frustum == nullptr || IsInsideFrustum(frustum))
	{
		XMMATRIX curWorld = transform->WorldMatrix() * parentWorld;

		for (auto child : children)
		{
			child->Render(curWorld, vp, frustum, sceneDepth);
		}

		if (!enabled || !show)
			return;

		const SHADER_STD_TRANSF STransformation(curWorld, vp);

		vs->WriteCB(0, &STransformation);

		Render();
	}
}

void Object::RenderGeom() const
{
	if (!enabled || !show)
		return;

	shape->Apply();
}

bool Object::IsInsideFrustum(const Frustum* frustum) const
{
	return (
		IntersectInPlaneSphere(frustum->front, bound) &&
		IntersectInPlaneSphere(frustum->back, bound) &&
		IntersectInPlaneSphere(frustum->right, bound) &&
		IntersectInPlaneSphere(frustum->left, bound) &&
		IntersectInPlaneSphere(frustum->top, bound) &&
		IntersectInPlaneSphere(frustum->bottom, bound));
}

bool Object::IsPicking(const Geometrics::Ray ray) const
{
	return Geometrics::IntersectRaySphere(ray, bound);
}

void Object::Visualize()
{
	if(IsInsideFrustum(CameraMgr::Instance()->Main()->GetFrustum()))
		Debugging::Instance()->Mark(bound.p, bound.rad, Colors::LightGreen);
}

void Object::AddChildren(Object* obj)
{
	children.push_back(obj);
}

void Object::GetChildren(std::vector<const Object*>& rObj)const
{
	for (auto c : children)
		rObj.push_back(c);
}

