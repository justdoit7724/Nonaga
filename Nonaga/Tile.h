#pragma once

#include "Object.h"

class Tile : public Object
{
public: 
	Tile(unsigned int id, std::shared_ptr<Shape> shape, std::shared_ptr<Shape> lodShape);
	Tile(bool isRed, std::shared_ptr<Shape> shape);

	void Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth)const override;
	void UpdateBound()override;

	void Move(int toId);

	int ID() { return id; }
	float FallingSpeed() { return fallingSpeed; }

	const bool isIndicator;
private:
	int id;

	float fallingSpeed;

	PShader* lodPs;
};
