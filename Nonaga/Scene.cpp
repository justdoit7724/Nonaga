#include "Scene.h"
#include "Object.h"
#include "Shader.h"
#include "ShaderFormat.h"
#include "Camera.h"
#include "Transform.h"
#include "SceneMgr.h"

Scene::~Scene()
{
	for (auto obj : objs)
		delete obj;
}

void Scene::Update(float elapsed, float spf)
{
	XMMATRIX im = DirectX::XMMatrixIdentity();
	for (auto obj : objs)
	{
		obj->Update(im);
	}
}

void Scene::Render(const XMMATRIX& vp, const Frustum* frustum, UINT sceneDepth) const
{
	XMMATRIX im = DirectX::XMMatrixIdentity();

	for (auto obj : objs)
	{
		obj->Render(im, vp, frustum,sceneDepth);
	}
}

void Scene::GetObjs(std::vector<const Object*>& rObj)const
{
	for (auto o : objs)
	{
		rObj.push_back(o);
	}
}

void Scene::AddObj(Object* obj)
{
	objs.push_back(obj);
}
