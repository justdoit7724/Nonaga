#pragma once
#include "Object.h"

class Shape;
class Camera;
class Scene;

class Token : public Object
{
public:
	Token(Scene* environemnt, unsigned int id, bool p1);
	Token(bool isRed);

	void Render(const XMMATRIX& parentWorld, const XMMATRIX& vp, const Frustum* frustum, UINT sceneDepth) const override;

	void Move(int toID, XMFLOAT3 pos);
	unsigned int ID() { return id; }
	bool IsP1()const { return isP1; }
	const bool isIndicator;
private:

	void Capture(XMFLOAT3 forward, XMFLOAT3 up, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* ds, UINT sceneDepth)const;
	void DrawDCM(UINT sceneDepth)const;

	const Scene* environment;
	unsigned int id;
	bool isP1;

	static Camera* captureCam;

	static Shape* mesh;

	ComPtr<ID3D11RenderTargetView> captureRTV[6];
	ComPtr<ID3D11ShaderResourceView> captureSRV;
	ComPtr<ID3D11DepthStencilView> captureDSV;
};