#include "HolyWaterItem.h"
#include "Brick.h"
HolyWaterItem::HolyWaterItem()
{
	this->visible = false;
}
void HolyWaterItem::Render()
{
	animation_set->at(0)->Render(x, y, nx);
	//RenderBoundingBox();
}
void HolyWaterItem::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjects)
{
	CGameObject::Update(dt);
	vy += ITEM_GRAVITY * dt;				// simple fall down
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
		float rdx, rdy;
		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny);

		x += min_tx * dx;
		y += min_ty * dy;

		for (UINT i = 0; i < coEventsResult.size(); ++i)
		{
			LPCOLLISIONEVENT e = coEventsResult[i];

			if (dynamic_cast<CBrick*>(e->obj))
			{
				// Block brick
				if (e->ny != 0)
				{
					y += 0.1f * e->ny;
					vy = 0;
				}
			}
		}
	}

	// clean up collision events
	for (UINT i = 0; i < coEvents.size(); i++) delete coEvents[i];
}
void HolyWaterItem::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x;
	top = y;
	right = x + HOLYWATER_ITEM_BBOX_WIDTH;
	bottom = y + HOLYWATER_ITEM_BBOX_WIDTH;
}