#include "stdafx.h"
#include "Tile.h"
#include "Object.h"
#include "Transform.h"
#include "TextureMgr.h"
#include "Shader.h"
#include "ShaderReg.h"
#include "Cylinder.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "ShaderFormat.h"

Tile::Tile(unsigned int id, std::shared_ptr<Shape> shape, std::shared_ptr<Shape> lodShape)
	:Object("Tile", shape, lodShape,
		"StandardVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts),
		"","","",
		"StandardPS.cso",2),id(id), fallingSpeed(fminf(Rand01()+0.5f,1)*20), isIndicator(false)
{
	vs->AddCB(0, 1, sizeof(SHADER_STD_TRANSF));
	ps->AddCB(SHADER_REG_CB_MATERIAL, 1, sizeof(SHADER_MATERIAL));
	SHADER_MATERIAL material(XMFLOAT3(0.7, 0.7, 0.7), 0, XMFLOAT3(0.5, 0.5, 0.5), XMFLOAT3(0.8, 0.8, 0.8));
	ps->WriteCB(SHADER_REG_CB_MATERIAL, &material);

	TextureMgr::Instance()->Load("tileNormal", "Data\\Texture\\wood_normal.jpg");
	TextureMgr::Instance()->Load("tile", "Data\\Texture\\wood.jpg");
	TextureMgr::Instance()->Load("tileLod", "Data\\Texture\\wood_lod.jpg");
	TextureMgr::Instance()->Load("tileRgh", "Data\\Texture\\wood_rgh.jpg");
	TextureMgr::Instance()->Load("tileLodRgh", "Data\\Texture\\wood_lod_rgh.jpg");
	TextureMgr::Instance()->Load("tileMetal", "Data\\Texture\\wood_metal.png");

	ps->AddSRV(SHADER_REG_SRV_DIFFUSE, 1);
	ps->AddSRV(SHADER_REG_SRV_NORMAL, 1);
	ps->AddSRV(SHADER_REG_SRV_ROUGHNESS, 1);
	ps->AddSRV(SHADER_REG_SRV_METALLIC, 1);
	ps->WriteSRV(SHADER_REG_SRV_DIFFUSE, TextureMgr::Instance()->Get("tile")); 
	ps->WriteSRV(SHADER_REG_SRV_NORMAL, TextureMgr::Instance()->Get("tileNormal"));
	ps->WriteSRV(SHADER_REG_SRV_ROUGHNESS, TextureMgr::Instance()->Get("tileRgh"));
	ps->WriteSRV(SHADER_REG_SRV_METALLIC, TextureMgr::Instance()->Get("tileMetal"));

	lodPs = new PShader("Std2PS.cso");
	lodPs->AddCB(SHADER_REG_CB_MATERIAL, 1, sizeof(SHADER_MATERIAL));
	lodPs->WriteCB(SHADER_REG_CB_MATERIAL, &material);
	lodPs->AddSRV(SHADER_REG_SRV_DIFFUSE, 1);
	lodPs->AddSRV(SHADER_REG_SRV_ROUGHNESS, 1);
	lodPs->AddSRV(SHADER_REG_SRV_METALLIC, 1);
	lodPs->WriteSRV(SHADER_REG_SRV_DIFFUSE, TextureMgr::Instance()->Get("tileLod"));
	lodPs->WriteSRV(SHADER_REG_SRV_ROUGHNESS, TextureMgr::Instance()->Get("tileLodRgh"));
	lodPs->WriteSRV(SHADER_REG_SRV_METALLIC, TextureMgr::Instance()->Get("tileMetal"));
	
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

Tile::~Tile()
{
	delete lodPs;
}

void Tile::Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth) const
{
	if (!enabled || !show)
		return;

	if (IsInsideFrustum(frustum))
	{
		if (sceneDepth == 0)
		{
			if (isIndicator)
			{
				XMMATRIX wvp = transform->WorldMatrix() * vp;
				vs->WriteCB(0, &wvp);
				Object::Render();
			}
			else // first depth
			{
				const SHADER_STD_TRANSF STransformation(transform->WorldMatrix(), vp);

				vs->WriteCB(0, &STransformation);
				Object::Render();
			}
		}
		else
		{
			assert(sceneDepth <= 1);

			if (!isIndicator) // second depth (deepest)
			{
				const SHADER_STD_TRANSF STransformation(transform->WorldMatrix(), vp);

				vs->WriteCB(0, &STransformation);
				vs->Apply();
				DX_DContext->HSSetShader(nullptr, nullptr, NULL);
				DX_DContext->DSSetShader(nullptr, nullptr, NULL);
				DX_DContext->GSSetShader(nullptr, nullptr, NULL);
				lodPs->Apply();

				blendState->Apply();
				dsState->Apply();
				rsState->Apply();

				lodShape->Apply();
			}
		}
	}
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
