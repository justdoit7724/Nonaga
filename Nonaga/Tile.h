#pragma once
#include "Object.h"

class Tile : public Object
{
public: 
	Tile(unsigned int id);
	Tile(bool isRed);

	void UpdateBound()override;

	void Move(int toId, XMFLOAT3 pos);

	unsigned int ID() { return id; }

private:
	unsigned int id;

	float fallingSpeed;
};
