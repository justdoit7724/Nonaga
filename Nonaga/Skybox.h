#pragma once
#include "Object.h"

class Buffer;

class Skybox : public Object
{
public:
	Skybox(ID3D11ShaderResourceView* srv);

	void Mapping();
	void Update(const XMMATRIX& prevWorld) override;
	void Render(const XMMATRIX& parentWorld, const XMMATRIX& vp, const Frustum* frustum, UINT sceneDepth)const override;
	void RenderGeom() const override {};

private:
	bool IsInsideFrustum(const Frustum* frustum) const override { return true; }
	ComPtr<ID3D11SamplerState> samplerState;
};

