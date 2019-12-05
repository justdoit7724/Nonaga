#pragma once
#include "DX_info.h"
#include "Singleton.h"
#include <vector>
#include <unordered_map>
#include "Camera.h"

class Scene;
class Object;

#define TASK_MAX 18

class TaskMgr : public Singleton<TaskMgr>
{
public:

	void Add(void const* subject,ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, Scene const* scene, XMMATRIX const& vp, const Frustum& frustum, UINT sceneDepth);
	void Work();
	bool HasTaskFrom(void const* subject);

private:

	struct Task
	{
		Task() {}
		
		bool isFree = true;
		ID3D11RenderTargetView* rtv;
		ID3D11DepthStencilView* dsv;
		Scene const* scene;
		Frustum frustum;
		XMMATRIX vp;
		UINT sceneDepth;
		void const* subject;

	};

	UINT startIdx = 0;
	UINT lastIdx = -1;
	Task tasks[TASK_MAX];
	std::unordered_map<const void*, UINT> subjects;
};