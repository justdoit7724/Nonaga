#pragma once
#include "DX_info.h"
#include <vector>

class Object;
class SceneMgr;
struct Frustum;

class Scene
{
public:
	virtual ~Scene();

	virtual void Update(float elapsed, float spf);
	virtual void Render(const XMMATRIX& vp, const Frustum& frustum, UINT sceneDepth)const;
	const Object* GetObj(UINT id)const;

	virtual void Message(UINT msg) {};
	bool Enabled() { return enabled; }
	void SetEnabled(bool e) { enabled = e; }

	const std::string key;
protected:
	void AddObj(Object* obj) { objs.push_back(obj); }

private:
	bool enabled=false;
	std::vector<Object*> objs;
};

