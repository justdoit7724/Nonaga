#pragma once
#include <string>
#include "DX_info.h"
#include "Network.h"
#include "Geometrics.h"
#include <memory>

struct Frustum;
class Transform;
class Shape;
class VShader;
class HShader;
class DShader;
class GShader;
class PShader;
class BlendState;
class DepthStencilState;
class RasterizerState;


class Object : public IDebug
{
public:
	Object(std::string name, std::shared_ptr<Shape> shape, std::shared_ptr<Shape> lodShape, std::string sVS, const D3D11_INPUT_ELEMENT_DESC* iLayouts, UINT layoutCount, std::string sHS, std::string sDS, std::string sGS, std::string sPS, int zOrder);
	Object(std::string name, std::shared_ptr<Shape> shape, std::shared_ptr<Shape> lodShape, ID3D11ShaderResourceView* diffSRV, ID3D11ShaderResourceView* normalSRV);
	~Object();

	virtual void Update();
	virtual void Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth) const;
	virtual void RenderGeom() const;

	virtual bool IsPicking(const Geometrics::Ray ray)const;
	virtual void UpdateBound();

	void Visualize() override;
	void SetEnabled(bool e) { enabled = e; }
	void SetShow(bool s) { show = s; }

	//TODO
	Transform* transform;
	std::shared_ptr < Shape> shape;
	std::shared_ptr < Shape> lodShape;
	VShader* vs;
	HShader* hs;
	DShader* ds;
	GShader* gs;
	PShader* ps;
	BlendState * blendState = nullptr;
	DepthStencilState * dsState = nullptr;
	RasterizerState* rsState = nullptr;

	const std::string name;
	const int zOrder;
	Geometrics::Sphere Bound() { return bound; }


protected:
	Object();
	void Render()const;
	virtual bool IsInsideFrustum(const Frustum& frustum) const;

	bool enabled = true;
	bool show = true;


	Geometrics::Sphere bound;
};

