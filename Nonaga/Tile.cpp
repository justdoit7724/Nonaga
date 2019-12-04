#include "Tile.h"
#include "Object.h"
#include "Transform.h"
#include "TextureMgr.h"
#include "Shader.h"
#include "ShaderReg.h"
#include "Cylinder.h"
#include "ShaderFormat.h"

Tile::Tile(unsigned int id, std::shared_ptr<Shape> shape, std::shared_ptr<Shape> lodShape)
	:Object("Tile", shape, lodShape,
		"StandardVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts),
		"","","",
		"StandardPS.cso",2),id(id), fallingSpeed(fminf(Rand01()+0.5f,1)*100), isIndicator(false)
{
	vs->AddCB(0, 1, sizeof(SHADER_STD_TRANSF));
	ps->AddCB(SHADER_REG_CB_MATERIAL, 1, sizeof(SHADER_MATERIAL));
	ps->WriteCB(SHADER_REG_CB_MATERIAL, &SHADER_MATERIAL(XMFLOAT3(0.7, 0.7, 0.7), 0, XMFLOAT3(0.5, 0.5, 0.5), XMFLOAT3(0.8, 0.8, 0.8), 40, 0.06f));

	TextureMgr::Instance()->Load("tileNormal", "Data\\Texture\\wood_normal.jpg");
	TextureMgr::Instance()->Load("tile", "Data\\Texture\\wood.jpg");
	ps->AddSRV(SHADER_REG_SRV_DIFFUSE, 1);
	ps->AddSRV(SHADER_REG_SRV_NORMAL, 1);
	ps->WriteSRV(SHADER_REG_SRV_DIFFUSE, TextureMgr::Instance()->Get("tile")); 
	ps->WriteSRV(SHADER_REG_SRV_NORMAL, TextureMgr::Instance()->Get("tileNormal"));
	
	transform->SetScale(10, 1, 10);
}

Tile::Tile(bool isRed, std::shared_ptr<Shape> shape)
	:Object("Tile", shape, shape,
		"IndicatorVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts),
		"","", "",
		"IndicatorPS.cso", 2), isIndicator(true)
{
	TextureMgr::Instance()->Load("normal", "Data\\Texture\\default_normal.png");
	TextureMgr::Instance()->Load("red", "Data\\Texture\\red_light.png");
	TextureMgr::Instance()->Load("green", "Data\\Texture\\green_light.png");

	vs->AddCB(0, 1, sizeof(XMMATRIX));

	ps->AddSRV(SHADER_REG_SRV_DIFFUSE, 1);
	ps->AddSRV(SHADER_REG_SRV_NORMAL, 1);
	ps->WriteSRV(SHADER_REG_SRV_DIFFUSE, TextureMgr::Instance()->Get(isRed ? "red" : "green"));
	ps->WriteSRV(SHADER_REG_SRV_NORMAL, TextureMgr::Instance()->Get("normal"));

	transform->SetScale(10, 1, 10);
}

void Tile::Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth) const
{
	if (!enabled || !show)
		return;

	if (IsInsideFrustum(frustum))
	{
		if (isIndicator)
		{
			XMMATRIX wvp = transform->WorldMatrix() * vp;
			vs->WriteCB(0, &wvp);
		}
		else
		{
			const SHADER_STD_TRANSF STransformation(transform->WorldMatrix(), vp);

			vs->WriteCB(0, &STransformation);
		}

		Object::Render();
	}
}

void Tile::RenderGeom() const
{
	if (id == -1)
		return;

	shape->Apply();
}

void Tile::UpdateBound()
{
	XMFLOAT3 boundlMinPt;
	XMFLOAT3 boundlMaxPt;
	shape->GetLBound(&boundlMinPt, &boundlMaxPt);
	XMMATRIX world = transform->WorldMatrix();
	XMFLOAT3 wMinPt = Multiply(boundlMinPt, world);
	XMFLOAT3 wMaxPt = Multiply(boundlMaxPt, world);
	bound.p = transform->GetPos();
	bound.rad = fabsf(wMinPt.x - wMaxPt.x) * 0.5f;
}

void Tile::Move(int toId)
{
	id = toId;
}
