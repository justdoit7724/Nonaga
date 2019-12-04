#pragma once
#include "DX_info.h"

class UICanvas;
class UI;

class MappingVisualization
{
public :
	MappingVisualization(ID3D11ShaderResourceView* shadow1, ID3D11ShaderResourceView* shadow2, ID3D11ShaderResourceView* ssao);
	~MappingVisualization();

private:
	UICanvas* canvas;
	UI* s1UI;
	UI* s2UI;
	UI* aoUI;
};

