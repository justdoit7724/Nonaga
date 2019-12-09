#include "stdafx.h"
#include "GamePlayScene.h"
#include "Camera.h"
#include "Game_info.h"
#include "Transform.h"
#include "CameraMgr.h"
#include "SceneMgr.h"
#include "Mouse.h"
#include "Buffer.h"
#include "Light.h"
#include "Nonaga.h"
#include "ShaderReg.h"
#include "ShadowMap.h"
#include "SSAOMap.h"
#include "Skybox.h"
#include "TextureMgr.h"
#include "BlendState.h"
#include "UI.h"
#include "TaskMgr.h"

GamePlayScene::GamePlayScene()
{
	curStage = GAMEPLAY_STAGE_LOBBY;

	lightStartDir = Normalize(XMFLOAT3(-cosf(XM_PIDIV2/1.2f)*2, -1, sinf(XM_PIDIV2/1.2f)*2));
	dLight = new DirectionalLight(
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.8f, 0.8f, 0.8f),
		XMFLOAT3(0.8f, 0.8f, 0.8f),
		lightStartDir);

	camera = new Camera("GamePlay", FRAME_KIND_ORTHOGONAL, SCREEN_WIDTH/10, SCREEN_HEIGHT/10, 0.1f, 300.0f, NULL, NULL,true);
	camera->transform->SetTranslation(XMFLOAT3(0, curCamDist, 0));
	camera->transform->SetRot(-UP, FORWARD);
	XMStoreFloat4x4(&orthogonalP, camera->StdProjMat());

	camera->SetFrame(FRAME_KIND_PERSPECTIVE, XMFLOAT2(NULL, NULL), 0.1f, 300.0f, XM_PIDIV2, 1);
	XMStoreFloat4x4(&perspectiveP, camera->StdProjMat());

	CameraMgr::Instance()->SetMain("GamePlay");

	D3D11_SAMPLER_DESC pointSamp_desc;
	ZeroMemory(&pointSamp_desc, sizeof(D3D11_SAMPLER_DESC));
	pointSamp_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pointSamp_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	pointSamp_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	pointSamp_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	pointSamp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	pointSamp_desc.MinLOD = 0;
	pointSamp_desc.MaxLOD = D3D11_FLOAT32_MAX;
	D3D11_SAMPLER_DESC anisotropicSamp_desc;
	ZeroMemory(&anisotropicSamp_desc, sizeof(D3D11_SAMPLER_DESC));
	anisotropicSamp_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	anisotropicSamp_desc.MaxAnisotropy = 4;
	anisotropicSamp_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicSamp_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicSamp_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicSamp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	anisotropicSamp_desc.MinLOD = 0;
	anisotropicSamp_desc.MaxLOD = D3D11_FLOAT32_MAX;
	D3D11_SAMPLER_DESC cmpPointSamp_desc;
	ZeroMemory(&cmpPointSamp_desc, sizeof(D3D11_SAMPLER_DESC));
	cmpPointSamp_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	cmpPointSamp_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	cmpPointSamp_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	cmpPointSamp_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	cmpPointSamp_desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	cmpPointSamp_desc.MinLOD = 0;
	cmpPointSamp_desc.MaxLOD = D3D11_FLOAT32_MAX;
	D3D11_SAMPLER_DESC linearPointSamp_desc;
	ZeroMemory(&linearPointSamp_desc, sizeof(D3D11_SAMPLER_DESC));
	linearPointSamp_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	linearPointSamp_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	linearPointSamp_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	linearPointSamp_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	linearPointSamp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	linearPointSamp_desc.MinLOD = 0;
	linearPointSamp_desc.MaxLOD = D3D11_FLOAT32_MAX;
	D3D11_SAMPLER_DESC linearSamp_desc;
	ZeroMemory(&linearSamp_desc, sizeof(D3D11_SAMPLER_DESC));
	linearSamp_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamp_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamp_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	linearSamp_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	linearSamp_desc.MinLOD = 0;
	linearSamp_desc.MaxLOD = D3D11_FLOAT32_MAX;

	r_assert(DX_Device->CreateSamplerState(&pointSamp_desc, pointSamp.GetAddressOf()));
	r_assert(DX_Device->CreateSamplerState(&linearPointSamp_desc, linearPointSamp.GetAddressOf()));
	r_assert(DX_Device->CreateSamplerState(&anisotropicSamp_desc, anisotropicSamp.GetAddressOf()));
	r_assert(DX_Device->CreateSamplerState(&cmpPointSamp_desc, cmpPointSamp.GetAddressOf()));
	r_assert(DX_Device->CreateSamplerState(&linearSamp_desc, linearSamp.GetAddressOf()));
	DX_DContext->PSSetSamplers(SHADER_REG_SAMP_POINT, 1, pointSamp.GetAddressOf());
	DX_DContext->PSSetSamplers(SHADER_REG_SAMP_LINEAR_POINT, 1, linearPointSamp.GetAddressOf());
	DX_DContext->PSSetSamplers(SHADER_REG_SAMP_ANISOTROPIC, 1, anisotropicSamp.GetAddressOf());
	DX_DContext->PSSetSamplers(SHADER_REG_SAMP_CMP_POINT, 1, cmpPointSamp.GetAddressOf());
	DX_DContext->PSSetSamplers(SHADER_REG_SAMP_LINEAR, 1, linearSamp.GetAddressOf());

	std::vector<std::string> cmTex;
	cmTex.push_back("Data\\Texture\\cm_px.jpg");
	cmTex.push_back("Data\\Texture\\cm_nx.jpg");
	cmTex.push_back("Data\\Texture\\cm_py.jpg");
	cmTex.push_back("Data\\Texture\\cm_ny.jpg");
	cmTex.push_back("Data\\Texture\\cm_pz.jpg");
	cmTex.push_back("Data\\Texture\\cm_nz.jpg");
	TextureMgr::Instance()->LoadCM("cm", cmTex);
	skybox = new Skybox();
	skybox->transform->SetTranslation(camera->transform->GetPos());
	AddObj(skybox);

	nonaga = new NonagaStage(this);
	std::vector<Object*> opaqueTokens;
	std::vector<Object*> transpTokens;
	std::vector<Object*> tiles;
	nonaga->GetOpaqueTokens(opaqueTokens);
	nonaga->GetTranspTokens(transpTokens);
	nonaga->GetTiles(tiles);
	for (auto go : opaqueTokens)
	{
		AddObj(go);
	}
	for (auto go : transpTokens)
	{
		AddObj(go);
	}
	for (auto go : tiles)
	{
		AddObj(go);
	}

	cbEye = new Buffer(sizeof(XMFLOAT4));

	curP = XMMatrixIdentity();

	slideStartPt = camera->transform->GetPos();
	slideEndPt = -slideEndForward * curCamDist;
	slideEndForward = Normalize(XMFLOAT3(0, -1, 4));
	slideEndUp = Normalize(XMFLOAT3(0, 4, 1));
	moveAngleX = acos(Dot(FORWARD, slideEndForward));
	moveAngleY = 0;


	std::vector<Object const*> cOpaqueTokens(3);
	cOpaqueTokens[0] = opaqueTokens[0];
	cOpaqueTokens[1] = opaqueTokens[1];
	cOpaqueTokens[2] = opaqueTokens[2]; 
	std::vector<Object const*> cTranspTokens(3);
	cTranspTokens[0] = transpTokens[0];
	cTranspTokens[1] = transpTokens[1];
	cTranspTokens[2] = transpTokens[2];
	oShadowMapping = new OpaqueShadowMap(2048, 2048, 128, 128, cOpaqueTokens);
	tShadowMapping = new TranspShadowMap(XMUINT2(2048, 2048), XMUINT2(128, 128), cTranspTokens);
	ssao = new SSAOMap();

	canvas = new UICanvas(SCREEN_WIDTH, SCREEN_HEIGHT);
	TextureMgr::Instance()->Load("thankyou", "Data\\Texture\\thankyou.png");
	endUI = new UI(canvas, XMFLOAT2(0, 50), XMFLOAT2(760, 190), 0, TextureMgr::Instance()->Get("thankyou"));
	endUI->SetEnabled(false);
	endUI->Fade(-1);
}

GamePlayScene::~GamePlayScene()
{
	delete cbEye;
	delete dLight;
	delete camera;
	delete nonaga;
}

void GamePlayScene::BindEye()
{
	XMFLOAT4 camEye = XMFLOAT4(
		camera->transform->GetPos().x,
		camera->transform->GetPos().y,
		camera->transform->GetPos().z, 0);
	cbEye->Write(&camEye);
	DX_DContext->PSSetConstantBuffers(SHADER_REG_CB_EYE, 1, cbEye->GetAddress());
}

void GamePlayScene::CameraMove(float spf)
{
	curCamDist -= spf * closeUpSpeed * Mouse::Instance()->GetWheel();
	curCamDist = Clamp(20, 100, curCamDist);

	static XMFLOAT2 prevMousePt = XMFLOAT2(0, 0);
	XMFLOAT3 cameraForward = camera->transform->GetForward();
	XMFLOAT2 mPt = Mouse::Instance()->Pos();
	if (Mouse::Instance()->RightState() == MOUSE_STATE_PRESSING)
	{
		moveAngleY += angleSpeed * spf * (mPt.x - prevMousePt.x);

		moveAngleX += (mPt.y-prevMousePt.y) * angleSpeed * spf;
		moveAngleX = Clamp(-XM_PIDIV4, XM_PIDIV2-0.01f, moveAngleX);
	}
	prevMousePt.x = mPt.x;
	prevMousePt.y = mPt.y;

	XMMATRIX rot = XMMatrixRotationX(moveAngleX)*XMMatrixRotationY(moveAngleY);
	XMFLOAT3 f = MultiplyDir(FORWARD, rot);
	XMFLOAT3 r = Normalize(Cross(UP, f));
	XMFLOAT3 u = Cross(f, r);
	camera->transform->SetRot(f,u,r);
	camera->transform->SetTranslation(-f * curCamDist);
}
void GamePlayScene::Update(float elapsed, float spf)
{
	XMMATRIX curTempP = camera->StdProjMat();
	switch (curStage)
	{
	case GAMEPLAY_STAGE_LOBBY:

		curTempP = XMLoadFloat4x4(&orthogonalP);
		break;
	case GAMEPLAY_STAGE_CAM_MODIFY:
	{
		curTime += spf;


		float t = curTime / camFrameLerpingTime;
		CameraFrameLerping(fminf(1,t));
		CameraSliding(Clamp(0,1,t-0.9f));
		LightRotating(Clamp(0, 1, t - 0.9f));
		curTempP = curP;
		skybox->transform->SetTranslation(camera->transform->GetPos());

		if (t>2)
			curStage = GAMEPLAY_STAGE_PLAY;
	}
		break;

	case GAMEPLAY_STAGE_PLAY:
	{
		CameraMove(spf);

		Geometrics::Ray camRay;
		camera->Pick(&camRay);

		nonaga->UpdateGame(&camRay, spf);

		skybox->transform->SetTranslation(camera->transform->GetPos());

		if (nonaga->IsEnd())
		{
			endUI->SetEnabled(true);
			curStage = GAMEPLAY_STAGE_END;
		}
	}
		break;
		
	case GAMEPLAY_STAGE_END:

		endUI->Fade(pow(spf+0.05f, 2));

		nonaga->UpdateGame(nullptr, spf);

		CameraMove(spf);
		skybox->transform->SetTranslation(camera->transform->GetPos());
		break;
	}

	camera->Update();

	Scene::Update(elapsed, spf);

	//binding
	BindEye();
	skybox->Mapping();
	oShadowMapping->Mapping(dLight);
	tShadowMapping->Mapping(dLight);
	ssao->Mapping(this, camera->VMat(), curTempP);

	TaskMgr::Instance()->Work();
}

void GamePlayScene::Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth, void const* subject) const
{
	dLight->Apply();

	XMMATRIX curTempVP = vp;
	switch (curStage)
	{
	case GAMEPLAY_STAGE_LOBBY:
		curTempVP = camera->VMat()*XMLoadFloat4x4(&orthogonalP);
		break;
	case GAMEPLAY_STAGE_CAM_MODIFY:
		curTempVP = camera->VMat()*curP;
		break;
	case GAMEPLAY_STAGE_PLAY:
		break;
	}

	nonaga->Render(curTempVP, frustum, sceneDepth);
	Scene::Render(curTempVP, frustum, sceneDepth, subject);
	canvas->Render(sceneDepth);
}

void GamePlayScene::Message(UINT msg)
{
	switch (msg)
	{
	case MSG_SCENE_PVP_INTRO_START:
		curStage = GAMEPLAY_STAGE_CAM_MODIFY;
		break;
	case MSG_SCENE_AI_NORMAL_INTRO_START:
		curStage = GAMEPLAY_STAGE_CAM_MODIFY;
		break;
	case MSG_SCENE_AI__HARD_INTRO_START:
		curStage = GAMEPLAY_STAGE_CAM_MODIFY;
		break;
	}

}

void GamePlayScene::CameraFrameLerping(float t)
{
	float mt = pow(t, 5);

	curP = XMMATRIX(
		Lerp(orthogonalP._11, perspectiveP._11, mt), 0, 0, 0,
		0, Lerp(orthogonalP._22, perspectiveP._22, mt), 0, 0,
		0, 0, Lerp(orthogonalP._33, perspectiveP._33, mt), Lerp(0, 1, mt),
		0, 0, Lerp(orthogonalP._43, perspectiveP._43, mt), Lerp(1, 0, mt)
	);
}

void GamePlayScene::CameraSliding(float t)
{
	float mt = (1-cosf(t * XM_PI)) / 2;

	XMFLOAT3 sForward = Normalize(Lerp(-UP, slideEndForward, mt));
	XMFLOAT3 sUp = Normalize(Lerp(FORWARD, slideEndUp, mt));

	camera->transform->SetTranslation(-sForward* curCamDist);
	camera->transform->SetRot(sForward, sUp);


}

void GamePlayScene::LightRotating(float t)
{
	float mt = (1 - cosf(t * 2 *XM_PI/3)) / 2;

	XMFLOAT3 startDir = MultiplyDir(lightStartDir, XMMatrixRotationY(mt * XM_PIDIV2));

	dLight->SetDir(startDir);
}
