#include "Token.h"
#include "Transform.h"
#include "Cube.h"
#include "TextureMgr.h"
#include "ShaderFormat.h"
#include "Shader.h"
#include "Shape.h"
#include "Camera.h"
#include "ShaderReg.h"
#include "CameraMgr.h"
#include "MeshLoader.h"
#include "Game_info.h"
#include "Scene.h"

Shape* Token::mesh = nullptr;
Camera* Token::captureCam = nullptr;

Token::Token(Scene* environemnt, unsigned int id, bool p1)
	:Object(nullptr,
		"StdDisplacementVS.cso", Std_ILayouts, ARRAYSIZE(Std_ILayouts),
		"StdDisplacementHS.cso", "StdDisplacementDS.cso", "",
		"StandardPS.cso", Z_ORDER_STANDARD),
		id(id), isP1(p1), environment(environemnt), isIndicator(false)
{
	TextureMgr::Instance()->Load("token", "Data\\Model\\Token\\pawn.png");
	TextureMgr::Instance()->Load("tokenNormal", "Data\\Model\\Token\\pawn_normal.png");
	TextureMgr::Instance()->Load("tokenDP", "Data\\Model\\Token\\pawn_displace.png");
	
	if (captureCam == nullptr)
	{
		captureCam = new Camera(FRAME_KIND_PERSPECTIVE, SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 100, XM_PIDIV2, 1);
	}
	if (mesh==nullptr)
	{
		MeshLoader::LoadToken(&mesh);
	}
	shape = mesh;
	shape->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	vs->AddCB(0, 1, sizeof(SHADER_STD_TRANSF));
	vs->AddCB(1, 1, sizeof(XMFLOAT4));
	ds->AddCB(0, 1, sizeof(XMMATRIX));
	ds->AddCB(1, 1, sizeof(float));
	float dpScale = 0.5f;
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
	ps->AddCB(SHADER_REG_PS_CB_MATERIAL, 1, sizeof(SHADER_MATERIAL));
	ps->WriteCB(SHADER_REG_PS_CB_MATERIAL, &SHADER_MATERIAL(XMFLOAT3(0.7, 0.7, 0.7), 1, XMFLOAT3(0.5,0.5,0.5), XMFLOAT3(0.8,0.8,0.8), 16));
	ps->AddSRV(SHADER_REG_PS_SRV_DIFFUSE, 1);
	ps->AddSRV(SHADER_REG_PS_SRV_NORMAL, 1);
	ps->WriteSRV(SHADER_REG_PS_SRV_DIFFUSE, TextureMgr::Instance()->Get("token"));
	ps->WriteSRV(SHADER_REG_PS_SRV_NORMAL, TextureMgr::Instance()->Get("tokenNormal"));
	ps->AddSamp(SHADER_REG_PS_SAMP_TEX, 1, &samp_desc);

	D3D11_TEXTURE2D_DESC capture_desc;
	capture_desc.Width = SCREEN_WIDTH;
	capture_desc.Height = SCREEN_HEIGHT;
	capture_desc.ArraySize = 6;
	capture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	capture_desc.CPUAccessFlags = 0;
	capture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	capture_desc.MipLevels = 0;
	capture_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
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
	srv_desc.TextureCube.MipLevels = -1;
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

	transform->SetScale(0.3, 0.3, 0.3);
}

Token::Token(bool isRed)
	:Object(new Cube(),
		nullptr,nullptr), isIndicator(true)
{
	TextureMgr::Instance()->Load("red", "Data\\Texture\\red_light.png");
	TextureMgr::Instance()->Load("green", "Data\\Texture\\green_light.png");
	ps->WriteSRV(SHADER_REG_PS_SRV_DIFFUSE, TextureMgr::Instance()->Get(isRed ? "red":"green"));
	ps->WriteSRV(SHADER_REG_PS_SRV_DIFFUSE, TextureMgr::Instance()->Get("tokenNormal"));

	transform->SetScale(7, 10, 7);
	enabled = false;
	ps->WriteSRV(SHADER_REG_PS_SRV_DIFFUSE, TextureMgr::Instance()->Get("token"));
	ps->WriteSRV(SHADER_REG_PS_SRV_NORMAL, TextureMgr::Instance()->Get("tokenNormal"));
}

void Token::Render(const XMMATRIX& parentWorld, const XMMATRIX& vp, const Frustum* frustum, UINT sceneDepth) const
{
	if (frustum == nullptr || IsInsideFrustum(frustum))
	{
		const SHADER_STD_TRANSF STransformation(transform->WorldMatrix() * parentWorld, vp);

		vs->WriteCB(0, &STransformation);

		if (!isIndicator)
		{
			if (sceneDepth == 0)
			{
				vs->WriteCB(1, &CameraMgr::Instance()->Main()->transform->GetPos());
				ds->WriteCB(0, &(STransformation.vp));

				// save prev CubeMap
				ID3D11ShaderResourceView* oriCM = nullptr;
				DX_DContext->PSGetShaderResources(SHADER_REG_PS_SRV_CM, 1, &oriCM);

				DrawDCM(sceneDepth + 1);

				Object::Render();

				DX_DContext->PSSetShaderResources(SHADER_REG_PS_SRV_CM, 1, &oriCM);
			}
			else
			{
			}
		}
		else if (sceneDepth == 0)
			Object::Render();
	}
}

void Token::Move(int toId, XMFLOAT3 pos)
{
	id = toId;
	transform->SetTranslation(pos);
}

void Token::Capture(XMFLOAT3 forward, XMFLOAT3 up, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, UINT sceneDepth)const
{
	DX_DContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, NULL);
	DX_DContext->ClearRenderTargetView(rtv, Colors::Red);
	DX_DContext->OMSetRenderTargets(1, &rtv ,dsv);

	captureCam->transform->SetRot(forward, up);
	captureCam->SetView();

	environment->Render(captureCam->VMat() * captureCam->StdProjMat(), captureCam->GetFrustum(), sceneDepth);
}

void Token::DrawDCM(UINT sceneDepth)const
{
	if (sceneDepth >= 2)
		return;

	captureCam->transform->SetTranslation(transform->GetPos());

	ID3D11RenderTargetView* oriRTV;
	ID3D11DepthStencilView* oriDSV;
	DX_DContext->OMGetRenderTargets(1, &oriRTV, &oriDSV);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	DX_DContext->PSSetShaderResources(SHADER_REG_PS_SRV_CM, 1, &nullSRV);

	Capture(RIGHT, UP, captureRTV[0].Get(), captureDSV.Get(), sceneDepth);
	Capture(-RIGHT, UP, captureRTV[1].Get(), captureDSV.Get(), sceneDepth);
	Capture(UP, -FORWARD, captureRTV[2].Get(), captureDSV.Get(), sceneDepth);
	Capture(-UP, FORWARD, captureRTV[3].Get(), captureDSV.Get(), sceneDepth);
	Capture(FORWARD, UP, captureRTV[4].Get(), captureDSV.Get(), sceneDepth);
	Capture(-FORWARD, UP, captureRTV[5].Get(), captureDSV.Get(), sceneDepth);

	DX_DContext->OMSetRenderTargets(1, &oriRTV, oriDSV);

	DX_DContext->PSSetShaderResources(SHADER_REG_PS_SRV_CM, 1, captureSRV.GetAddressOf());
}


