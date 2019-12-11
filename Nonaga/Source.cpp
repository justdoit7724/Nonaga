#include "stdafx.h"

#include "Window.h"
#include "Scene.h"
#include "Game_info.h"
#include "Graphic.h"
#include "SceneMgr.h"
#include "Timer.h"
#include "TextureMgr.h"
#include "Mouse.h"

#include "Lobby.h"
#include "GamePlayScene.h"
#include "CameraMgr.h"
#include "Keyboard.h"
#include <dxgidebug.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	srand(time(NULL));
	Window window(hInstance, "Low Level");
	Graphic* graphic = new Graphic(window.Hwnd());



	Scene* playScene = new GamePlayScene();
	SceneMgr::Instance()->Add("GamePlay", playScene);
	SceneMgr::Instance()->SetEnabled("GamePlay", true);
	Scene* lobbyScene = new Lobby();
	SceneMgr::Instance()->Add("Lobby", lobbyScene);
	SceneMgr::Instance()->SetEnabled("Lobby", true);

	/*Scene* debugScene = new DebuggingScene();
	SceneMgr::Instance()->Add("Debugging", debugScene);
	SceneMgr::Instance()->SetEnabled("Debugging", true);*/

	Timer* worldTimer = new Timer();

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (msg.message != WM_QUIT) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		else
		{

			SceneMgr::Instance()->Process(worldTimer->Elapsed(), worldTimer->SPF());

			Mouse::Instance()->Update(worldTimer->SPF());

			graphic->Present();

			//window.SetTitle(std::to_string(worldTimer->FPS()));
			worldTimer->Update();
		}


	}

	delete worldTimer;

	TextureMgr::Instance()->Release();
	SceneMgr::Instance()->Release();
	CameraMgr::Instance()->Release();

	delete graphic;

	return 0;
}