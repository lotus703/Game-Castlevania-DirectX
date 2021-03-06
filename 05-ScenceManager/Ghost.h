#pragma once
#include "GameObject.h"
#include "Simon.h"
#include "define.h"
class Ghost : public CGameObject
{
protected:
	Simon* mSimon;
public:
	Ghost(Simon* simon);
	~Ghost();
	void Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects);
	void Render();
	RECT GetBound();
	void GetBoundingBox(float& left, float& top, float& right, float& bottom);
};

