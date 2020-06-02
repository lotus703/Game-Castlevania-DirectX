#include "MovingPlatform.h"

MovingPlatform::MovingPlatform() : CGameObject()
{
	vx = MOVING_PLATFORM_SPEED;
	vy = 0;
}
void MovingPlatform::Render()
{
	animation_set->at(0)->Render(x, y, 1);
}
void MovingPlatform::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	CGameObject::Update(dt);
	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;

	coEvents.clear();
	CalcPotentialCollisions(coObjects, coEvents);

	if (coEvents.size() == 0)
	{
		y += dy;
		x += dx;
	}
	else
	{
		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0;
		float rdy = 0;
		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny);

		x += min_tx * dx;
		//y += min_ty * dy + ny * 0.4f;

		for (UINT i = 0; i < coEventsResult.size(); i++)
		{
			LPCOLLISIONEVENT e = coEventsResult[i];
			// Collision with brick
			if (dynamic_cast<CBrick*>(e->obj))
			{
				if (e->nx != 0 || e->ny != 0)
				{
					//DebugOut(L" Redirecting Moving Platform\n");
					vx = -vx;
				}


			}
		}
	}

	// clean up collision events
	for (int i = 0; i < coEvents.size(); i++) delete coEvents[i];
}
void MovingPlatform::GetBoundingBox(float &l, float &t, float &r, float &b)
{
	l = x;
	t = y;
	r = x + MOVING_PLATFORM_BBOX_WIDTH;
	b = y + MOVING_PLATFORM_BBOX_HEIGHT;
}
RECT MovingPlatform::GetBound()
{
	return CGameObject::GetBound();
}