#pragma once
#include "Scene.h"

class NonagaStage;
class DirectionalLight;
class Buffer;
class ShadowMap;
class Camera;
class SSAOMap;
class Skybox;
class UICanvas;

class GamePlayScene : public Scene
{
public:
	GamePlayScene();
	~GamePlayScene();

	void Update(float elapsed, float spf) override;
	void Render(const XMMATRIX& vp, const Frustum* frustum, UINT sceneDepth)const override;

	void Message(UINT msg)override;

private:
	void BindEye();
	void CameraMove(float spf);
	void CameraFrameLerping(float t);
	void CameraSliding(float t);
	void LightRotating(float t);
	float curTime = 0;
	const float camFrameLerpingTime = 2;

	XMFLOAT4X4 orthogonalP;
	XMFLOAT4X4 perspectiveP;
	XMMATRIX curP;

	enum GAMEPLAY_STAGE
	{
		GAMEPLAY_STAGE_LOBBY=0,
		GAMEPLAY_STAGE_CAM_MODIFY=1,
		GAMEPLAY_STAGE_PLAY=2
	};
	GAMEPLAY_STAGE curStage;

	DirectionalLight* dLight;

	Camera* camera;

	NonagaStage* nonaga; 

	Buffer* cbEye;

	XMFLOAT3 slideStartPt, slideEndPt;
	XMFLOAT3 slideEndForward;
	XMFLOAT3 slideEndUp;

	float moveAngleX = 0;
	float moveAngleY = 0;
	float curCamDist=60;
	const float closeUpSpeed = 50;
	const float angleSpeed = 3.141592f * 0.025f;

	Skybox* skybox;
	ShadowMap* shadowMapping;
	SSAOMap* ssao;

	UICanvas* canvas;
};

