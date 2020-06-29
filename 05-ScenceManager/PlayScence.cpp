﻿#include <iostream>
#include <fstream>

#include "PlayScence.h"
#include "Utils.h"
#include "Textures.h"
#include "Sprites.h"
#include "Portal.h"

using namespace std;
//CPlayScene::CPlayScene() : CScene(this->id, this->sceneFilePath)
//{
//	
//	key_handler = new CPlayScenceKeyHandler(this);
//}
CPlayScene::CPlayScene(int id, LPCWSTR filePath):CScene(id, filePath)
{
	key_handler = new CPlayScenceKeyHandler(this);
}


/*
	Load scene resources from scene file (textures, sprites, animations and objects)
	See scene1.txt, scene2.txt for detail format specification
*/

#define SCENE_SECTION_UNKNOWN -1
#define SCENE_SECTION_TILE_SHEET 1
#define SCENE_SECTION_SPRITES 2
#define SCENE_SECTION_ANIMATIONS 3
#define SCENE_SECTION_ANIMATION_SETS 4
#define SCENE_SECTION_OBJECTS	6
#define SCENE_SECTION_MAP_INFO				7
#define SCENE_SECTION_MAPS 8

#define OBJECT_TYPE_SIMON						0
#define OBJECT_TYPE_BRICK						1	 
#define OBJECT_TYPE_TORCH					2	 
#define OBJECT_TYPE_WHIP						3	 
#define OBJECT_TYPE_ITEM_HEART		4	 
#define OBJECT_TYPE_ITEM_CHAIN			5	 
#define OBJECT_TYPE_ITEM_DAGGER			6	 
#define OBJECT_TYPE_DAGGER					7	 
#define OBJECT_TYPE_KNIGHT		8 
#define OBJECT_TYPE_BOTSTAIR	9
#define OBJECT_TYPE_TOPSTAIR	10
#define OBJECT_TYPE_ITEM_BOOMERANG	11
#define OBJECT_TYPE_BOOMERANG					12	
#define OBJECT_TYPE_BREAKABLE_BRICK	13
#define OBJECT_TYPE_CROWN_ITEM	14
#define	OBJECT_TYPE_ROCKS	15
#define OBJECT_TYPE_BAT	16
#define OBJECT_TYPE_MOVING_PFLATFORM	17
#define OBJECT_TYPE_ITEM_SMALL_HEART 18
#define OBJECT_TYPE_ITEM_MONEY_BAG 19
#define OBJECT_TYPE_ITEM_HOLYWATER 20
#define OBJECT_TYPE_ITEM_AXE 21
#define OBJECT_TYPE_ITEM_WATCH 22
#define OBJECT_TYPE_HOLYWATER					23
#define OBJECT_TYPE_AXE	24

#define OBJECT_TYPE_PORTAL	50

#define MAX_SCENE_LINE 1024


/*
	Parse a line in section [OBJECTS] 
*/
void CPlayScene::_ParseSection_TILE_SHEET(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 5) return; // skip invalid lines

	int texID = atoi(tokens[0].c_str());
	wstring path = ToWSTR(tokens[1]);
	int R = atoi(tokens[2].c_str());
	int G = atoi(tokens[3].c_str());
	int B = atoi(tokens[4].c_str());
	CTextures::GetInstance()->Add(texID, path.c_str(), D3DCOLOR_XRGB(R, G, B));
}

void CPlayScene::_ParseSection_SPRITES(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 6) return; // skip invalid lines

	int ID = atoi(tokens[0].c_str());
	int l = atoi(tokens[1].c_str());
	int t = atoi(tokens[2].c_str());
	int r = atoi(tokens[3].c_str());
	int b = atoi(tokens[4].c_str());
	int texID = atoi(tokens[5].c_str());

	LPDIRECT3DTEXTURE9 tex = CTextures::GetInstance()->Get(texID);
	if (tex == NULL)
	{
		DebugOut(L"[ERROR] Texture ID %d not found!\n", texID);
		return;
	}

	CSprites::GetInstance()->Add(ID, l, t, r, b, tex);
}

void CPlayScene::_ParseSection_ANIMATIONS(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 3) return; // skip invalid lines - an animation must at least has 1 frame and 1 frame time

	//DebugOut(L"--> %s\n",ToWSTR(line).c_str());

	LPANIMATION ani = new CAnimation();

	int ani_id = atoi(tokens[0].c_str());
	for (int i = 1; i < tokens.size(); i += 2)	// why i+=2 ?  sprite_id | frame_time  
	{
		int sprite_id = atoi(tokens[i].c_str());
		int frame_time = atoi(tokens[i + 1].c_str());
		ani->Add(sprite_id, frame_time);
	}

	CAnimations::GetInstance()->Add(ani_id, ani);
}

void CPlayScene::_ParseSection_ANIMATION_SETS(string line)
{
	vector<string> tokens = split(line);

	if (tokens.size() < 2) return; // skip invalid lines - an animation set must at least id and one animation id

	int ani_set_id = atoi(tokens[0].c_str());

	LPANIMATION_SET s = new CAnimationSet();

	CAnimations* animations = CAnimations::GetInstance();

	for (int i = 1; i < tokens.size(); i++)
	{
		int ani_id = atoi(tokens[i].c_str());

		LPANIMATION ani = animations->Get(ani_id);
		s->push_back(ani);
	}

	CAnimationSets::GetInstance()->Add(ani_set_id, s);
}
void CPlayScene::_ParseSection_OBJECTS(string line)
{
	vector<string> tokens = split(line);

	//DebugOut(L"--> %s\n",ToWSTR(line).c_str());

	if (tokens.size() < 3) return; // skip invalid lines - an object set must have at least id, x, y

	int object_type = atoi(tokens[0].c_str());
	float x = atof(tokens[1].c_str());
	float y = atof(tokens[2].c_str());

	int ani_set_id = atoi(tokens[3].c_str());

	CAnimationSets * animation_sets = CAnimationSets::GetInstance();

	CGameObject *obj = NULL;
	switch (object_type)
	{
	case OBJECT_TYPE_SIMON:
		if (player!=NULL) 
		{
			DebugOut(L"[ERROR] MARIO object was created before!\n");
			return;
		}
		obj = Simon::GetInstance();
		player = (Simon*)obj;  
		DebugOut(L"[INFO] Player object created!\n");
		break;
	case OBJECT_TYPE_CROWN_ITEM:
	{
		obj = new CrownItem();
		//Items::GetInstance()->AddItem(OBJECT_TYPE_CROWN_ITEM, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_BRICK: 
	{
		int width = atoi(tokens[4].c_str());
		int height = atoi(tokens[5].c_str());
		obj = new CBrick();
		obj->SetWidth(width);
		obj->SetHeight(height);
		listBrick.push_back(obj);
		break;
	}
	case OBJECT_TYPE_WHIP: obj = new Whip(); break;
	case OBJECT_TYPE_BAT: 
		obj = new Bat(x,y);
		listEnemy.push_back(obj);
		break;
	case OBJECT_TYPE_DAGGER: 
	{
		obj = new Dagger();
		dagger = (Dagger*)obj;
		obj->visible = false;
		listWeapon.push_back(obj);
		break;
	}
	case OBJECT_TYPE_BOOMERANG:
	{
		obj = new Boomerang();
		boomerang = (Boomerang*)obj;
		obj->visible = false;
		listWeapon.push_back(obj);
		break;
	}
	case OBJECT_TYPE_HOLYWATER:
	{
		obj = new HolyWater();
		holywater = (HolyWater*)obj;
		obj->visible = false;
		listWeapon.push_back(obj);
		break;
	}
	case OBJECT_TYPE_AXE:
	{
		obj = new Axe();
		axe = (Axe*)obj;
		obj->visible = false;
		listWeapon.push_back(obj);
		break;
	}
	case OBJECT_TYPE_KNIGHT: 
		obj = new Knight(x,y);
		listEnemy.push_back(obj);
		break;
	case OBJECT_TYPE_ROCKS: 
		obj = new Rock();
		Rocks::GetInstance()->AddRock((Rock*)obj);
		listEfect.push_back(obj);
		break;
	case OBJECT_TYPE_MOVING_PFLATFORM: 
		obj = new MovingPlatform();
		listBrick.push_back(obj);
		break;
	case OBJECT_TYPE_BOTSTAIR:
	{
		obj = new BotStair(); 
		int direction = atoi(tokens[3].c_str());
		DebugOut(L"Bot stair direction is: %d  \n", direction);
		if(direction == 1)
		obj->StairTag = CGameObject::StairTypes::ToRight;
		else
		obj->StairTag = CGameObject::StairTypes::ToLeft;
		listStair.push_back(obj);
		break;
	}
	case OBJECT_TYPE_TOPSTAIR:
	{
		obj = new TopStair();
		int direction = atoi(tokens[3].c_str());
		DebugOut(L"Top stair direction is: %d  \n", direction);
		if (direction == 1)
		obj->StairTag = CGameObject::StairTypes::ToRight;
		else		
		obj->StairTag = CGameObject::StairTypes::ToLeft;
		listStair.push_back(obj);
		break;
	}
	case OBJECT_TYPE_TORCH: 
	{
		int i = atoi(tokens[4].c_str());
		int state = atoi(tokens[5].c_str());
		obj = new Torch();
		obj->SetState(state);
		obj->SetItemId(i);
		listTorch.push_back(obj);
		break;
	}
	case OBJECT_TYPE_BREAKABLE_BRICK:
	{
		obj = new BreakableBrick();
		listEnemy.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_HEART: 
	{
		obj = new HeartItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_HEART, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_SMALL_HEART:
	{
		obj = new SmallHeartItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_SMALL_HEART, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_MONEY_BAG:
	{
		int state = atoi(tokens[4].c_str());
		obj = new MoneyBagItem();
		obj->SetState(state);
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_MONEY_BAG, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_CHAIN:
	{
		obj = new ChainItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_CHAIN, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_DAGGER:
	{
		obj = new DaggerItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_DAGGER, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_BOOMERANG:
	{
		obj = new BoomerangItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_BOOMERANG, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_HOLYWATER:
	{
		obj = new HolyWaterItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_HOLYWATER, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_AXE:
	{
		obj = new AxeItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_AXE, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_ITEM_WATCH:
	{
		obj = new WatchItem();
		Items::GetInstance()->AddItem(OBJECT_TYPE_ITEM_WATCH, obj);
		listItem.push_back(obj);
		break;
	}
	case OBJECT_TYPE_PORTAL:
		{	
			float r = atof(tokens[4].c_str());
			float b = atof(tokens[5].c_str());
			scene_id = atoi(tokens[6].c_str());
			obj = new CPortal(x, y, r, b, scene_id);
			listPortal.push_back(obj);
		}
		break;
	default:
		DebugOut(L"[ERR] Invalid object type: %d\n", object_type);
		return;
	}

	// General object setup
	obj->SetPosition(x, y);

	LPANIMATION_SET ani_set = animation_sets->Get(ani_set_id);

	obj->SetAnimationSet(ani_set);
	//objects.push_back(obj);
}
void CPlayScene::_ParseSection_MAP_INFO(string line)
{
	vector<string> tokens = split(line);
	if (tokens.size() < 2) return; // skip invalid lines
	this->tileColumns = atoi(tokens[0].c_str());
	this->tileRows = atoi(tokens[1].c_str());
	this->map_width = atoi(tokens[2].c_str());
}
void CPlayScene::_ParseSection_MAP(string line)
{
	vector<string> tokens = split(line);
	if (tokens.size() < 4) return; // skip invalid lines - an object set must have at least id, x, y
	for (int i = 0; i < tokens.size(); i++)
	{
		RECT rectTile;
		int index = atoi(tokens[i].c_str());
		if (index >= 0)
		{
			rectTile.left = (index % tileColumns) * TILE_WIDTH;
			rectTile.top = (index / tileColumns) * TILE_HEIGHT;
			rectTile.right = rectTile.left + TILE_WIDTH;
			rectTile.bottom = rectTile.top + TILE_HEIGHT;
			int x, y;
			x = i * TILE_WIDTH;
			y = this->tileMapLineY;
			Map* map = new Map(x, y, rectTile.left, rectTile.top, rectTile.right, rectTile.bottom);
			tileMap.push_back(map);
		}
	}
	this->tileMapLineY += TILE_HEIGHT;
}
void CPlayScene::Load()
{
	DebugOut(L"[INFO] Start loading scene resources from : %s \n", sceneFilePath);
	HUD = Board::GetInstance();
	grid = new Grid();
	

	DebugOut(L"[INFO] Done loading scene resources %s\n", sceneFilePath);
	if (CScene::GetSceneId() == 1)
	{
		ifstream f;
		f.open(sceneFilePath);

		// current resource section flag
		int section = SCENE_SECTION_UNKNOWN;

		char str[MAX_SCENE_LINE];
		while (f.getline(str, MAX_SCENE_LINE))
		{
			string line(str);

			if (line[0] == '#') continue;	// skip comment lines	
			if (line == "[TILESHEET]") {
				section = SCENE_SECTION_TILE_SHEET; continue;
			}
			if (line == "[SPRITES]") {
				section = SCENE_SECTION_SPRITES; continue;
			}
			if (line == "[ANIMATIONS]") {
				section = SCENE_SECTION_ANIMATIONS; continue;
			}
			if (line == "[ANIMATIONS_SETS]") {
				section = SCENE_SECTION_ANIMATION_SETS; continue;
			}
			if (line == "[OBJECTS]") {
				section = SCENE_SECTION_OBJECTS; continue;
			}
			if (line == "[MAP_INFO]") {
				section = SCENE_SECTION_MAP_INFO; continue;
			}
			if (line == "[MAP]") {
				section = SCENE_SECTION_MAPS; continue;
			}
			if (line[0] == '[') { section = SCENE_SECTION_UNKNOWN; continue; }

			//
			// data section
			//

			switch (section)
			{
			case SCENE_SECTION_OBJECTS: _ParseSection_OBJECTS(line); break;
			case SCENE_SECTION_MAP_INFO: _ParseSection_MAP_INFO(line); break;
			case SCENE_SECTION_MAPS:	_ParseSection_MAP(line); break;
			case SCENE_SECTION_TILE_SHEET: _ParseSection_TILE_SHEET(line); break;
			case SCENE_SECTION_SPRITES: _ParseSection_SPRITES(line); break;
			case SCENE_SECTION_ANIMATIONS: _ParseSection_ANIMATIONS(line); break;
			case SCENE_SECTION_ANIMATION_SETS: _ParseSection_ANIMATION_SETS(line); break;
			}
		}

		f.close();

		CTextures::GetInstance()->Add(ID_TEX_BBOX, L"textures\\bbox.png", D3DCOLOR_XRGB(255, 0, 255));
		for (UINT i = 0; i < listTorch.size(); i++)
		{
			grid->InsertIntoGrid(listTorch.at(i));
		}
		for (UINT i = 0; i < listBrick.size(); i++)
		{
			grid->InsertIntoGrid(listBrick.at(i));
		}
	}
	else
	{
		listBrick.clear();
		listTorch.clear();
		listPortal.clear();
		listEnemy.clear();
		listStair.clear();
		listItem.clear();
		ifstream f;
		f.open(sceneFilePath);

		// current resource section flag
		int section = SCENE_SECTION_UNKNOWN;

		char str[MAX_SCENE_LINE];
		while (f.getline(str, MAX_SCENE_LINE))
		{
			string line(str);

			if (line[0] == '#') continue;	// skip comment lines	
			if (line == "[TILESHEET]") {
				section = SCENE_SECTION_TILE_SHEET; continue;
			}
			if (line == "[SPRITES]") {
				section = SCENE_SECTION_SPRITES; continue;
			}
			if (line == "[ANIMATIONS]") {
				section = SCENE_SECTION_ANIMATIONS; continue;
			}
			if (line == "[ANIMATIONS_SETS]") {
				section = SCENE_SECTION_ANIMATION_SETS; continue;
			}
			if (line == "[OBJECTS]") {
				section = SCENE_SECTION_OBJECTS; continue;
			}
			if (line == "[MAP_INFO]") {
				section = SCENE_SECTION_MAP_INFO; continue;
			}
			if (line == "[MAP]") {
				section = SCENE_SECTION_MAPS; continue;
			}
			if (line[0] == '[') { section = SCENE_SECTION_UNKNOWN; continue; }

			//
			// data section
			//

			switch (section)
			{
			case SCENE_SECTION_OBJECTS: _ParseSection_OBJECTS(line); break;
			case SCENE_SECTION_MAP_INFO: _ParseSection_MAP_INFO(line); break;
			case SCENE_SECTION_MAPS:	_ParseSection_MAP(line); break;
			case SCENE_SECTION_TILE_SHEET: _ParseSection_TILE_SHEET(line); break;
			case SCENE_SECTION_SPRITES: _ParseSection_SPRITES(line); break;
			case SCENE_SECTION_ANIMATIONS: _ParseSection_ANIMATIONS(line); break;
			case SCENE_SECTION_ANIMATION_SETS: _ParseSection_ANIMATION_SETS(line); break;
			}
		}

		f.close();

		CTextures::GetInstance()->Add(ID_TEX_BBOX, L"textures\\bbox.png", D3DCOLOR_XRGB(255, 0, 255));
	
		for (UINT i = 0; i < listBrick.size(); i++)
		{
			grid->InsertIntoGrid(listBrick.at(i));
		}
		for (UINT i = 0; i < listTorch.size(); i++)
		{
			grid->InsertIntoGrid(listTorch.at(i));
		}
		for (UINT i = 0; i < listStair.size(); i++)
		{
			grid->InsertIntoGrid(listStair.at(i));
		}
	}
}

void CPlayScene::Update(DWORD dt)
{
	// We know that Mario is the first object in the list hence we won't add him into the colliable object list
	// TO-DO: This is a "dirty" way, need a more organized way 
	grid->GetListCollisionFromGrid(listColObjects);
	DebugOut(L"Size of listCoObjects is %d \n", listColObjects.size());
	listStair.clear();
	listTorch.clear();
	listBrick.clear();
	for (UINT i = 0; i < listColObjects.size(); i++)
	{
		LPGAMEOBJECT temp = listColObjects.at(i);
		if (dynamic_cast<Torch*>(temp))
			listTorch.push_back(listColObjects.at(i));
		else
			if (dynamic_cast<TopStair*>(temp) || dynamic_cast<BotStair*>(temp))
				listStair.push_back(listColObjects.at(i));
			else
				listBrick.push_back(listColObjects.at(i));
	}
	CheckCollision_ItemAndSimon();
	CheckCollision_WhipAndEnemy();
	CheckCollision_PortalAndSimon();
	CheckCollision_DaggerAndTorch();
	CheckCollision_SimonAndBoomerang();
	CheckCollision_SimonAndEnemy();
	Handle_SimonAndMoving();
	player->Update(dt, &listBrick);
	for (UINT i = 0; i < listTorch.size(); i++)
	{
		if (listTorch[i]->isVisible() == true)
			listTorch[i]->Update(dt, &listBrick);
	}
	for (UINT i = 0; i < listItem.size(); i++)
	{
		if (listItem[i]->isVisible() == true)
			listItem[i]->Update(dt, &listBrick);
	}
	for (UINT i = 0; i < listEfect.size(); i++)
	{
		if (listEfect[i]->isVisible() == true)
			listEfect[i]->Update(dt, &listBrick);
	}
	for (UINT i = 0; i < listWeapon.size(); i++)
	{
		if (listWeapon[i]->isVisible() == true)
			listWeapon[i]->Update(dt, &listBrick);
	}
	for (UINT i = 0; i < listBrick.size(); i++)
	{
			listBrick[i]->Update(dt, &listBrick);
	}
	for (UINT i = 0; i < listEnemy.size(); i++)
	{
		if (listEnemy[i]->isVisible() == true)
			listEnemy[i]->Update(dt, &listBrick);
	}
	// skip the rest if scene was already unloaded (Mario::Update might trigger PlayScene::Unload)
	if (player == NULL) return; 

	// Update camera to follow simon
	float cx, cy;
	player->GetPosition(cx, cy); //50.0f, 0.0f
	if(cx <= 0)
	{
		player->x = 0;
	}
	CGame *game = CGame::GetInstance();
	/*if ((cx > game->GetScreenWidth()/2))
	{*/
	
	 if(cx >= game->GetScreenWidth() / 2)
	 {
		
		cx -= game->GetScreenWidth() / 2;
		cy -= game->GetScreenHeight() / 2;
		if (cx > (map_width - (game->GetScreenWidth())))
		{
			cx = map_width - (game->GetScreenWidth());
		}
		
	 }
	else if (cx < game->GetScreenWidth() / 2)
	 {
		 cx = 0.0f;
	 }
	 if (player->isRevive)
	 {
		 player->isRevive = false;

		 player->SetVisible(true);
		 player->SetState(SIMON_STATE_IDLE);
		 player->SetHealth(16);
		 player->numLife--;
		 HUD->SetTimeHud(300);
		 //NOTE chỉ mới xử lí ở scene 2, chưa xử lí các scene khác
		 if (game->GetSceneId() == 2)
			 player->SetPosition(32, 130);
	 }
	 HUD->Update(dt);
	CGame::GetInstance()->SetCamPos(cx, -40.0f /*cy*/);
}

void CPlayScene::Render()
{
		for (int i = 0; i < tileMap.size(); i++)
		{
			tileMap[i]->Render();
		}
		for (UINT i = 0; i < listPortal.size(); i++)
		{
			listPortal.at(i)->Render();
		}
		for (UINT i = 0; i < listItem.size(); i++)
		{
			if(listItem.at(i)->isVisible()==true)
			listItem.at(i)->Render();
		}
		for (UINT i = 0; i < listEfect.size(); i++)
		{
			if (listEfect.at(i)->isVisible() == true)
				listEfect.at(i)->Render();
		}
		for (UINT i = 0; i < listWeapon.size(); i++)
		{
			if (listWeapon.at(i)->isVisible() == true)
				listWeapon.at(i)->Render();
		}
		for (UINT i = 0; i < listTorch.size(); i++)
		{
			listTorch.at(i)->Render();
		}
		for (UINT i = 0; i < listStair.size(); i++)
		{
			listStair.at(i)->Render();
		}
		for (UINT i = 0; i < listEnemy.size(); i++)
		{
			if(listEnemy.at(i)->isVisible() == true)
			listEnemy.at(i)->Render();
		}
		//Vẽ gạch
		for (UINT i = 0; i < listBrick.size(); i++)
		{
				listBrick.at(i)->Render();
		}
		player->Render();
		HUD->Render();
		//CGame* game = CGame::GetInstance();
	
}
/*
	Unload current scene
*/
void CPlayScene::Unload()
{
	for (int i = 0; i < objects.size(); i++)
	{
		if (dynamic_cast<Simon*>(objects[i]) || dynamic_cast<Whip*>(objects[i]) || dynamic_cast<Dagger*>(objects[i]))
		{
			;
		}
		else
		{
			delete objects[i];
		}
	}
	objects.clear();
	listTorch.clear();
	listBrick.clear();
	listItem.clear();
	listStair.clear();
	//player = NULL;

	DebugOut(L"[INFO] Scene %s unloaded! \n", sceneFilePath);
	tileMap.clear();
}
void CPlayScene::CheckCollision_ItemAndSimon()
{
	for (UINT i = 0; i < listItem.size(); i++)
	{
		if (listItem.at(i)->visible == true)
		{
			if (player->CheckCollision(listItem.at(i)))
			{
				if (dynamic_cast<HeartItem*>(listItem.at(i)))
				{
					player->IncreaseNumHeart(5);
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<ChainItem*>(listItem.at(i)))
				{
					player->isEatingItem = true;
					listItem.at(i)->visible = false;
					player->whip->LevelUp();
				}
				if (dynamic_cast<DaggerItem*>(listItem.at(i)))
				{
					player->SetSubWeapon(ID_DAGGER);
					player->isCollectDagger = true;
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<BoomerangItem*>(listItem.at(i)))
				{
					player->isCollectHolyWater = false;
					player->isCollectDagger = false;
					player->SetSubWeapon(ID_BOOMERANG);
					player->isCollectDagger = false; //không thể dùng dagger sau khi nhặt boomerang
					player->isCollectBoomerang = true;
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<CrownItem*>(listItem.at(i)))
				{
					player->SetScore(2000);
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<SmallHeartItem*>(listItem.at(i)))
				{
					player->IncreaseNumHeart(1);
					player->SetScore(100);
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<MoneyBagItem*>(listItem.at(i)))
				{
					player->SetScore(1000);
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<HolyWaterItem*>(listItem.at(i)))
				{
					player->isCollectBoomerang = false;
					player->isCollectDagger = false;
					player->SetSubWeapon(ID_HOLYWATER);
					player->isCollectHolyWater = true;
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<AxeItem*>(listItem.at(i)))
				{
					player->SetSubWeapon(ID_AXE);
					player->isCollectAxe = true;
					listItem.at(i)->visible = false;
				}
				if (dynamic_cast<WatchItem*>(listItem.at(i)))
				{
					player->SetSubWeapon(ID_WATCH);
					//player->isCollectDagger = true;
					listItem.at(i)->visible = false;
				}
			}
		}
	}
}
void CPlayScene::CheckCollision_WhipAndEnemy()
{
	player->whip->SetDamage();
	if (player->state == SIMON_STATE_ATTACK || player->state == SIMON_STATE_SIT_AND_ATTACK || player->state == SIMON_STATE_THROW || player->state == SIMON_STATE_ATTACK_UPSTAIR || player->state == SIMON_STATE_ATTACK_DOWNSTAIR)
	{
		if (player->animation_set->at(SIMON_ANI_ATTACK)->GetCurrentFrame() == 2 || player->animation_set->at(SIMON_ANI_SIT_AND_ATTACK)->GetCurrentFrame() == 2)
		{
			for (UINT i = 0; i < listTorch.size(); i++)
			{
				if (player->whip->CheckCollision(listTorch.at(i)))
				{
					listTorch.at(i)->SetState(TORCH_DESTROYED);
					listTorch.at(i)->animation_set->at(TORCH_DESTROYED)->SetAniStartTime(GetTickCount());
				}
			}
			for (UINT i = 0; i < listEnemy.size(); i++)
			{
				LPGAMEOBJECT temp = listEnemy.at(i);
				if (dynamic_cast<BreakableBrick*>(temp))
				{
					//BreakableBrick* breakable_brick = dynamic_cast<BreakableBrick*> (temp);
					if (player->whip->AABB(player->whip->GetBound(), temp->GetBound()))
					{
						DebugOut(L"[INFO]Whip Collision with Breakable \n");
						if (temp->GetState() == BREAKABLE_BRICK_STATE_ORIGIN)
							temp->SetState(BREAKABLE_BRICK_STATE_HALF_PART);
						else if (temp->GetState() == BREAKABLE_BRICK_STATE_HALF_PART)
						{
							temp->Die();
							DebugOut(L"abcccccccccccccc\n");
						}
						Rocks::GetInstance()->DropRock(temp->x, temp->y);
					}
				}
				else if (dynamic_cast<Knight*>(temp))
				{
					if (player->whip->AABB(player->whip->GetBound(), temp->GetBound()))
					{
						DebugOut(L"[INFO] Whip Collision with Knight \n");
						temp->TakeDamage(player->whip->damage);
					}
				}
			}
		}
	}
}
void CPlayScene::CheckCollision_PortalAndSimon()
{
	for (UINT i = 0; i < listPortal.size(); i++)
	{
		if (player->CheckCollision(listPortal.at(i)))
		{
			CGame::GetInstance()->SwitchScene(this->scene_id);
		}
	}
}
void CPlayScene::CheckCollision_DaggerAndTorch()
{
	for (UINT i = 0; i < listTorch.size(); i++)
	{
		
		if (dagger->CheckCollision(listTorch.at(i)))
		{
			dagger->visible = false;
			listTorch.at(i)->SetState(TORCH_DESTROYED);
			listTorch.at(i)->animation_set->at(TORCH_DESTROYED)->SetAniStartTime(GetTickCount());
		}
	}
}
void CPlayScene::CheckCollision_SimonAndBoomerang()
{
	for (UINT i = 0; i < objects.size(); i++)
	{
		if (dynamic_cast<Boomerang*>(objects.at(i)))
		{
			
		}
	}
}
void CPlayScene::CheckCollision_BoomerangAndEnemy()
{
	for (UINT i = 0; i < objects.size(); i++)
	{
		if (dynamic_cast<Knight*>(objects.at(i)))
		{
			if (boomerang->CheckCollision(objects.at(i)))
			{
				objects.at(i)->visible = false;
			}
		}

	}
}
void CPlayScene::CheckCollision_SimonAndEnemy()
{
	if (player->GetState() == SIMON_STATE_DIE)
		return;
	for (UINT i = 0; i < listEnemy.size(); i++)
	{
		if (listEnemy.at(i)->visible == true)
		{
			if (player->CheckCollision(listEnemy.at(i)))
			{
				if (dynamic_cast<Bat*>(listEnemy.at(i)))
				{
					player->DecreaseHealth();
					listEnemy.at(i)->visible = false;
					player->SetScore(100);
					player->StartUntouchable();
					player->SetState(SIMON_STATE_DEFLECT);
				}
				if (dynamic_cast<Knight*>(listEnemy.at(i)))
				{
					player->DecreaseHealth();
					player->StartUntouchable();
					player->SetState(SIMON_STATE_DEFLECT);
				}
				if (dynamic_cast<BreakableBrick*>(listEnemy.at(i)))
				{
					DebugOut(L"Bug!\n");
				}	
			}
		}
	}
}
void CPlayScene::Handle_SimonAndMoving()
{
	for (UINT i = 0; i < listBrick.size(); i++)
	{
		if (player->CheckCollision(listBrick.at(i)))
		{
			if (dynamic_cast<MovingPlatform*>(listBrick.at(i)))
			{
				DebugOut(L"xxxxxxxxxxxxxxxxxxx\n");
			}
			else
			{
				DebugOut(L"yyyyyyyyyyyyyyyyyy\n");
			}
		}
	}
}
void CPlayScenceKeyHandler::OnKeyDown(int KeyCode)
{
	//DebugOut(L"[INFO] KeyDown: %d\n", KeyCode);

	Simon *simon = ((CPlayScene*)scence)->GetPlayer();
	Dagger* dagger = ((CPlayScene*)scence)->GetDagger();
	Boomerang* boomerang = ((CPlayScene*)scence)->GetBoomerang();
	HolyWater* holywater = ((CPlayScene*)scence)->GetHolyWater();
	Axe* axe = ((CPlayScene*)scence)->GetAxe();
	if (simon->GetState() == SIMON_STATE_DIE) return;
	if (simon->isEatingItem)
	{
		simon->SetState(SIMON_STATE_IDLE);
		return;
	}
	switch (KeyCode)
	{
	case DIK_S:
	{
		if ((simon->GetState() == SIMON_STATE_ATTACK) || (simon->GetState() == SIMON_STATE_SIT_AND_ATTACK))
			return;
		if ((simon->GetState() == SIMON_STATE_ATTACK_UPSTAIR) || (simon->GetState() == SIMON_STATE_ATTACK_DOWNSTAIR))
			return;
		if (simon->isOnStair)
		{
			if (simon->isUpstair)
			{
				simon->SetState(SIMON_STATE_ATTACK_UPSTAIR);
			}
			else
			{
				simon->SetState(SIMON_STATE_ATTACK_DOWNSTAIR);
			}
		}
		else
		{
			if (simon->GetState() == SIMON_STATE_IDLE || simon->GetState() == SIMON_STATE_JUMP)
			{
				simon->SetState(SIMON_STATE_ATTACK);
			}

			else if (simon->GetState() == SIMON_STATE_SIT) {
				simon->isAttack = true;
				simon->SetState(SIMON_STATE_SIT_AND_ATTACK);
			}
		}
		break;
	}
	case DIK_X:
	{
		if (simon->isCollectDagger)
		{
			if (simon->GetNumHeart() > 0)
			{
				if (simon->GetState() == SIMON_STATE_THROW) return;
				if (dagger->visible) return;
				DebugOut(L" Dagger Available\n");
				float x, y;
				simon->GetPosition(x, y);
				dagger->SetPosition(x, y + 5);
				dagger->SetOrientation(simon->nx);
				dagger->SetVisible(true);
				simon->SetState(SIMON_STATE_THROW);
				simon->SetNumHeart(simon->GetNumHeart() - 1);
			}
				
			
		}
		//use boomerang
		else if (simon->isCollectBoomerang)
		{
			if (simon->GetNumHeart() > 0)
			{
				if (simon->GetState() == SIMON_STATE_THROW) return;
				if (boomerang->visible) return;
				float x, y;
				simon->GetPosition(x, y);
				boomerang->SetPosition(x, y + 5);
				boomerang->SetOrientation(simon->nx);
				boomerang->SetVisible(true);
				simon->SetState(SIMON_STATE_THROW);
				simon->SetNumHeart(simon->GetNumHeart() - 1);
			}
		}
		//use holywater
		else if (simon->isCollectHolyWater)
		{
			if (simon->GetNumHeart() > 0)
			{
				if (simon->GetState() == SIMON_STATE_THROW) return;
				if (holywater->visible) return;
				float x, y;
				simon->GetPosition(x, y);
				holywater->firstx = x;
				holywater->SetPosition(x, y + 5);
				holywater->SetOrientation(simon->nx);
				holywater->SetVisible(true);
				simon->SetState(SIMON_STATE_THROW);
				simon->SetNumHeart(simon->GetNumHeart() - 1);
			}
		}
		//use axe
		else if (simon->isCollectAxe)
		{
			if (simon->GetNumHeart() > 0)
			{
				if (simon->GetState() == SIMON_STATE_THROW) return;
				if (holywater->visible) return;
				float x, y;
				simon->GetPosition(x, y);
				axe->firstx = x;
				axe->SetPosition(x, y + 5);
				axe->SetOrientation(simon->nx);
				axe->SetVisible(true);
				simon->SetState(SIMON_STATE_THROW);
				simon->SetNumHeart(simon->GetNumHeart() - 1);
			}
		}
		else
		{
			DebugOut(L"NOT OK \n");
		}
		break;
	}
	case DIK_SPACE:
	{
		if (simon->isAttack) return;
		if (simon->isOnStair) return;
		if (simon->isJumping == false)
		{
			simon->SetState(SIMON_STATE_JUMP);
			simon->isJumping = true;
		}
		break;
	}
	case DIK_A: 
		simon->Reset();
		break;
	}
}

void CPlayScenceKeyHandler::KeyState(BYTE *states)
{
	CGame *game = CGame::GetInstance();
	Simon* simon = ((CPlayScene*)scence)->GetPlayer();
	vector<LPGAMEOBJECT> listStair = ((CPlayScene*)scence)->GetListStair();
	if (simon->GetState() == SIMON_STATE_DIE) return;
	if (simon->isEatingItem)
	{
		simon->SetState(SIMON_STATE_IDLE);
		return;
	}
	if (simon->GetState() == SIMON_STATE_JUMP) return;
	if (simon->GetState() == SIMON_STATE_ATTACK && simon->animation_set->at(SIMON_ANI_ATTACK)->IsOver(SIMON_ATTACK_TIME) == false)
		return;
	if (simon->GetState() == SIMON_STATE_SIT_AND_ATTACK && simon->animation_set->at(SIMON_ANI_SIT_AND_ATTACK)->IsOver(SIMON_ATTACK_TIME) == false)
		return;
	if (simon->GetState() == SIMON_STATE_ATTACK_UPSTAIR && simon->animation_set->at(SIMON_ANI_ATTACK_UPSTAIR)->IsOver(SIMON_ATTACK_TIME) == false)
		return;
	if (simon->GetState() == SIMON_STATE_ATTACK_DOWNSTAIR && simon->animation_set->at(SIMON_ANI_ATTACK_DOWNSTAIR)->IsOver(SIMON_ATTACK_TIME) == false)
		return;
	if (simon->GetState() == SIMON_STATE_THROW &&
		simon->animation_set->at(SIMON_ANI_THROW)->IsOver(SIMON_ATTACK_TIME) == false)
		return;
	if (simon->GetState() == SIMON_STATE_DEFLECT &&
		simon->animation_set->at(SIMON_ANI_DEFLECT)->IsOver(SIMON_DEFLECT_TIME) == false)
		return;
	// disable control key when Mario die 
	if (simon->GetState() == SIMON_STATE_DIE) return;
	if (game->IsKeyDown(DIK_RIGHT))		//simon->SetState(SIMON_STATE_WALKING_RIGHT);
	{						
		if (simon->GetState() == SIMON_STATE_ATTACK || simon->GetState() == SIMON_STATE_SIT_AND_ATTACK) return;
		if (game->IsKeyDown(DIK_DOWN))
		{
			if (simon->isOnStair == false)
			{
				simon->nx = 1;
				simon->SetState(SIMON_STATE_SIT);
				return;
			}
		}
		for (int i = 0; i < listStair.size(); i++)
		{
			LPGAMEOBJECT temp = listStair.at(i);
			if (dynamic_cast<BotStair*>(temp))
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					simon->isHitBottomStair = true;
					break;
				}
			}
			else
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					simon->isHitTopStair = true;
					break;
				}
			}
		}
		if (simon->isOnStair)
		{
			DebugOut(L"OK------------\n");
			if (simon->isUpstair)
				simon->isHitBottomStair = false;
			if (simon->isUpstair == false)
				simon->isHitTopStair = false;

			if (simon->StairDirection == 1)
			{
				simon->isUpstair = true;
			}
			else if (simon->StairDirection == -1)
			{
				if (simon->isUpstair)
				{
					simon->isUpstair = false;
				}
				else
				{
					simon->isUpstair = false;
				}
			}
			simon->isStopOnStair = false;
			simon->SetState(SIMON_STATE_ONSTAIR);
			return;
		}
		else
		{
			simon->SetOrientation(1);
			simon->SetState(SIMON_STATE_WALKING);
		}
	}
	else if (game->IsKeyDown(DIK_LEFT))
	{
		if (simon->isAttack || simon->isSitAttack) return;
		if (game->IsKeyDown(DIK_DOWN))
		{
			if (simon->isOnStair == false)
			{
				simon->nx = -1;
				simon->SetState(SIMON_STATE_SIT);
				return;
			}
		}
		for (int i = 0; i < listStair.size(); i++)
		{
			LPGAMEOBJECT temp = listStair.at(i);
			if (dynamic_cast<BotStair*>(temp))
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					simon->isHitBottomStair = true;
					break;
				}
			}
			else
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					simon->isHitTopStair = true;
					break;
				}
			}
		}
		if (simon->isOnStair)
		{
			// dòng này để tránh Simon đang lên cầu thang nhưng lại checkColWithStair do biến isHitBottom vẫn true
				if (simon->isUpstair)
					simon->isHitBottomStair = false;
			if (simon->isUpstair == false)
				simon->isHitTopStair = false;

			if (simon->StairDirection == 1)  //stair từ trái qua phải
			{
				simon->isUpstair = true;
			}
			else if (simon->StairDirection == -1)  //stair từ phải qua trái
			{
				if (simon->isUpstair)
				{
					simon->isUpstair = false;
				}
				else
				{
					simon->isUpstair = false;
				}
			}
			simon->isStopOnStair = false;
			simon->SetState(SIMON_STATE_ONSTAIR);
			return;
		}
		else
		{
			simon->SetOrientation(-1);
			simon->SetState(SIMON_STATE_WALKING);
		}
	}
	else if (game->IsKeyDown(DIK_DOWN))
	{
		if (simon->isAttack) return;
		for (int i = 0; i < listStair.size(); i++)
		{
			LPGAMEOBJECT temp = listStair.at(i);
			if (dynamic_cast<BotStair*>(temp))
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					simon->isHitBottomStair = true;
				}
			}
			else if (dynamic_cast<TopStair*>(temp))
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					if (simon->nx > 0)
					{
						simon->StairDirection = -1;
						if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToLeft)
						{
							simon->StairDirection = 1;
						}
					}
					else if (simon->nx < 0)
					{
						simon->StairDirection = 1;
						if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToRight)
						{
							simon->StairDirection = -1;
						}
					}
					if (abs(simon->x - listStair.at(i)->x) <= 10)
					{
						DebugOut(L"Distance is: %f", (abs(simon->x - listStair.at(i)->x)));
						if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToRight)
						{
							DebugOut(L"To Right \n");
							simon->isHitTop = true;
							simon->isWalkingToStair = true;
							simon->nx = 1;
							simon->SetState(SIMON_STATE_WALKING);
						}
						else if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToLeft)
						{
							DebugOut(L"To Left \n");
							simon->isHitTop = true;
							simon->isWalkingToStair = true;
							simon->nx = -1;
							simon->SetState(SIMON_STATE_WALKING);
						}
					}
					else
					{
						simon->isOnStair = true;
					}
					if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToRight)
					{
						simon->isHitTop = true;
						simon->SetPosition(listStair.at(i)->x-10, simon->GetPosition().y);
						simon->isOnStair = true;
					}
				}
			}
		}
		if (simon->isOnStair)
		{
			simon->isHitTop = false;
			simon->isStopOnStair = false;
			if (simon->isUpstair)
				simon->isHitBottomStair = false;
			simon->isUpstair = false;
			simon->SetState(SIMON_STATE_ONSTAIR);
		}
		else
		{
			if (simon->isHitTop == false)
			{
				//DebugOut(L"DIK_RIGHT OK \n");
				if (simon->isAttack == false && simon->isJumping == false)
						simon->SetState(SIMON_STATE_SIT);
			}
		}
	}
	else if (game->IsKeyDown(DIK_UP))
	{
		if (simon->state == SIMON_STATE_JUMP) return;
		if (simon->isAttack) return; //khi đánh trên cầu thang thì k lên		
		for (int i = 0; i < listStair.size(); i++)
		{
			LPGAMEOBJECT temp = listStair.at(i);
			if (dynamic_cast<BotStair*>(temp))
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToRight)
					{
						simon->StairDirection = 1;
					}
					else if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToLeft)
					{
						simon->StairDirection = -1;
					}
					if (abs(simon->x - listStair.at(i)->x) < 14)
					{
						//DebugOut(L"Distance is: %f \n", abs(simon->x - list.at(i)->x));
						if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToRight)
						{
							simon->isWalkingToStair = true;
							simon->nx = 1;
							simon->SetState(SIMON_STATE_WALKING);
						}
						else if (listStair.at(i)->StairTag == CGameObject::StairTypes::ToLeft)
						{
							simon->isWalkingToStair = true;
							simon->nx = -1;
							simon->SetState(SIMON_STATE_WALKING);
						}
					}
					else
					{
						simon->isWalkingToStair = false;
						simon->isOnStair = true;
						break;
					}
				}
			}
			else if (dynamic_cast<TopStair*>(temp))
			{
				if (CGame::GetInstance()->AABB(listStair.at(i)->GetBound(), simon->GetBound()))
				{
					if(simon->nx > 0 && listStair.at(i)->StairTag == CGameObject::StairTypes::ToLeft)
					simon->isHitTopStair = true;
					if (simon->nx < 0 && listStair.at(i)->StairTag == CGameObject::StairTypes::ToRight)
						simon->isHitTopStair = true;
				}
			}
		}
		if (simon->isOnStair)
		{
			simon->isStopOnStair = false;
			if (simon->isUpstair == false) //dòng này để tránh Simon đang xuống cầu thang nhưng lại checkColWithStair do biến isHitTopStair vẫn true
				simon->isHitTopStair = false;
			simon->isUpstair = true;
			simon->SetState(SIMON_STATE_ONSTAIR);
		}
	}
	else
	{
		if (simon->isOnStair)
		{
			simon->SetState(SIMON_STATE_ONSTAIR);
		}
		else
			simon->SetState(SIMON_STATE_IDLE);
	}
}

void  CPlayScenceKeyHandler::OnKeyUp(int KeyCode)
{
	Simon* simon = ((CPlayScene*)scence)->GetPlayer();
	if (simon->GetState() == SIMON_STATE_DIE) return;
	switch (KeyCode)
	{
	case DIK_LEFT:
		if (simon->isOnStair)
		{
			simon->isStopOnStair = true;
			simon->vy = 0;
			simon->vx = 0;
		}
		else
		{
			if ((simon->state == SIMON_STATE_ATTACK) || (simon->state == SIMON_STATE_SIT) || simon->state == SIMON_STATE_ATTACK || simon->isJumping == true)
				return;
			simon->SetState(SIMON_STATE_IDLE);
		}
		break;
	case DIK_RIGHT:
		if (simon->isOnStair)
		{
			simon->isStopOnStair = true;
			simon->vx = 0;
			simon->vy = 0;
		}
		else
		{
			if ((simon->isAttack) || (simon->state == SIMON_STATE_SIT) || simon->isJumping == true)
				return;
			simon->SetState(SIMON_STATE_IDLE);
		}
		break;
	case DIK_DOWN:
		if (simon->isOnStair)
		{
			simon->isStopOnStair = true;
			simon->vx = 0;
			simon->vy = 0;
		}
		else
		{
			if ((simon->isAttack) || (simon->isSitAttack))
				return;
			simon->SetState(SIMON_STATE_IDLE);
		}
		break;
	case DIK_UP:
	{
		simon->isStopOnStair = true;
		simon->vx = 0;
		simon->vy = 0;
		break;
	}

	}
}
