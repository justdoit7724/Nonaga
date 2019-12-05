#include "TaskMgr.h"
#include "Scene.h"

void MoveIdx(UINT& idx) {
	idx += 1;
	idx = (idx >= TASK_MAX) ? 0 : idx;
}

void TaskMgr::Add(void const* subject, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, Scene const* scene, XMMATRIX const& vp, const Frustum& frustum, UINT sceneDepth)
{
	MoveIdx(lastIdx);

	tasks[lastIdx].rtv = rtv;
	tasks[lastIdx].dsv = dsv;
	tasks[lastIdx].scene = scene;
	tasks[lastIdx].frustum = frustum;
	tasks[lastIdx].vp = vp;
	tasks[lastIdx].sceneDepth = sceneDepth;
	tasks[lastIdx].subject = subject;
	tasks[lastIdx].isFree = false;

	if (subjects.count(subject))
		subjects[subject]++;
	else
		subjects.insert(std::pair<void const*, UINT>(subject, 1));
}

void TaskMgr::Work()
{
	if (tasks[startIdx].isFree)
		return;

	ID3D11RenderTargetView* oriRtv;
	ID3D11DepthStencilView* oriDsv;
	DX_DContext->OMGetRenderTargets(1, &oriRtv, &oriDsv);

	DX_DContext->ClearRenderTargetView(tasks[startIdx].rtv, Colors::Transparent);
	DX_DContext->ClearDepthStencilView(tasks[startIdx].dsv, D3D11_CLEAR_DEPTH, 1.0f, NULL);
	DX_DContext->OMSetRenderTargets(1, &tasks[startIdx].rtv, tasks[startIdx].dsv);
	tasks[startIdx].scene->Render(tasks[startIdx].vp, tasks[startIdx].frustum, tasks[startIdx].sceneDepth, tasks[startIdx].subject);
	tasks[startIdx].isFree = true;

	if ((--subjects[tasks[startIdx].subject]) == 0)
		subjects.erase(tasks[startIdx].subject);
	
	DX_DContext->OMSetRenderTargets(1, &oriRtv, oriDsv);

	MoveIdx(startIdx);
}

bool TaskMgr::HasTaskFrom(void const* subject)
{
	return (subjects.count(subject) >= 1);
}
