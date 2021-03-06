﻿#include "Simon.h"
#include <algorithm>
#include <assert.h>
#include "Utils.h"
#include "Game.h"
#include "Brick.h"
#include "MovingPlatform.h"
#include "BreakableBrick.h"

Simon* Simon::__instance = NULL;

 
Simon* Simon::GetInstance()
{
	if (__instance == NULL)
		__instance = new Simon();
	return __instance;
}

Simon::Simon(float x, float y) : CGameObject()
{
	untouchable = 0;
	SetState(SIMON_STATE_IDLE);

	start_x = x;
	start_y = y;
	this->x = x;
	this->y = y;
	whip = new Whip();
	//dagger = new Dagger();
	SetNumLife(3);
	SetHealth(16);
	SetNumHeart(5);
}
void Simon::Render()
{
	int ani = -1;
	if (state == SIMON_STATE_DIE) ani = SIMON_ANI_DIE;
	else if (state == SIMON_STATE_SIT) ani = SIMON_ANI_SIT;
	else if (state == SIMON_STATE_JUMP)
	{  
		if (vy > 0)
		{
			ani = SIMON_ANI_IDLE;
		}
		else
			ani = SIMON_ANI_JUMP;
	}
	else if (state == SIMON_STATE_ATTACK) ani = SIMON_ANI_ATTACK;
	else if (state == SIMON_STATE_SIT_AND_ATTACK) ani = SIMON_ANI_SIT_AND_ATTACK;
	else if (state == SIMON_STATE_ATTACK_UPSTAIR) ani = SIMON_ANI_ATTACK_UPSTAIR;
	else if (state == SIMON_STATE_ATTACK_DOWNSTAIR) ani = SIMON_ANI_ATTACK_DOWNSTAIR;
	else if (state == SIMON_STATE_DEFLECT) ani = SIMON_ANI_DEFLECT;
	else if (state == SIMON_STATE_THROW)
	{	
			if (isOnStair)
			{
				if (isUpstair)
				{
					ani = SIMON_ANI_THROW_UP_STAIR;
				}
				else
				{
					ani = SIMON_ANI_THROW_DOWN_STAIR;
				}
			}
			else
				ani = SIMON_ANI_THROW;
	}
	else if (state == SIMON_STATE_ONSTAIR)
	{
		if (isUpstair)
		{
			if (isStopOnStair)
			{
				ani = SIMON_ANI_UPSTAIR_STOP; // ani = 6;
			}
			else
			{
				ani = SIMON_ANI_UPSTAIR; //ani = 7
			}
		}
		else if (isUpstair == false)
		{
			if (isStopOnStair)
			{
				ani = SIMON_ANI_DOWNSTAIR_STOP; //ani = 8;
			}
			else
			{
				ani = SIMON_ANI_DOWNSTAIR; //ani = 9;
			}
		}
	}
	else
	{
		if (vx == 0)
		{
			if (isEatingItem)
			{
				DebugOut(L"OK \n");
				ani = SIMON_ANI_CHANGECOLOR;
			}
			else
			{
				ani = SIMON_ANI_IDLE;
			}

		}
		else
		{
			if (CGame::GetInstance()->IsKeyDown(DIK_RIGHT) || CGame::GetInstance()->IsKeyDown(DIK_LEFT) || this->isWalkingToStair == true)
			{
				ani = SIMON_ANI_WALKING;	
			}
			else
				ani = SIMON_ANI_IDLE;
		}

	}

	int alpha = 255;
	if (untouchable)
	{
		alpha = rand() % 255;
	}
	animation_set->at(ani)->Render(x, y, nx, alpha);
	
	//RenderBoundingBox();
	if (state == SIMON_STATE_ATTACK || state == SIMON_STATE_SIT_AND_ATTACK || state == SIMON_STATE_ATTACK_UPSTAIR || state == SIMON_STATE_ATTACK_DOWNSTAIR)
		whip->Render(animation_set->at(ani)->GetCurrentFrame());
}
void Simon::SetState(int state)
{
	CGameObject::SetState(state);
	switch (state)
	{
	case SIMON_STATE_ONSTAIR:
		if (isUpstair)
		{
			nx = StairDirection;
			if (nx > 0)
			{
				//simon đi lên hướng phải
				vx = 0.04f;
				vy = -0.04f;
			}
			else
			{
				//simon đi lên hướng trái
				vx = -0.04f;
				vy = -0.04f;
			}
		}
		else if (isUpstair == false)
		{
			nx = -StairDirection;
			if (nx > 0)
			{
				//simon xuống hướng phải
				vx = 0.04f;
				vy = 0.04f;
			}
			else
			{
				//simon đi xuống hướng trái
				vx = -0.04f;
				vy = 0.04f;
			}
		}
		if (isStopOnStair)
		{
			//đang đứng ở cầu thang
			vx = 0;
			vy = 0;
			isSitAttack = false;
			isAttack = false;
		}
		//////////////////
		break;
	case SIMON_STATE_WALKING:
		if (isWalkingToStair)
		{
			if (nx > 0)
				vx = SIMON_WALKING_TO_STAIR_SPEED;
			else
				vx = -SIMON_WALKING_TO_STAIR_SPEED;
		}
		else
		{
			if (isJumping) return;
			if (nx > 0)
			{
				vx = SIMON_WALKING_SPEED;
			}
			else
			{
				vx = -SIMON_WALKING_SPEED;
			}
		}
			break;
	case SIMON_STATE_THROW:
	{
			animation_set->at(SIMON_ANI_THROW)->Reset();
			animation_set->at(SIMON_ANI_THROW)->SetAniStartTime(GetTickCount());
		break;
	}
	case SIMON_STATE_ATTACK_UPSTAIR:
	{
		isStanding = true;
		animation_set->at(SIMON_ANI_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_DOWNSTAIR)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_UPSTAIR)->Reset();
		animation_set->at(SIMON_ANI_SIT_AND_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_UPSTAIR)->SetAniStartTime(GetTickCount());
		break;
	}
	case SIMON_STATE_ATTACK_DOWNSTAIR:
	{
		isStanding = true;
		animation_set->at(SIMON_ANI_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_DOWNSTAIR)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_UPSTAIR)->Reset();
		animation_set->at(SIMON_ANI_SIT_AND_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_DOWNSTAIR)->SetAniStartTime(GetTickCount());
		break;
	}
	case SIMON_STATE_SIT:
	{
		isStanding = false;
		if(!isOnMoving)
		vx = 0;
		break;
	}
	case SIMON_STATE_JUMP:
	{
		// TODO: need to check if Mario is *current* on a platform before allowing to jump again
		//isStanding = true;
		isStanding = true;
		vy = -SIMON_JUMP_SPEED_Y;
		break;
	}
	case SIMON_STATE_ATTACK:
	{
		animation_set->at(SIMON_ANI_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_SIT_AND_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_DOWNSTAIR)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_UPSTAIR)->Reset();
		animation_set->at(SIMON_ANI_ATTACK)->SetAniStartTime(GetTickCount());
		break;
	}
	case SIMON_STATE_SIT_AND_ATTACK:
	{
		isStanding = false;
		animation_set->at(SIMON_ANI_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_DOWNSTAIR)->Reset();
		animation_set->at(SIMON_ANI_ATTACK_UPSTAIR)->Reset();
		animation_set->at(SIMON_ANI_SIT_AND_ATTACK)->Reset();
		animation_set->at(SIMON_ANI_SIT_AND_ATTACK)->SetAniStartTime(GetTickCount());
		break;
	}
	case SIMON_STATE_IDLE:
	{
		isStanding = true;
		isSitAttack = false;
		isAttack = false;
		if(!isOnMoving)
		vx = 0;
		break;
	}
	case SIMON_STATE_DEFLECT:
	{
		isHitEnemy = true;
		vx = vy = dx = dy = 0;
		this->vx = (-this->nx) * SIMON_DEFLECT_SPEED_X;
		vy = -SIMON_DEFLECT_SPEED_Y;
		break;
	}
	case SIMON_STATE_DIE:
		vy = SIMON_DIE_DEFLECT_SPEED;
		break;
	}
}
void Simon::GetBoundingBox(float &left, float &top, float &right, float &bottom)
{
	if (state == SIMON_STATE_SIT || state == SIMON_STATE_SIT_AND_ATTACK)
	{
		left = x;
		top = y + 7;
		right = x + SIMON_BBOX_WIDTH;
		bottom = y + SIMON_BBOX_HEIGHT;
	}
	else if (state == SIMON_STATE_JUMP)
	{
		left = x;
		top = y;
		right = x + SIMON_BBOX_WIDTH;
		bottom = y + SIMON_BBOX_HEIGHT;
	}
	else if (state == SIMON_STATE_DIE)
	{
		left = x;
		top = y;
		right = x + 32;
		bottom = y + 15;
	}
	else
	{
		left = x;
		top = y;
		right = x + SIMON_BBOX_WIDTH;
		bottom = y + SIMON_BBOX_HEIGHT;

	}
}

/*
	Reset Mario status to the beginning state of a scene
*/
void Simon::Reset()
{
	SetState(SIMON_STATE_IDLE);
	this->isOnStair = false;
	SetPosition(start_x, start_y);
	SetSpeed(0, 0);
}
RECT Simon::GetBound()
{
	RECT rect;
	float l, t, r, b;
	GetBoundingBox(l, t, r, b);
	rect.left = l;
	rect.top = t;
	rect.right = r;
	rect.bottom = b;
	return rect;
}
void Simon::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	CGameObject::Update(dt, coObjects);
	if (this->health == 0)
	{
			SetState(SIMON_STATE_DIE);
	}
	if (isHitEnemy)
	{
		if (timerDeflect < 700)
		{
			timerDeflect += dt;
		}
		else
		{
			SetState(SIMON_STATE_IDLE);
			timerDeflect = 0;
			if(untouchable==0)
			{
				StartUntouchable();
			}
			isHitEnemy = false;
		}
	}
	if (state == SIMON_STATE_DIE)
	{
		vx = 0;
		if (timerDie < 2500)
			timerDie += dt;
		else
		{
			timerDie = 0;
			isRevive = true;
		}
	}
	if (isEatingItem)
	{
		if (timerChangeColor < 700)
			timerChangeColor += dt;
		else
		{
			isEatingItem = false;
			timerChangeColor = 0;
		}
	}
	if (state == SIMON_STATE_ATTACK || state == SIMON_STATE_SIT_AND_ATTACK || state == SIMON_STATE_THROW || state == SIMON_STATE_ATTACK_UPSTAIR || state == SIMON_STATE_ATTACK_DOWNSTAIR)
	{
		int ani = -1;
		if (state == SIMON_STATE_ATTACK)
			ani = SIMON_ANI_ATTACK;
		else if (state == SIMON_STATE_SIT_AND_ATTACK)
		{
			ani = SIMON_ANI_SIT_AND_ATTACK;
		}
		else if (state == SIMON_STATE_THROW)
		{
			ani = SIMON_ANI_THROW;
		}
		else if (state == SIMON_STATE_ATTACK_UPSTAIR)
		{
			ani = SIMON_ANI_ATTACK_UPSTAIR;
		}
		else if (state == SIMON_STATE_ATTACK_DOWNSTAIR)
		{
			ani = SIMON_ANI_ATTACK_DOWNSTAIR;
		}
		whip->SetOrientation(nx);
		whip->SetWhipPosition(D3DXVECTOR2(x, y), isStanding);
		if (animation_set->at(ani)->GetCurrentFrame() == 2)	 
		{
			if (!isOnStair)
			{
				SetState(SIMON_STATE_IDLE);
			}
			else
			{
				SetState(SIMON_STATE_ONSTAIR);
			}
			//whip->Update(dt, coObjects);
		}
	}
	//simon không ở trên cầu thang
	if (!isOnStair)
	{
		vy += SIMON_GRAVITY * dt;
		CheckCollisionWithGround(dt, coObjects);

		isWalkingToStair = false;  //chỉnh speed lại thành đi bth 
	}
	else
	{
		x += dx;
		y += dy;
	}
	if (isOnStair)
	{
		if (isHitTopStair == true && isUpstair == true)
		{
			CheckCollisionOnStair(dt, coObjects);
		}
		else if (isHitBottomStair == true && isUpstair == false)
		{
			CheckCollisionOnStair(dt, coObjects);
		}
	}
}
void Simon::CheckCollisionWithGround(DWORD dt, vector<LPGAMEOBJECT>* colliable_objects)
{

	if (isLand)
	{
		SetState(SIMON_STATE_SIT);

		if (timerLand < 300)
			timerLand += dt;
		else
		{
			timerLand = 0;
			isLand = false;
			isJumping = false;
			SetState(SIMON_STATE_IDLE);
		}

	}
	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	coEvents.clear();

	if (GetTickCount() - untouchable_start > SIMON_UNTOUCHABLE_TIME)
	{
		untouchable_start = 0;
		untouchable = 0;
	}
	CalcPotentialCollisions(colliable_objects, coEvents);

	if (coEvents.size() == 0)
	{
		x += dx; //dx=vx*dt
		y += dy;
	}
	else
	{
		isOnGround = true;
		float min_tx, min_ty, nx = 0, ny;

		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny);

		//block 	
		x += min_tx * dx + nx * 0.1f;
		y += min_ty * dy + ny * 0.1f;
		for (UINT i = 0; i < coEventsResult.size(); i++)
		{
			LPCOLLISIONEVENT  e = coEventsResult[i];
			if (dynamic_cast<MovingPlatform*>(e->obj))
			{
				MovingPlatform* m = dynamic_cast<MovingPlatform*> (e->obj);	
				if (ny != 0)
				{			
					y += ny * 0.8f;
					if (ny == -1)
					{
						isOnMoving = true;
						this->vx = m->vx;
						vy = 0;
					}	
				}
			}
			else if  (dynamic_cast<CBrick*>(e->obj))
			{
				isOnMoving = false;
				if (ny != 0)
				{
					//vy = 0.3f;
					if (ny == -1)
					{
						vy = 0;
						continue;
					}
					else
						y += dy;
				}
				if (e->nx != 0)
				{
					x += 0;
	
				}
			}
			/*else
			{
				isOnMoving = false;
			}*/
		}
		if (ny == -1)
		{
			//nếu simon rớt xuống độ cao lớn hơn 50 thì ngồi xuống
			if (this->GetPosition().y - firstY >= 50)
			{
				isLand = true;
			}
			vy = 0.5f;
			firstY = this->GetPosition().y;
		}
		if (ny == -1 && isJumping == true)
		{
			if (isAttack == false) //set đk này để khi chạm đất mà đánh chưa xong thì phải hoàn thành hd đánh r mới đứng yên
				SetState(SIMON_STATE_IDLE);

			isJumping = false;
		}
	}
	// clean up collision events
	for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];
}
void Simon::CheckCollisionOnStair(DWORD dt, vector<LPGAMEOBJECT>* colliable_objects)
{
	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;


	y -= 20.0f;  //kéo vy lên 1 đoạn
	vy = 100000.0f;  //kéo xuống lại ngay lập tức 
	//(diễn ra trong 1 frame)

	CGameObject::Update(dt);

	coEvents.clear();

	CalcPotentialCollisions(colliable_objects, coEvents);

	if (coEvents.size() == 0)
	{
		y += dy;
	}
	else
	{
		float min_tx, min_ty, nx = 0, ny;

		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny);

		y += min_ty * dy + ny * 0.2f;

		if (ny != 0)
		{
			vy = 0;
			vx = 0;
			isOnStair = false;
			isHitTopStair = false;
			isHitBottomStair = false;
			isStopOnStair = false;
			isWalkingToStair = false;
			SetState(SIMON_STATE_IDLE);
		}
	}

	vy = SIMON_GRAVITY * dt;
	CGameObject::Update(dt);
}
