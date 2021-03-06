#pragma once
#include "GameObject.h"
#include "define.h"
#include "Brick.h"
class Ghoul : public CGameObject
{
public:
	Ghoul();
	~Ghoul();
	void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	void Render();
	RECT GetBound();
	void GetBoundingBox(float& left, float& top, float& right, float& bottom);
};

