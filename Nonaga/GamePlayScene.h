#pragma once

#include "Scene.h"

class NonagaStage;
class DirectionalLight;
class Buffer;
class OpaqueShadowMap;
class TranspShadowMap;
class Camera;
class SSAOMap;
class Skybox;
class UICanvas;
class UI;

class GamePlayScene : public Scene
{
public:
	GamePlayScene();
	~GamePlayScene();

	void Update(float elapsed, float spf) override;
	void Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth, void const* subject)const override;

	void Message(UINT msg)override;

private:
	void BindSamp();
	void BindEye();
	void CameraMove(float spf);
	void CameraFrameLerping(float t);
	void CameraSliding(float t);
	void LightRotating(float t);
	float curTime = 0;
	const float camFrameLerpingTime = 4;

	XMFLOAT4X4 orthogonalP;
	XMFLOAT4X4 perspectiveP;
	XMMATRIX curP;

	ComPtr<ID3D11SamplerState> pointSamp;
	ComPtr<ID3D11SamplerState> linearSamp;
	ComPtr<ID3D11SamplerState> anisotropicSamp;
	ComPtr<ID3D11SamplerState> linearPointSamp;
	ComPtr<ID3D11SamplerState> oShadowSamp;
	ComPtr<ID3D11SamplerState> tShadowSamp;
	ComPtr<ID3D11SamplerState> cmpPointSamp;
	ComPtr<ID3D11SamplerState> cmpLinearPointSamp;

	enum GAMEPLAY_STAGE
	{
		GAMEPLAY_STAGE_LOBBY=0,
		GAMEPLAY_STAGE_CAM_MODIFY=1,
		GAMEPLAY_STAGE_PLAY=2,
		GAMEPLAY_STAGE_END=3
	};
	GAMEPLAY_STAGE curStage;

	DirectionalLight* dLight;

	Camera* camera;

	NonagaStage* nonaga; 

	Buffer* cbEye;

	XMFLOAT3 slideStartPt, slideEndPt;
	XMFLOAT3 slideEndForward;
	XMFLOAT3 slideEndUp;
	XMFLOAT3 lightStartDir;

	float moveAngleX = 0;
	float moveAngleY = 0;
	float curCamDist=40;
	const float closeUpSpeed = 90;
	const float angleSpeed = 3.141592f * 0.025f;

	Skybox* skybox;
	OpaqueShadowMap* oShadowMapping;
	TranspShadowMap* tShadowMapping;
	SSAOMap* ssao;

	UI* endUI;
	UICanvas* canvas;
};

