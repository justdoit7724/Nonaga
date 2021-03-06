#include "stdafx.h"
#include "Camera.h"
#include "Transform.h"
#include "CameraMgr.h"
#include "Scene.h"
#include "Mouse.h"
#include "Game_info.h"

#define Z_ORDER_MAX 5

Camera::Camera(std::string key, FRAME_KIND frameKind, float orthoScnWidth, float orthoScnHeight, float n, float f, float verticalViewRad, float aspectRatio, bool skipFrustum)
	:key(key)
{
	CameraMgr::Instance()->Add(key, this);

	transform = new Transform();

	SetView();
	SetFrame(frameKind, XMFLOAT2(orthoScnWidth, orthoScnHeight), n, f, verticalViewRad, aspectRatio);

	frustum.skip = skipFrustum;
}
Camera::Camera(FRAME_KIND frameKind, float orthoScnWidth, float orthoScnHeight, float n, float f, float verticalViewRad, float aspectRatio, bool skipFrustum)
	:key("")
{
	transform = new Transform();

	SetView();
	SetFrame(frameKind, XMFLOAT2(orthoScnWidth, orthoScnHeight), n, f, verticalViewRad, aspectRatio);

	frustum.skip = skipFrustum;
}

Camera::~Camera()
{
	if(key!="")
		CameraMgr::Instance()->Remove(key);

	delete transform;
	delete[] projMats;
}
void Camera::SetFrame(const FRAME_KIND fKind, XMFLOAT2 orthoSize, const float n, const float f, const float verticalViewRad, const float aspectRatio)
{
	assert(n > 0.0f);

	curFrame = fKind;
	size = orthoSize;
	this->n = n;
	this->f = f;
	this->verticalRadian = verticalViewRad;
	this->aspectRatio = aspectRatio;

	projMats = new XMMATRIX[5];
	float interval = 1.0f / Z_ORDER_MAX;
	assert(Z_ORDER_MAX >= 1);
	switch (fKind)
	{
	case FRAME_KIND_PERSPECTIVE:
	{
		float sX = 1.0f / (aspectRatio * tan(verticalViewRad * 0.5f));
		float sY = 1.0f / tan(verticalViewRad * 0.5f);
		for (int i = 0; i < Z_ORDER_MAX; ++i)
		{
			float minD = i * interval;
			float maxD = (i + 1) * interval;
			float A = maxD * f / (f - n) - minD * n / (f - n);
			float B = n * (minD - A);

			projMats[i] = XMMATRIX(
				sX, 0, 0, 0,
				0, sY, 0, 0,
				0, 0, A, 1,
				0, 0, B, 0);
		}
		stdProjMat = XMMATRIX(
			sX, 0, 0, 0,
			0, sY, 0, 0,
			0, 0, f / (f - n), 1,
			0, 0, -n * f / (f - n), 0);
	}
		break;
	case FRAME_KIND_ORTHOGONAL:
	{
		float sX = 2.0f / size.x;
		float sY = 2.0f / size.y;
		for (int i = 0; i < Z_ORDER_MAX; ++i)
		{
			float sZ = interval / (f - n);
			float M = (-n / (f - n) + i) * interval;

			projMats[i] = XMMATRIX(
				sX, 0, 0, 0,
				0, sY, 0, 0,
				0, 0, sZ, 0,
				0, 0, M, 1
			);
		}
		stdProjMat = XMMATRIX(
			sX, 0, 0, 0,
			0, sY, 0, 0,
			0, 0, 1.0f / (f - n), 0,
			0, 0, -n / (f - n), 1
		);
	}
		break;
	}
	
}

void Camera::SetView()
{
	XMFLOAT3 pos = transform->GetPos();
	XMFLOAT3 forward = transform->GetForward();
	XMFLOAT3 up = transform->GetUp();
	XMFLOAT3 right = transform->GetRight();

	float x = -Dot(pos, right);
	float y = -Dot(pos, up);
	float z = -Dot(pos, forward);

	viewMat = XMMATRIX( // inverse of cam world matrix
		right.x, up.x, forward.x, 0,
		right.y, up.y, forward.y, 0,
		right.z, up.z, forward.z, 0,
		x, y, z, 1);
}

void Camera::Update()
{
	if (!frustum.skip)
	{
		XMFLOAT3 p = transform->GetPos();
		XMFLOAT3 forward = transform->GetForward();
		XMFLOAT3 up = transform->GetUp();
		XMFLOAT3 right = transform->GetRight();

		float tri = tan(verticalRadian * 0.5f);
		XMFLOAT3 trDir = Normalize(
			right * tri * f * aspectRatio +
			up * tri * f +
			forward * f);
		XMFLOAT3 blDir = Normalize(
			-right * tri * f * aspectRatio +
			-up * tri * f +
			forward * f);

		frustum.front = Geometrics::PlaneInf(p + forward * f, -forward);
		frustum.back = Geometrics::PlaneInf(p + forward * n, forward);
		frustum.left = Geometrics::PlaneInf(p, Cross(up, blDir));
		frustum.right = Geometrics::PlaneInf(p, Cross(-up, trDir));
		frustum.top = Geometrics::PlaneInf(p, Cross(right, trDir));
		frustum.bottom = Geometrics::PlaneInf(p, Cross(blDir, right));
	}

	SetView();
}
void Camera::Pick(OUT Geometrics::Ray* ray)const
{
	XMFLOAT2 scnPos = Mouse::Instance()->Pos();

	XMFLOAT2 pPos = XMFLOAT2(
		((scnPos.x * 2) / (float)SCREEN_WIDTH - 1),
		-(scnPos.y * 2) / (float)SCREEN_HEIGHT + 1);
	XMFLOAT3 vDir = XMFLOAT3(NULL,NULL,NULL);

	const XMFLOAT3 forward = transform->GetForward();
	const XMFLOAT3 up = transform->GetUp();
	const XMFLOAT3 right = transform->GetRight();
	const XMFLOAT3 eye = transform->GetPos();
	switch (curFrame)
	{
	case FRAME_KIND_PERSPECTIVE:
	{
		// vPos at z which is on d 
		vDir = Normalize(XMFLOAT3(
			pPos.x * aspectRatio* tan(verticalRadian * 0.5f),
			pPos.y* tan(verticalRadian * 0.5f),
			1));

		XMMATRIX invVDirMat = XMMATRIX(
			right.x, right.y, right.z, 0,
			up.x, up.y, up.z, 0,
			forward.x, forward.y, forward.z, 0,
			eye.x, eye.y, eye.z, 1);
		XMMATRIX id = VMat() * invVDirMat;
		//invVDirMat = XMMatrixInverse(&XMMatrixDeterminant(VMat()), VMat());

		ray->o = eye;
		ray->d = MultiplyDir(vDir, invVDirMat);
	}
		break;
	case FRAME_KIND_ORTHOGONAL:
	{
		float invSX = size.x/2;
		float invSY = size.y/2;
		XMFLOAT3 vPos = XMFLOAT3(
			pPos.x * invSX,
			pPos.y * invSY,
			1);
		vDir = Normalize(vPos);

		ray->o = eye + right * vPos.x + up * vPos.y;

		ray->d = forward;
	}
		break;
	default:
		break;
	}


	
}
