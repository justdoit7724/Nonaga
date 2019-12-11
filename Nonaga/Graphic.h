#pragma once

#include "DX_info.h"

class Graphic
{
public:
	Graphic(HWND _hwnd);
	~Graphic();

	void Present();

private:

	HWND hwnd;

	ComPtr<ID3D11Texture2D> backBuffer;
	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	D3D11_VIEWPORT viewport;
	ComPtr<IDXGISwapChain> swapchain;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11DepthStencilView> dsView;
	ComPtr<ID3D11RasterizerState> rasterizerState;
};

