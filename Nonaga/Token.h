#pragma once

#include "Object.h"

class Shape;
class Camera;
class Scene;

class Token : public Object
{
public:
	Token(std::shared_ptr<Shape> shape, std::shared_ptr<Shape> lodShape, Scene* environemnt, unsigned int id, bool p1);
	Token(std::shared_ptr<Shape> shape, bool isRed);
	~Token();

	void Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth) const override;

	void Move(int toID);
	unsigned int ID() { return id; }
	bool IsP1()const { return isP1; }
	float FallingSpeed() { return fallingSpeed; }
	const bool isIndicator;
private:

	void Capture(XMFLOAT3 forward, XMFLOAT3 up, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* ds, UINT sceneDepth)const;
	void DrawDCM(UINT sceneDepth)const;

	const Scene* environment;
	unsigned int id;
	const bool isP1;

	static Camera* captureCam;

	ComPtr<ID3D11RenderTargetView> captureRTV[6];
	ComPtr<ID3D11ShaderResourceView> captureSRV;
	ComPtr<ID3D11DepthStencilView> captureDSV;

	VShader* lodVs;
	PShader* lodPs;

	float fallingSpeed;
};