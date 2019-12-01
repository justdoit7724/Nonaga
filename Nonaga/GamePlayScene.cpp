#include "GamePlayScene.h"
#include "Camera.h"
#include "Game_info.h"
#include "Transform.h"
#include "CameraMgr.h"
#include "Debugging.h"
#include "SceneMgr.h"
#include "Mouse.h"
#include "Buffer.h"
#include "Light.h"
#include "Nonaga.h"
#include "ShaderReg.h"
#include "ShadowMap.h"
#include "SSAOMap.h"
#include "Skybox.h"
#include "Debugging.h"
#include "TextureMgr.h"
#include "Cube.h"
#include "BlendState.h"
#include "UI.h"

GamePlayScene::GamePlayScene()
{
	curStage = GAMEPLAY_STAGE_LOBBY;

	dLight = new DirectionalLight(
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.8f, 0.8f, 0.8f),
		XMFLOAT3(0.8f, 0.8f, 0.8f),
		Normalize(XMFLOAT3(2,-1,0)));

	camera = new Camera("GamePlay", FRAME_KIND_ORTHOGONAL, SCREEN_WIDTH/10, SCREEN_HEIGHT/10, 0.1f, 300.0f, NULL, NULL,true);
	camera->transform->SetTranslation(XMFLOAT3(0, curCamDist, 0));
	camera->transform->SetRot(-UP, FORWARD);
	XMStoreFloat4x4(&orthogonalP, camera->StdProjMat());

	camera->SetFrame(FRAME_KIND_PERSPECTIVE, XMFLOAT2(NULL, NULL), 0.1f, 300.0f, XM_PIDIV2, 1);
	XMStoreFloat4x4(&perspectiveP, camera->StdProjMat());

	CameraMgr::Instance()->SetMain("GamePlay");

	std::vector<std::string> cm;
	cm.push_back("Data\\Texture\\cm_px.jpg");
	cm.push_back("Data\\Texture\\cm_nx.jpg");
	cm.push_back("Data\\Texture\\cm_py.jpg");
	cm.push_back("Data\\Texture\\cm_ny.jpg");
	cm.push_back("Data\\Texture\\cm_pz.jpg");
	cm.push_back("Data\\Texture\\cm_nz.jpg");
	TextureMgr::Instance()->LoadCM("cm", cm);
	skybox = new Skybox(TextureMgr::Instance()->Get("cm"));
	AddObj(skybox);

	nonaga = new NonagaStage(this);
	std::vector<Object*> opaqueTokens;
	std::vector<Object*> transpTokens;
	nonaga->GetOpaqueTokens(opaqueTokens);
	nonaga->GetTranspTokens(transpTokens);
	for (auto go : opaqueTokens)
	{
		AddObj(go);
	}
	for (auto go : transpTokens)
	{
		AddObj(go);
	}

	cbEye = new Buffer(sizeof(XMFLOAT4));

	Debugging::Instance()->EnableGrid(10);

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
		CameraMgr::Instance()->Main()->transform->GetPos().x,
		CameraMgr::Instance()->Main()->transform->GetPos().y,
		CameraMgr::Instance()->Main()->transform->GetPos().z, 0);
	cbEye->Write(&camEye);
	DX_DContext->PSSetConstantBuffers(SHADER_REG_PS_CB_EYE, 1, cbEye->GetAddress());
}

void GamePlayScene::CameraMove(float spf)
{
	curCamDist -= spf * closeUpSpeed * Mouse::Instance()->GetWheel();

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
	static float curTime = 0;
	curTime += spf;
	dLight->SetDir(MultiplyDir(dLight->GetDir(), XMMatrixRotationY(curTime)));

	switch (curStage)
	{
	case GAMEPLAY_STAGE_LOBBY:
		break;
	case GAMEPLAY_STAGE_CAM_MODIFY:
	{
		curTime += spf;

		float t = curTime / camFrameLerpingTime;
		CameraFrameLerping(t);
		CameraSliding(Clamp(0,1,t-0.9));
		LightRotating(t);
		if (t>2)
			curStage = GAMEPLAY_STAGE_PLAY;
	}
		break;
	case GAMEPLAY_STAGE_PLAY:
		CameraMove(spf);

		Geometrics::Ray camRay;
		camera->Pick(&camRay);

		nonaga->Update(camRay, spf);
		break;
	}

	camera->Update();

	Scene::Update(elapsed, spf);

	//binding
	BindEye();
	skybox->Mapping();
	//oShadowMapping->Mapping(dLight);
	tShadowMapping->Mapping(dLight);
	//ssao->Mapping(this, camera);
}

void GamePlayScene::Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth) const
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

	Scene::Render(curTempVP, frustum, sceneDepth);
	nonaga->Render(curTempVP, frustum, sceneDepth);
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
	curP = XMMATRIX(
		Lerp(orthogonalP._11, perspectiveP._11, t), 0, 0, 0,
		0, Lerp(orthogonalP._22, perspectiveP._22, t), 0, 0,
		0, 0, Lerp(orthogonalP._33, perspectiveP._33, t), Lerp(0, 1, t),
		0, 0, Lerp(orthogonalP._43, perspectiveP._43, t), Lerp(1, 0, t)
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

}
