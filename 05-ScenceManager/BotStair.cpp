#include "BotStair.h"
BotStair::BotStair()
{
}
void BotStair::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x;
	top = y;
	right = x + BOTSTAIR_BBOX_WIDTH;
	bottom = y + BOTSTAIR_BBOX_HEIGHT;
}
void BotStair::Render()
{
	RenderBoundingBox();
}