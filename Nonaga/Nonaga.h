#pragma once

#include "Geometrics.h"
#include "TileSpaceInfo.h"

class Token;
class Tile;
class PlaySpace;
class Scene;
class Object;
struct Frustum;

#define NONE -1

class NonagaLogic
{
public:
	NonagaLogic(PlaySpace* const* space);
	~NonagaLogic();

	int CanMoveToken(int from, int to);
	bool CanPickTile(int id);
	bool CanMoveTileTo(int from, int to);
	void TokenMove(int from, int to);
	void TileMove(int from, int to);
	int GetScore(const std::vector<Token*>& tokens);
	bool IsP1Turn() { return p1Turn; }

private:
	PlaySpace* const* space;
	int unmovableTileID;
	bool p1Turn;
	bool CheckDirection(XMINT2 holdingTokenID2, XMINT2 offset, XMINT2& destId2);
	int CheckNeighbor(int centerID, int except);
};

class NonagaStage
{
public:
	NonagaStage(Scene* environment);
	~NonagaStage();

	void UpdateGame(const Geometrics::Ray* ray, float spf);
	void GetOpaqueTokens(std::vector<Object*>& objOutput);
	void GetTranspTokens(std::vector<Object*>& objOutput);
	void GetTiles(std::vector<Object*>& objOutput);

	void Render(const XMMATRIX& vp, const Frustum& frustum, unsigned int sceneDepth)const;

	bool IsEnd() { return (curPlayState == PLAY_STATE_FINISH); }
	
private:
	bool IsWin();
	void TokenDragStart(const Geometrics::Ray* ray);
	void TokenDragging();
	void TileDragStart(const Geometrics::Ray* ray);
	void TileDragging();
	bool GetCurID2(const Geometrics::Ray* ray);

	
	PlaySpace* playSpace[TILE_SPACE_COUNT_Z * TILE_SPACE_COUNT_X]{ nullptr };

	NonagaLogic* logic;

	enum PLAY_STATE
	{
		PLAY_STATE_P1_TOKEN,
		PLAY_STATE_P1_TOKEN_MOVING,
		PLAY_STATE_P1_TILE,
		PLAY_STATE_P1_TILE_MOVING,
		PLAY_STATE_P2_TOKEN,
		PLAY_STATE_P2_TOKEN_MOVING,
		PLAY_STATE_P2_TILE,
		PLAY_STATE_P2_TILE_MOVING,
		PLAY_STATE_FINISH
	}curPlayState;

	XMMATRIX tileSpaceMat;
	XMMATRIX invTileSpaceMat;
	Geometrics::PlaneInf detectPlane;
	int pDetectID;
	int holdingObjID;
	// for preventing change of current detect position between mouse pressing and up
	int pressingDetectID;
	
	bool isMove;

	Object* moveObj;
	XMFLOAT3 moveStart;
	XMFLOAT3 moveDest;
	float curTime;
	const float moveTokenSpeed = 0.5;
	const float moveTileSpeed = 1.0;
	const float jumpTokenHeight = 10;
	const float jumpTileHeight = 5;

	std::vector<Token*> tokens;
	Token* redToken, * greenToken;
	std::vector < Tile*> tiles;
	Tile* redTile, *greenTile;
};

