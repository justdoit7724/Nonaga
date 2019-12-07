#pragma once

#include "Shape.h"
class Cylinder :
	public Shape
{
public:
	Cylinder(const int intensity);
private:
	void CalcMinMaxPt(Vertex* vertice, int vertCount);
};

