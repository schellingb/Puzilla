/*
  Puzilla
  Copyright (C) 2013-2019 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "include.h"
#include "world.h"

static ZL_Surface srf1GAMLogo, srfBG, srfBox;
static scalar intro_logo, intro_1gam;
static ZL_Rectf recStart;

static void OnStartPress()
{
	imcFull.Play(true);
	ZL_SceneManager::GoToScene(SCENE_GAME);
}

struct sSceneTitle : public ZL_Scene
{
	sSceneTitle() : ZL_Scene(SCENE_TITLE) { }

	int InitTransitionEnter(ZL_SceneType, void*)
	{
		srf1GAMLogo = ZL_Surface("Data/1gamlogo.png").SetDrawOrigin(ZL_Origin::Center);
		srfBG = ZL_Surface("Data/box_inner.png").SetTextureRepeatMode().SetScale(0.6f).SetColor(ZLRGB(.3,.3,.3));
		srfBox = ZL_Surface("Data/box_inner.png").SetScale(0.6f);

		ZL_Timer::AddTransitionFloat(&(intro_logo = 0), 1, 700,   0);
		ZL_Timer::AddTransitionFloat(&(intro_1gam = 0), 1, 700, 600);

		return 400;
	}

	void InitAfterTransition()
	{
		ZL_Display::sigPointerUp.connect(this, &sSceneTitle::OnPointerUp);
		ZL_Display::sigKeyDown.connect(this, &sSceneTitle::OnKeyDown);
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		return 400;
	}

	void DeInitAfterTransition()
	{
		srf1GAMLogo = ZL_Surface();
		srfBG = ZL_Surface();
	}

	void OnPointerUp(ZL_PointerPressEvent& e)
	{
		if      (recStart.Contains(e)) OnStartPress();
		else if (e.x >= ZLFROMW(220) && e.y >= ZLFROMH(150)) ZL_Application::OpenExternalUrl("http://www.onegameamonth.com/");
		else if (e.y <= 40) ZL_Application::OpenExternalUrl("https://zillalib.github.io/");
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		if (e.key == ZLK_ESCAPE) ZL_Application::Quit();
	}

	void Calculate()
	{
		recStart = ZL_Rectf(ZLHALFW - 250, 180, ZLHALFW + 250, 280);
	}

	void Draw()
	{
		ZL_Display::ClearFill(ZLRGB(.3,0,0));
		scalar sz = srfBox.GetWidth()*srfBox.GetScaleW();
		srfBG.DrawTo(ZLHALFW-sz*100-(sz/s(2)), ZLFROMH(sz*100), ZLWIDTH, ZLHEIGHT);

		scalar logoy = 300 * (1 - intro_logo);
		scalar gamy = 300 * (1 - intro_1gam);

		const unsigned char titlemap[5][27] = {
			{ 1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,0,0,1,0,0,0,1,1,1 },
			{ 1,0,1,0,1,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,1 },
			{ 1,1,1,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,1 },
			{ 1,0,0,0,1,0,1,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,1 },
			{ 1,0,0,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1 },
		};
		const ZL_Color titlecol[] = { ZL_Color::Red, ZL_Color::Green, ZL_Color::Yellow, ZL_Color::Cyan };
		for (int y = 0; y < 5; y++)
			for (int x = 0; x < 27; x++)
				if (titlemap[y][x])
					srfBox.Draw(ZLHALFW + (s(x)-(s(27)/s(2)))*sz, logoy + ZLFROMH((y+4)*sz), titlecol[((y+x)+(ZLTICKS/200))%4]);


		ZL_Display::PushMatrix();
		ZL_Display::Translate(ZLHALFW, ZLHALFH);
		ZL_Display::Rotate(scos(ZLTICKS*s(.001))*s(.02));
		ZL_Display::Scale(1+ssin(ZLTICKS*s(.001))*s(.02));
		ZL_Display::Translate(-ZLHALFW, -ZLHALFH);

		srf1GAMLogo.Draw(gamy+ZLFROMW(120)+6/2, ZLFROMH(70)+6/2, s(-.3), ZL_Color::Black);
		srf1GAMLogo.Draw(gamy+ZLFROMW(120), ZLFROMH(70), s(-.3));

		ZL_Display::DrawRect(recStart, ZL_Color::Black, ZLLUMA(0,.5));
		DrawTextBordered(recStart.Center(), "Start Game");

		ZL_Display::PopMatrix();

		DrawTextBordered(ZL_Vector(ZLHALFW, 20), "(c) 2013-2019 Bernhard Schelling", fntMain);
	}
} SceneMenu;
