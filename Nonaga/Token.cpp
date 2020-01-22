#include "stdafx.h"
#include "Token.h"
#include "Transform.h"
#include "TextureMgr.h"
#include "ShaderFormat.h"
#include "Shader.h"
#include "Shape.h"
#include "Camera.h"
#include "ShaderReg.h"
#include "CameraMgr.h"
#include "Game_info.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "Scene.h"
#include "TaskMgr.h"

Camera* Token::captureCam = nullptr;

Token::Token(std::shared_ptr<Shape> shape, std::shared_ptr<Shape> lodShape, Scene* environemnt, unsigned int id, bool p1)
	:Object("Token",
		shape, lodShape,
		"StdDisplacementVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts),
		"StdDisplacementHS.cso", "StdDisplacementDS.cso", "",
		"StandardPS.cso", Z_ORDER_STANDARD),
		id(id), isP1(p1), environment(environemnt), isIndicator(false), fallingSpeed(fminf(Rand01() + 0.5f, 1) * 15)
{
	TextureMgr::Instance()->Load("token", "Data\\Model\\Token\\pawnDark.png");
	TextureMgr::Instance()->Load("tokenLod", "Data\\Model\\Token\\pawnDark_lod.png");
	TextureMgr::Instance()->Load("tokenTransp", "Data\\Texture\\white.png");
	TextureMgr::Instance()->Load("tokenNormal", "Data\\Model\\Token\\pawn_normal.png");
	TextureMgr::Instance()->Load("tokenDP", "Data\\Model\\Token\\pawn_displace.png");
	TextureMgr::Instance()->Load("tokenRgh", "Data\\Model\\Token\\pawn_rgh.jpg");
	TextureMgr::Instance()->Load("token2Rgh", "Data\\Model\\Token\\pawn2_rgh.jpg");
	TextureMgr::Instance()->Load("tokenLodRgh", "Data\\Model\\Token\\pawn_lod_rgh.jpg");
	TextureMgr::Instance()->Load("token2LodRgh", "Data\\Model\\Token\\pawn2_lod_rgh.jpg");
	TextureMgr::Instance()->Load("token1Metal", "Data\\Model\\Token\\pawn_metal.png");
	TextureMgr::Instance()->Load("token2Metal", "Data\\Model\\Token\\pawn2_metal.png");
	std::vector<std::string> defaultCMs;
	defaultCMs.push_back("Data\\Texture\\default.png");
	defaultCMs.push_back("Data\\Texture\\default.png");
	defaultCMs.push_back("Data\\Texture\\default.png");
	defaultCMs.push_back("Data\\Texture\\default.png");
	defaultCMs.push_back("Data\\Texture\\default.png");
	defaultCMs.push_back("Data\\Texture\\default.png");
	TextureMgr::Instance()->LoadCM("defaultCM", defaultCMs);

	if (captureCam == nullptr)
	{
		captureCam = new Camera(FRAME_KIND_PERSPECTIVE, SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 100, XM_PIDIV2, 1,false);
	}
	shape->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	vs->AddCB(0, 1, sizeof(SHADER_STD_TRANSF));
	vs->AddCB(1, 1, sizeof(XMFLOAT4));
	ds->AddCB(0, 1, sizeof(XMMATRIX));
	ds->AddCB(1, 1, sizeof(float));
	float dpScale = 0.2f;
	ds->WriteCB(1, &dpScale);
	ds->AddSRV(0, 1);
	ds->WriteSRV(0, TextureMgr::Instance()->Get("tokenDP"));
	D3D11_SAMPLER_DESC samp_desc;
	ZeroMemory(&samp_desc, sizeof(D3D11_SAMPLER_DESC));
	samp_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samp_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samp_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samp_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samp_desc.MinLOD = 0;
	samp_desc.MaxLOD = D3D11_FLOAT32_MAX;
	ds->AddSamp(0, 1, &samp_desc);
	ps->AddCB(SHADER_REG_CB_MATERIAL, 1, sizeof(SHADER_MATERIAL));
	SHADER_MATERIAL material(XMFLOAT3(0.7, 0.7, 0.7), isP1? 0:0.9, XMFLOAT3(0.5, 0.5, 0.5), XMFLOAT3(0.8, 0.8, 0.8));
	

	ps->WriteCB(SHADER_REG_CB_MATERIAL, &material);
	ps->AddSRV(SHADER_REG_SRV_DCM, 1);
	ps->WriteSRV(SHADER_REG_SRV_DCM, TextureMgr::Instance()->Get("defaultCM"));
	ps->AddSRV(SHADER_REG_SRV_DIFFUSE, 1);
	ps->AddSRV(SHADER_REG_SRV_NORMAL, 1);
	ps->AddSRV(SHADER_REG_SRV_ROUGHNESS, 1);
	ps->AddSRV(SHADER_REG_SRV_METALLIC, 1);
	ps->WriteSRV(SHADER_REG_SRV_DIFFUSE, TextureMgr::Instance()->Get(isP1? "token" : "tokenTransp"));
	ps->WriteSRV(SHADER_REG_SRV_NORMAL, TextureMgr::Instance()->Get("tokenNormal"));
	ps->WriteSRV(SHADER_REG_SRV_ROUGHNESS, TextureMgr::Instance()->Get(isP1?"tokenRgh":"token2Rgh"));
	ps->WriteSRV(SHADER_REG_SRV_METALLIC, TextureMgr::Instance()->Get(isP1?"token1Metal":"token2Metal"));

	lodVs = new VShader("StandardVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts));
	lodVs->AddCB(0, 1, sizeof(SHADER_STD_TRANSF));
	lodPs = new PShader("StandardOptPS.cso");
	lodPs->AddCB(SHADER_REG_CB_MATERIAL, 1, sizeof(SHADER_MATERIAL));
	lodPs->WriteCB(SHADER_REG_CB_MATERIAL, &material);
	lodPs->AddSRV(SHADER_REG_SRV_DIFFUSE, 1);
	lodPs->AddSRV(SHADER_REG_SRV_ROUGHNESS, 1);
	lodPs->AddSRV(SHADER_REG_SRV_METALLIC, 1);
	lodPs->WriteSRV(SHADER_REG_SRV_DIFFUSE, TextureMgr::Instance()->Get("tokenLod"));
	lodPs->WriteSRV(SHADER_REG_SRV_ROUGHNESS, TextureMgr::Instance()->Get(isP1?"tokenLodRgh":"token2LodRgh"));
	lodPs->WriteSRV(SHADER_REG_SRV_METALLIC, TextureMgr::Instance()->Get(isP1 ? "token1Metal" : "token2Metal"));

	D3D11_TEXTURE2D_DESC capture_desc;
	capture_desc.Width = SCREEN_WIDTH;
	capture_desc.Height = SCREEN_HEIGHT;
	capture_desc.ArraySize = 6;
	capture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	capture_desc.CPUAccessFlags = 0;
	capture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	capture_desc.MipLevels = 1;
	capture_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	capture_desc.SampleDesc = { 1,0 };
	capture_desc.Usage = D3D11_USAGE_DEFAULT;
	ComPtr<ID3D11Texture2D> captureTex;
	r_assert(
		DX_Device->CreateTexture2D(&capture_desc, nullptr, captureTex.GetAddressOf())
	);
	D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
	rtv_desc.Format = capture_desc.Format;
	rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtv_desc.Texture2DArray.MipSlice = 0;
	rtv_desc.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < 6; ++i)
	{
		rtv_desc.Texture2DArray.FirstArraySlice = i;
		r_assert(
			DX_Device->CreateRenderTargetView(captureTex.Get(), &rtv_desc, captureRTV[i].GetAddressOf())
		);
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = capture_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srv_desc.TextureCube.MostDetailedMip = 0;
	srv_desc.TextureCube.MipLevels = 1;
	r_assert(
		DX_Device->CreateShaderResourceView(captureTex.Get(), &srv_desc, captureSRV.GetAddressOf())
	);

	D3D11_TEXTURE2D_DESC ds_desc;
	ds_desc.ArraySize = 1;
	ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ds_desc.CPUAccessFlags = 0;
	ds_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ds_desc.Width = capture_desc.Width;
	ds_desc.Height = capture_desc.Height;
	ds_desc.MipLevels = 1;
	ds_desc.MiscFlags = 0;
	ds_desc.SampleDesc = { 1,0 };
	ds_desc.Usage = D3D11_USAGE_DEFAULT;
	ComPtr<ID3D11Texture2D> dsTex;
	r_assert(
		DX_Device->CreateTexture2D(&ds_desc, nullptr, dsTex.GetAddressOf())
	);
	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Flags = 0;
	dsv_desc.Format = ds_desc.Format;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Texture2D.MipSlice = 0;
	r_assert(
		DX_Device->CreateDepthStencilView(dsTex.Get(), &dsv_desc, captureDSV.GetAddressOf())
	);


	transform->SetScale(0.2, 0.2, 0.2);
}

Token::Token(std::shared_ptr<Shape> shape, bool isRed)
	:Object("Token", shape, shape,
		"IndicatorVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts),
		"", "", "",
		"IndicatorPS.cso",Z_ORDER_STANDARD), isIndicator(true), isP1(NULL)
{
	vs->AddCB(0, 1, sizeof(XMMATRIX));

	TextureMgr::Instance()->Load("red", "Data\\Texture\\red_light.png");
	TextureMgr::Instance()->Load("green", "Data\\Texture\\green_light.png");
	ps->AddSRV(SHADER_REG_SRV_DIFFUSE, 1);
	ps->WriteSRV(SHADER_REG_SRV_DIFFUSE, TextureMgr::Instance()->Get(isRed ? "red":"green"));

	transform->SetScale(0.2, 0.2, 0.2);
	enabled = false;
}

Token::~Token()
{
	if (captureCam)
	{
		delete captureCam;
		captureCam = nullptr;
	}

	delete lodVs;
	delete lodPs;
}


void Token::Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth) const
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
			else
			{
				const SHADER_STD_TRANSF STransformation(transform->WorldMatrix(), vp);

				vs->WriteCB(0, &STransformation);

				vs->WriteCB(1, &CameraMgr::Instance()->Main()->transform->GetPos());
				ds->WriteCB(0, &(STransformation.vp));

				if (isP1)
				{
					Object::Render();
				}
				else
				{
					if(!TaskMgr::Instance()->HasTaskFrom(this))
						DrawDCM(sceneDepth + 1);

					ps->WriteSRV(SHADER_REG_SRV_DCM, captureSRV.Get());

					Object::Render();

					ID3D11ShaderResourceView* nullSRV = nullptr;
					DX_DContext->PSSetShaderResources(SHADER_REG_SRV_DCM, 1, &nullSRV);
				}
			}
		}
		else
		{
			if (!isIndicator)
			{
				const SHADER_STD_TRANSF STransformation(transform->WorldMatrix(), vp);

				lodVs->WriteCB(0, &STransformation);
				lodVs->Apply();
				DX_DContext->HSSetShader(nullptr, nullptr, NULL);
				DX_DContext->DSSetShader(nullptr, nullptr, NULL);
				DX_DContext->GSSetShader(nullptr, nullptr, NULL);
				lodPs->Apply();

				blendState->Apply();
				dsState->Apply();
				rsState->Apply();

				lodShape->Apply();
			}

			// skip for deeper rendering
		}
	}
}

void Token::Move(int toId)
{
	id = toId;
}

void Token::Capture(XMFLOAT3 forward, XMFLOAT3 up, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, UINT sceneDepth)const
{
	captureCam->transform->SetRot(forward, up);
	captureCam->Update();

	TaskMgr::Instance()->Add(this, rtv, dsv, environment, captureCam->VMat() * captureCam->StdProjMat(), captureCam->GetFrustum(), sceneDepth);
}

void Token::DrawDCM(UINT sceneDepth)const
{
	if (sceneDepth >= 2)
		return;

	captureCam->transform->SetTranslation(transform->GetPos());

	Capture(RIGHT, UP, captureRTV[0].Get(), captureDSV.Get(), sceneDepth);
	Capture(-RIGHT, UP, captureRTV[1].Get(), captureDSV.Get(), sceneDepth);
	Capture(UP, -FORWARD, captureRTV[2].Get(), captureDSV.Get(), sceneDepth);
	Capture(-UP, FORWARD, captureRTV[3].Get(), captureDSV.Get(), sceneDepth);
	Capture(FORWARD, UP, captureRTV[4].Get(), captureDSV.Get(), sceneDepth);
	Capture(-FORWARD, UP, captureRTV[5].Get(), captureDSV.Get(), sceneDepth);
}


