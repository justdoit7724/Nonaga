#include "stdafx.h"
#include "MappingVisualization.h"
#include "UI.h"
#include "Game_info.h"

MappingVisualization::MappingVisualization(ID3D11ShaderResourceView* shadow1, ID3D11ShaderResourceView* shadow2, ID3D11ShaderResourceView* ssao)
{
	canvas = new UICanvas(SCREEN_WIDTH, SCREEN_HEIGHT);
}

MappingVisualization::~MappingVisualization()
{
}
