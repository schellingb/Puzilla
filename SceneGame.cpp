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

static ZL_TextBuffer txtScoreLabel, txtScore, txtCombo, txtGameOver;
static float scoredisplay = 0;
static bool paused = true;
static ZL_Rectf recPause, recPauseContinue, recPauseQuit, recPauseRestart;

static ZL_Surface srfUIButtons, srfUIPause;
static struct sGameButton
{
	bool is_down;
	ZL_Rectf rec;
	unsigned char down_touch_id, surface_tile_index;
	sGameButton() : is_down(false) {}
	sGameButton(scalar x, scalar y, unsigned char surface_tile_index) : is_down(false), rec(x, y, x+s(140), y+s(140)), surface_tile_index(surface_tile_index) { }
	bool OnPointerDown(ZL_PointerPressEvent& e)
	{
		if (is_down || !rec.Contains(e)) return false;
		is_down = true;
		down_touch_id = e.which;
		return true;
	}
	bool OnPointerUp(ZL_PointerPressEvent& e)
	{
		if (!is_down || down_touch_id != e.which) return false;
		is_down = false;
		return true;
	}
	void Draw()
	{
		scalar off = s(is_down ? 3 : 0);
		srfUIButtons.SetTilesetIndex(surface_tile_index);
		srfUIButtons.DrawTo(rec.left+5, rec.low-5, rec.right+5, rec.high-5, ZLBLACK);
		srfUIButtons.DrawTo(rec.left+off, rec.low-off, rec.right+off, rec.high-off, (is_down ? ZLLUM(0.8) : ZLOPAQUE));
	}
} btnJump1, btnJump2, btnFire, btnLeft, btnRight;
static ZL_Rectf recBtnPause;

void StartGame()
{
	World.Init();
	scoredisplay = World.score;
	txtScore = ZL_TextBuffer(fntBig, "0");
	paused = false;
}

void UpdateUI()
{
	recPause         = ZL_Rectf(ZLHALFW - 150, ZLHALFH - 200, ZLHALFW + 150, ZLHALFH + 200);
	recPauseContinue = ZL_Rectf(ZLHALFW - 120, ZLHALFH - 170, ZLHALFW + 120, ZLHALFH - 100);
	recPauseQuit     = ZL_Rectf(ZLHALFW - 120, ZLHALFH -  70, ZLHALFW + 120, ZLHALFH -   0);
	recPauseRestart  = ZL_Rectf(ZLHALFW - 120, ZLHALFH +  30, ZLHALFW + 120, ZLHALFH + 100);

	btnJump1 = sGameButton(         10 , 160, 0);
	btnJump2 = sGameButton(ZLFROMW(150), 160, 0);
	btnFire  = sGameButton(         10 ,  10, 1);
	btnLeft  = sGameButton(ZLFROMW(300),  10, 2);
	btnRight = sGameButton(ZLFROMW(150),  10, 3);
	recBtnPause = ZL_Rectf(ZLFROMW(167+10), ZLFROMH(64+10), ZLFROMW(10), ZLFROMH(10));
}

void SetPause(bool setpause, bool playsound = true)
{
	if (paused == setpause) return;
	if (playsound) imcFull.Play(true);
	paused = setpause;
}

struct sSceneGame : public ZL_Scene
{
	sSceneGame() : ZL_Scene(SCENE_GAME) { }

	void InitGlobal()
	{
		txtScoreLabel = ZL_TextBuffer(fntBig, "SCORE:");
		txtGameOver = ZL_TextBuffer(fntBig, "GAME OVER");
		srfUIButtons = ZL_Surface("Data/uibuttons.png").SetTilesetClipping(2, 2);
		srfUIPause = ZL_Surface("Data/uipause.png");
	}

	int InitTransitionEnter(ZL_SceneType, void*)
	{
		ZL_Display::sigPointerDown.connect(this, &sSceneGame::OnPointerDown);
		ZL_Display::sigPointerUp.connect(this, &sSceneGame::OnPointerUp);
		ZL_Display::sigKeyDown.connect(this, &sSceneGame::OnKeyDown);
		ZL_Display::sigKeyUp.connect(this, &sSceneGame::OnKeyUp);
		ZL_Display::sigActivated.connect(this, &sSceneGame::OnActivated);
		ZL_Display::sigResized.connect(this, &sSceneGame::OnResize);
		StartGame();
		UpdateUI();
		return 200;
	}

	int DeInitTransitionLeave(ZL_SceneType SceneTypeTo)
	{
		ZL_Display::AllSigDisconnect(this);
		return 400;
	}

	void OnResize(ZL_WindowResizeEvent& e)
	{
		UpdateUI();
	}

	void OnActivated(ZL_WindowActivateEvent& e)
	{
		if (!e.key_focus || e.minimized) SetPause(true, false);
	}

	void OnPointerDown(ZL_PointerPressEvent& e)
	{
		showTouchUI = true;
		if (World.gameovertime) { StartGame(); return; }
		if (!paused)
		{
			if      (btnFire.OnPointerDown(e))  World.PlayerAttackInput = true;
			else if (btnJump1.OnPointerDown(e)) UpdatePlayerInput();
			else if (btnJump2.OnPointerDown(e)) UpdatePlayerInput();
			else if (btnLeft.OnPointerDown(e))  UpdatePlayerInput();
			else if (btnRight.OnPointerDown(e)) UpdatePlayerInput();
			else if (recBtnPause.Contains(e)) SetPause(true);
			return;
		}
		if (!recPause.Contains(e)) SetPause(false);
		else if (recPauseContinue.Contains(e)) SetPause(false);
		else if (recPauseQuit.Contains(e)) { imcFull.Play(true); ZL_SceneManager::GoToScene(SCENE_TITLE); }
		else if (recPauseRestart.Contains(e)) { StartGame(); SetPause(false); }
	}

	void OnPointerUp(ZL_PointerPressEvent& e)
	{
		if      (btnFire.OnPointerUp(e))  World.PlayerAttackInput = false;
		else if (btnJump1.OnPointerUp(e)) UpdatePlayerInput();
		else if (btnJump2.OnPointerUp(e)) UpdatePlayerInput();
		else if (btnLeft.OnPointerUp(e))  UpdatePlayerInput();
		else if (btnRight.OnPointerUp(e)) UpdatePlayerInput();
	}

	void OnKeyDown(ZL_KeyboardEvent& e)
	{
		showTouchUI = false;
		if (e.is_repeat) return;
		if (paused)
		{
			SetPause(false);
			return;
		}
		if (e.key == ZLK_ESCAPE) { SetPause(true); return; }
		if (World.gameovertime) { StartGame(); return; }
		if (e.key == ZLK_SPACE)
		{
			World.PlayerAttackInput = true;
			ZL_LOG1("INPUT", "Attack Input: %d", World.PlayerAttackInput);
		}
		UpdatePlayerInput();
	}

	void OnKeyUp(ZL_KeyboardEvent& e)
	{
		if (e.key == ZLK_SPACE)
		{
			World.PlayerAttackInput = false;
			ZL_LOG1("INPUT", "Attack Input: %d", World.PlayerAttackInput);
		}
		UpdatePlayerInput();
	}

	void UpdatePlayerInput()
	{
		World.PlayerMoveInput.x = 0;
		World.PlayerMoveInput.y = 0;
		if (btnLeft.is_down || ZL_Display::KeyDown[ZLK_LEFT] || ZL_Display::KeyDown[ZLK_A]) World.PlayerMoveInput.x -= 1;
		if (btnRight.is_down || ZL_Display::KeyDown[ZLK_RIGHT] || ZL_Display::KeyDown[ZLK_D]) World.PlayerMoveInput.x += 1;
		if (btnJump1.is_down || btnJump2.is_down || ZL_Display::KeyDown[ZLK_UP] || ZL_Display::KeyDown[ZLK_W]) World.PlayerMoveInput.y += 1;
	}

	void Calculate()
	{
		if (paused) return;
		if (World.gameovertime) return;
		World.Calculate();
		if (World.score != scoredisplay)
		{
			scoredisplay += (World.score - scoredisplay)*ZLELAPSED*2;
			if (scoredisplay > World.score-1) scoredisplay = World.score;
			ZL_String strScore;
			int n = (int)scoredisplay;
			if (scoredisplay > 1000000) strScore = ZL_String::format("%d.%03d.%03d", (n/1000000), ((n/1000)%1000), (n%1000));
			else if (scoredisplay > 1000) strScore = ZL_String::format("%d.%03d", (n/1000), (n%1000));
			else strScore = n;
			txtScore = ZL_TextBuffer(fntBig, strScore);
			if (World.combocount > 1)
			{
				txtCombo = ZL_TextBuffer(fntBig, ZL_String(World.combocount)<<" x COMBO!!");
			}
		}
	}

	void Draw()
	{
		ZL_Display::FillGradient(0, 0, ZLWIDTH, ZLHEIGHT, ZLRGB(0,0,.3), ZLRGB(0,0,.3), ZLRGB(.4,.4,.4), ZLRGB(.4,.4,.4));
		World.Draw();
		//fntMain.Draw(ZLFROMW(170), ZLFROMH(45), "Game Scene");
		txtScoreLabel.Draw(150-2, ZLFROMH(45)-2, 0.8f, 0.8f, ZLBLACK, ZL_Origin::TopRight);
		txtScoreLabel.Draw(150-2, ZLFROMH(45)+2, 0.8f, 0.8f, ZLBLACK, ZL_Origin::TopRight);
		txtScoreLabel.Draw(150+2, ZLFROMH(45)+2, 0.8f, 0.8f, ZLBLACK, ZL_Origin::TopRight);
		txtScoreLabel.Draw(150+2, ZLFROMH(45)-2, 0.8f, 0.8f, ZLBLACK, ZL_Origin::TopRight);
		txtScoreLabel.Draw(150, ZLFROMH(45), 0.8f, 0.8f, ZL_Origin::TopRight);
		scalar scorescale = 1 + ZL_Math::Clamp01((World.score - scoredisplay) / 500);
		txtScore.Draw(150-2, ZLFROMH(75)-2, scorescale, scorescale, ZLBLACK, ZL_Origin::TopRight);
		txtScore.Draw(150-2, ZLFROMH(75)+2, scorescale, scorescale, ZLBLACK, ZL_Origin::TopRight);
		txtScore.Draw(150+2, ZLFROMH(75)+2, scorescale, scorescale, ZLBLACK, ZL_Origin::TopRight);
		txtScore.Draw(150+2, ZLFROMH(75)-2, scorescale, scorescale, ZLBLACK, ZL_Origin::TopRight);
		txtScore.Draw(150, ZLFROMH(75), scorescale, scorescale, ZL_Origin::TopRight);
		if (showTouchUI)
		{
			btnJump1.Draw();
			btnJump2.Draw();
			btnFire.Draw();
			btnLeft.Draw();
			btnRight.Draw();
			srfUIPause.DrawTo(recBtnPause.left+5, recBtnPause.low-5, recBtnPause.right+5, recBtnPause.high-5, ZLBLACK);
			srfUIPause.DrawTo(recBtnPause);
		}
		if (World.combotime && World.combocount > 1 && (ZLTICKS - World.combotime) < 1000)
		{
			scalar sc = 5 - (s(ZLTICKS - World.combotime)/200.0f);
			txtCombo.Draw(ZLHALFW-4, ZLHALFH-4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtCombo.Draw(ZLHALFW-4, ZLHALFH+4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtCombo.Draw(ZLHALFW+4, ZLHALFH+4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtCombo.Draw(ZLHALFW+4, ZLHALFH-4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtCombo.Draw(ZLCENTER, sc, sc, ZLWHITE, ZL_Origin::Center);
		}
		if (World.gameovertime)
		{
			scalar sc = MIN(4, (s(ZLTICKS - World.combotime)/300.0f));
			txtGameOver.Draw(ZLHALFW-4, ZLHALFH-4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtGameOver.Draw(ZLHALFW-4, ZLHALFH+4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtGameOver.Draw(ZLHALFW+4, ZLHALFH+4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtGameOver.Draw(ZLHALFW+4, ZLHALFH-4, sc, sc, ZLBLACK, ZL_Origin::Center);
			txtGameOver.Draw(ZLCENTER, sc, sc, ZL_Color::Red, ZL_Origin::Center);
		}
		else if (paused)
		{
			ZL_Display::PushMatrix();
			ZL_Display::Translate(ZLHALFW, ZLHALFH);
			ZL_Display::Rotate(scos(ZLTICKS*s(.001))*s(.02));
			ZL_Display::Scale(1+ssin(ZLTICKS*s(.001))*s(.02));
			ZL_Display::Translate(-ZLHALFW, -ZLHALFH);

			ZL_Display::DrawRect(recPause, ZL_Color::Black, ZLLUMA(1,.5));
			DrawTextBordered(ZL_Vector(recPause.MidX(), recPause.high - 50), "PAUSE");
			ZL_Display::DrawRect(recPauseContinue, ZL_Color::Black, ZLLUMA(0,.5));
			ZL_Display::DrawRect(recPauseQuit    , ZL_Color::Black, ZLLUMA(0,.5));
			ZL_Display::DrawRect(recPauseRestart , ZL_Color::Black, ZLLUMA(0,.5));
			DrawTextBordered(recPauseContinue.Center(), "CONTINUE");
			DrawTextBordered(recPauseQuit.Center(), "QUIT");
			DrawTextBordered(recPauseRestart.Center(), "RESTART");

			ZL_Display::PopMatrix();
		}
	}
} SceneGame;
