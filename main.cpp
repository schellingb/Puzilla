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

ZL_Font fntMain, fntBig;

#ifdef __SMARTPHONE__
bool showTouchUI = true;
#else
bool showTouchUI = false;
#endif

struct sPuzilla : public ZL_Application
{
	sPuzilla() : ZL_Application(60) { }

	virtual void Load(int argc, char *argv[])
	{
		if (!LoadReleaseDesktopDataBundle()) return;
		ZL_Display::Init("Puzilla", 1280, 720, ZL_DISPLAY_ALLOWRESIZEHORIZONTAL);
		ZL_Display::ClearFill(ZL_Color::White);
		ZL_Display::SetAA(true);
		ZL_Audio::Init();
		ZL_Timer::Init();

		fntMain = ZL_Font("Data/fntMain.png");
		fntBig = ZL_Font("Data/typomoderno_ttf.zip", 40).SetDrawAtBaseline(false);

		World.InitGlobal();

		ZL_SceneManager::Init(SCENE_TITLE);
	}

	//enable for slow mo debug of animations
	//virtual void BeforeFrame() { ZL_Application::Elapsed /= s(5*5*5*5); ZL_Application::ElapsedTicks/=s(5); ZL_Application::Ticks /= s(5); }

	//display fps
	//virtual void AfterFrame() { fntMain.Draw(ZLFROMW(50), ZLFROMH(30), ZL_String::format("%d", FPS)); }
} Puzilla;

void DrawTextBordered(const ZL_Vector& p, const char* txt, ZL_Font& fnt, ZL_Origin::Type origin, scalar scale, const ZL_Color& colborder, const ZL_Color& colfill)
{
	fnt.Draw(p.x+1, p.y-1, txt, scale, scale, colborder, origin);
	fnt.Draw(p.x+1, p.y+1, txt, scale, scale, colborder, origin);
	fnt.Draw(p.x-1, p.y-1, txt, scale, scale, colborder, origin);
	fnt.Draw(p.x-1, p.y+1, txt, scale, scale, colborder, origin);
	fnt.Draw(p.x  , p.y  , txt, scale, scale, colfill, origin);
}

//-------------------------------------------------------------------------------

#define CHARGE_IMCSONG_LEN 0x1
#define CHARGE_IMCSONG_ROWLENSAMPLES 500000
#define CHARGE_IMCSONG_ENVLISTSIZE 4
#define CHARGE_IMCSONG_ENVCOUNTERLISTSIZE 5
#define CHARGE_IMCSONG_OSCLISTSIZE 10
#define CHARGE_IMCSONG_EFFECTLISTSIZE 0
#define CHARGE_IMCSONG_VOL 97
unsigned int CHARGE_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char CHARGE_ImcSongPatternData[] = {
  0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
unsigned char CHARGE_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope CHARGE_ImcSongEnvList[] = {
  { 0, 256, 27, 25, 16, 255, true, 255, },
  { 0, 256, 29, 24, 16, 255, true, 255, },
  { 0, 256, 518, 14, 255, 255, true, 255, },
  { 0, 256, 31, 24, 16, 255, true, 255, },
};
TImcSongEnvelopeCounter CHARGE_ImcSongEnvCounterList[] = {
 { 0, 0, 2 }, { -1, -1, 256 }, { 1, 0, 0 }, { 2, 0, 184 },
  { 3, 0, 0 },
};
TImcSongOscillator CHARGE_ImcSongOscillatorList[] = {
  { 8, 0, IMCSONGOSCTYPE_SINE, 0, -1, 100, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_SAW, 0, -1, 38, 1, 4 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 0, 0, 68, 3, 1 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
unsigned char CHARGE_ImcSongChannelVol[8] = {81, 100, 100, 100, 100, 100, 100, 100 };
unsigned char CHARGE_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool CHARGE_ImcSongChannelStopNote[8] = {true, false, false, false, false, false, false, false };
TImcSongData imcDataCHARGE = {
  CHARGE_IMCSONG_LEN, CHARGE_IMCSONG_ROWLENSAMPLES, CHARGE_IMCSONG_ENVLISTSIZE, CHARGE_IMCSONG_ENVCOUNTERLISTSIZE, CHARGE_IMCSONG_OSCLISTSIZE, CHARGE_IMCSONG_EFFECTLISTSIZE, CHARGE_IMCSONG_VOL,
  CHARGE_ImcSongOrderTable, CHARGE_ImcSongPatternData, CHARGE_ImcSongPatternLookupTable, CHARGE_ImcSongEnvList, CHARGE_ImcSongEnvCounterList, CHARGE_ImcSongOscillatorList, NULL,
  CHARGE_ImcSongChannelVol, CHARGE_ImcSongChannelEnvCounter, CHARGE_ImcSongChannelStopNote };
ZL_SynthImcTrack imcCharge(&imcDataCHARGE, false);

#define PUSH_IMCSONG_LEN 0x1
#define PUSH_IMCSONG_ROWLENSAMPLES 16537
#define PUSH_IMCSONG_ENVLISTSIZE 3
#define PUSH_IMCSONG_ENVCOUNTERLISTSIZE 4
#define PUSH_IMCSONG_OSCLISTSIZE 9
#define PUSH_IMCSONG_EFFECTLISTSIZE 2
#define PUSH_IMCSONG_VOL 179
unsigned int PUSH_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char PUSH_ImcSongPatternData[] = {
  0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
unsigned char PUSH_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope PUSH_ImcSongEnvList[] = {
  { 0, 256, 5, 0, 24, 255, true, 255, },
  { 0, 256, 43, 3, 21, 255, true, 255, },
  { 0, 256, 87, 5, 19, 255, true, 255, },
};
TImcSongEnvelopeCounter PUSH_ImcSongEnvCounterList[] = {
 { -1, -1, 256 }, { 0, 0, 128 }, { 1, 0, 206 }, { 2, 0, 238 },
};
TImcSongOscillator PUSH_ImcSongOscillatorList[] = {
  { 6, 169, IMCSONGOSCTYPE_SINE, 0, -1, 152, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 0, 0, 60, 3, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
TImcSongEffect PUSH_ImcSongEffectList[] = {
  { 7874, 416, 1, 0, IMCSONGEFFECTTYPE_OVERDRIVE, 0, 0 },
  { 67, 0, 1, 0, IMCSONGEFFECTTYPE_LOWPASS, 0, 0 },
};
unsigned char PUSH_ImcSongChannelVol[8] = {171, 100, 100, 100, 100, 100, 100, 100 };
unsigned char PUSH_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool PUSH_ImcSongChannelStopNote[8] = {true, false, false, false, false, false, false, false };
TImcSongData imcDataPUSH = {
  PUSH_IMCSONG_LEN, PUSH_IMCSONG_ROWLENSAMPLES, PUSH_IMCSONG_ENVLISTSIZE, PUSH_IMCSONG_ENVCOUNTERLISTSIZE, PUSH_IMCSONG_OSCLISTSIZE, PUSH_IMCSONG_EFFECTLISTSIZE, PUSH_IMCSONG_VOL,
  PUSH_ImcSongOrderTable, PUSH_ImcSongPatternData, PUSH_ImcSongPatternLookupTable, PUSH_ImcSongEnvList, PUSH_ImcSongEnvCounterList, PUSH_ImcSongOscillatorList, PUSH_ImcSongEffectList,
  PUSH_ImcSongChannelVol, PUSH_ImcSongChannelEnvCounter, PUSH_ImcSongChannelStopNote };
ZL_SynthImcTrack imcPush(&imcDataPUSH, false);

//-------------------------------------------------------------------------------

#define BLOCK_IMCSONG_ENVLISTSIZE 4
#define BLOCK_IMCSONG_ENVCOUNTERLISTSIZE 33
#define BLOCK_IMCSONG_OSCLISTSIZE 24
#define BLOCK_IMCSONG_EFFECTLISTSIZE 0
#define BLOCK_IMCSONG_VOL 100
TImcSongEnvelope BLOCK_ImcSongEnvList[] = {
  { 0, 256, 204, 8, 16, 4, true, 255, },
  { 0, 256, 209, 8, 16, 255, true, 255, },
  { 64, 256, 261, 8, 15, 255, true, 255, },
  { 0, 256, 3226, 8, 16, 255, true, 255, },
};
TImcSongEnvelopeCounter BLOCK_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 256 }, { 2, 0, 256 }, { -1, -1, 256 },
  { 3, 0, 256 }, { 0, 1, 256 }, { 1, 1, 256 }, { 2, 1, 256 },
  { 3, 1, 256 }, { 0, 2, 256 }, { 1, 2, 256 }, { 2, 2, 256 },
  { 3, 2, 256 }, { 0, 3, 256 }, { 1, 3, 256 }, { 2, 3, 256 },
  { 3, 3, 256 }, { 0, 4, 256 }, { 1, 4, 256 }, { 2, 4, 256 },
  { 3, 4, 256 }, { 0, 5, 256 }, { 1, 5, 256 }, { 2, 5, 256 },
  { 3, 5, 256 }, { 0, 6, 256 }, { 1, 6, 256 }, { 2, 6, 256 },
  { 3, 6, 256 }, { 0, 7, 256 }, { 1, 7, 256 }, { 2, 7, 256 },
  { 3, 7, 256 },
};
TImcSongOscillator BLOCK_ImcSongOscillatorList[] = {
  { 6, 127, IMCSONGOSCTYPE_SINE, 0, -1, 206, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 0, -1, 186, 4, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 0, 0, 100, 3, 3 },
  { 6, 127, IMCSONGOSCTYPE_SINE, 1, -1, 206, 6, 7 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 1, -1, 186, 8, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 1, 3, 100, 3, 3 },
  { 6, 127, IMCSONGOSCTYPE_SINE, 2, -1, 206, 10, 11 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 2, -1, 186, 12, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 2, 6, 100, 3, 3 },
  { 6, 127, IMCSONGOSCTYPE_SINE, 3, -1, 206, 14, 15 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 3, -1, 186, 16, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 3, 9, 100, 3, 3 },
  { 6, 127, IMCSONGOSCTYPE_SINE, 4, -1, 206, 18, 19 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 4, -1, 186, 20, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 4, 12, 100, 3, 3 },
  { 6, 127, IMCSONGOSCTYPE_SINE, 5, -1, 206, 22, 23 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 5, -1, 186, 24, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 5, 15, 100, 3, 3 },
  { 6, 127, IMCSONGOSCTYPE_SINE, 6, -1, 206, 26, 27 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 6, -1, 186, 28, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 6, 18, 100, 3, 3 },
  { 6, 127, IMCSONGOSCTYPE_SINE, 7, -1, 206, 30, 31 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 7, -1, 186, 32, 3 },
  { 7, 0, IMCSONGOSCTYPE_NOISE, 7, 21, 100, 3, 3 },
};
unsigned char BLOCK_ImcSongChannelVol[8] = {130, 130, 130, 130, 130, 130, 130, 130 };
unsigned char BLOCK_ImcSongChannelEnvCounter[8] = {0, 5, 9, 13, 17, 21, 25, 29 };
bool BLOCK_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataBLOCK = {
  0, 0, BLOCK_IMCSONG_ENVLISTSIZE, BLOCK_IMCSONG_ENVCOUNTERLISTSIZE, BLOCK_IMCSONG_OSCLISTSIZE, BLOCK_IMCSONG_EFFECTLISTSIZE, BLOCK_IMCSONG_VOL,
  NULL,NULL, NULL, BLOCK_ImcSongEnvList, BLOCK_ImcSongEnvCounterList, BLOCK_ImcSongOscillatorList, NULL,
  BLOCK_ImcSongChannelVol, BLOCK_ImcSongChannelEnvCounter, BLOCK_ImcSongChannelStopNote };
ZL_SynthImcTrack imcBlock = ZL_SynthImcTrack(&imcDataBLOCK).Play();
unsigned char imcBlockCounter = 0;

//-------------------------------------------------------------------------------

#define IMCDISCHARGE_IMCSONG_LEN 0x1
#define IMCDISCHARGE_IMCSONG_ROWLENSAMPLES 2594
#define IMCDISCHARGE_IMCSONG_ENVLISTSIZE 5
#define IMCDISCHARGE_IMCSONG_ENVCOUNTERLISTSIZE 6
#define IMCDISCHARGE_IMCSONG_OSCLISTSIZE 11
#define IMCDISCHARGE_IMCSONG_EFFECTLISTSIZE 2
#define IMCDISCHARGE_IMCSONG_VOL 100
unsigned int IMCDISCHARGE_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char IMCDISCHARGE_ImcSongPatternData[] = {
  0x50, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
unsigned char IMCDISCHARGE_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope IMCDISCHARGE_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 86, 8, 16, 255, true, 255, },
  { 0, 256, 53, 10, 255, 255, true, 255, },
  { 0, 256, 184, 3, 21, 255, true, 255, },
  { 0, 256, 92, 8, 10, 255, true, 255, },
};
TImcSongEnvelopeCounter IMCDISCHARGE_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 256 }, { 2, 0, 248 }, { -1, -1, 256 },
  { 3, 0, 206 }, { 4, 0, 256 },
};
TImcSongOscillator IMCDISCHARGE_ImcSongOscillatorList[] = {
  { 9, 48, IMCSONGOSCTYPE_SQUARE, 0, -1, 146, 1, 2 },
  { 6, 66, IMCSONGOSCTYPE_SQUARE, 0, -1, 66, 4, 3 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 0, 0, 8, 3, 3 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 0, 1, 138, 5, 3 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
TImcSongEffect IMCDISCHARGE_ImcSongEffectList[] = {
  { 152, 181, 1, 0, IMCSONGEFFECTTYPE_RESONANCE, 3, 3 },
  { 9271, 194, 1, 0, IMCSONGEFFECTTYPE_OVERDRIVE, 0, 3 },
};
unsigned char IMCDISCHARGE_ImcSongChannelVol[8] = {100, 100, 100, 100, 100, 100, 100, 100 };
unsigned char IMCDISCHARGE_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool IMCDISCHARGE_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataIMCDISCHARGE = {
  IMCDISCHARGE_IMCSONG_LEN, IMCDISCHARGE_IMCSONG_ROWLENSAMPLES, IMCDISCHARGE_IMCSONG_ENVLISTSIZE, IMCDISCHARGE_IMCSONG_ENVCOUNTERLISTSIZE, IMCDISCHARGE_IMCSONG_OSCLISTSIZE, IMCDISCHARGE_IMCSONG_EFFECTLISTSIZE, IMCDISCHARGE_IMCSONG_VOL,
  IMCDISCHARGE_ImcSongOrderTable, IMCDISCHARGE_ImcSongPatternData, IMCDISCHARGE_ImcSongPatternLookupTable, IMCDISCHARGE_ImcSongEnvList, IMCDISCHARGE_ImcSongEnvCounterList, IMCDISCHARGE_ImcSongOscillatorList, IMCDISCHARGE_ImcSongEffectList,
  IMCDISCHARGE_ImcSongChannelVol, IMCDISCHARGE_ImcSongChannelEnvCounter, IMCDISCHARGE_ImcSongChannelStopNote };
ZL_SynthImcTrack imcDischarge(&imcDataIMCDISCHARGE, false);

//-------------------------------------------------------------------------------

#define FULL_IMCSONG_LEN 0x1
#define FULL_IMCSONG_ROWLENSAMPLES 2594
#define FULL_IMCSONG_ENVLISTSIZE 2
#define FULL_IMCSONG_ENVCOUNTERLISTSIZE 3
#define FULL_IMCSONG_OSCLISTSIZE 9
#define FULL_IMCSONG_EFFECTLISTSIZE 0
#define FULL_IMCSONG_VOL 100
unsigned int FULL_ImcSongOrderTable[] = {
  0x000000001,
};
unsigned char FULL_ImcSongPatternData[] = {
  0x62, 0x64, 255,  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
unsigned char FULL_ImcSongPatternLookupTable[] = { 0, 1, 1, 1, 1, 1, 1, 1, };
TImcSongEnvelope FULL_ImcSongEnvList[] = {
  { 0, 256, 65, 8, 16, 4, true, 255, },
  { 0, 256, 370, 8, 12, 255, true, 255, },
};
TImcSongEnvelopeCounter FULL_ImcSongEnvCounterList[] = {
 { 0, 0, 256 }, { 1, 0, 256 }, { -1, -1, 256 },
};
TImcSongOscillator FULL_ImcSongOscillatorList[] = {
  { 9, 66, IMCSONGOSCTYPE_SQUARE, 0, -1, 126, 1, 2 },
  { 7, 66, IMCSONGOSCTYPE_SAW, 0, 0, 242, 2, 2 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 1, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 2, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 3, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 4, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 5, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 6, -1, 100, 0, 0 },
  { 8, 0, IMCSONGOSCTYPE_SINE, 7, -1, 100, 0, 0 },
};
unsigned char FULL_ImcSongChannelVol[8] = {51, 100, 100, 100, 100, 100, 100, 100 };
unsigned char FULL_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool FULL_ImcSongChannelStopNote[8] = {false, false, false, false, false, false, false, false };
TImcSongData imcDataFULL = {
  FULL_IMCSONG_LEN, FULL_IMCSONG_ROWLENSAMPLES, FULL_IMCSONG_ENVLISTSIZE, FULL_IMCSONG_ENVCOUNTERLISTSIZE, FULL_IMCSONG_OSCLISTSIZE, FULL_IMCSONG_EFFECTLISTSIZE, FULL_IMCSONG_VOL,
  FULL_ImcSongOrderTable, FULL_ImcSongPatternData, FULL_ImcSongPatternLookupTable, FULL_ImcSongEnvList, FULL_ImcSongEnvCounterList, FULL_ImcSongOscillatorList, NULL,
  FULL_ImcSongChannelVol, FULL_ImcSongChannelEnvCounter, FULL_ImcSongChannelStopNote };
ZL_SynthImcTrack imcFull(&imcDataFULL, false);

//-------------------------------------------------------------------------------

#define EXPLOSION_IMCSONG_ENVLISTSIZE 3
#define EXPLOSION_IMCSONG_ENVCOUNTERLISTSIZE 25
#define EXPLOSION_IMCSONG_OSCLISTSIZE 16
#define EXPLOSION_IMCSONG_EFFECTLISTSIZE 0
#define EXPLOSION_IMCSONG_VOL 179
TImcSongEnvelope EXPLOSION_ImcSongEnvList[] = {
  { 0, 256, 5, 0, 24, 255, true, 255, },
  { 0, 256, 42, 6, 18, 255, true, 255, },
  { 0, 256, 87, 5, 19, 255, true, 255, },
};
TImcSongEnvelopeCounter EXPLOSION_ImcSongEnvCounterList[] = {
 { -1, -1, 256 }, { 0, 0, 128 }, { 1, 0, 248 }, { 2, 0, 238 },
  { 0, 1, 128 }, { 1, 1, 248 }, { 2, 1, 238 }, { 0, 2, 128 },
  { 1, 2, 248 }, { 2, 2, 238 }, { 0, 3, 128 }, { 1, 3, 248 },
  { 2, 3, 238 }, { 0, 4, 128 }, { 1, 4, 248 }, { 2, 4, 238 },
  { 0, 5, 128 }, { 1, 5, 248 }, { 2, 5, 238 }, { 0, 6, 128 },
  { 1, 6, 248 }, { 2, 6, 238 }, { 0, 7, 128 }, { 1, 7, 248 },
  { 2, 7, 238 },
};
TImcSongOscillator EXPLOSION_ImcSongOscillatorList[] = {
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 0, -1, 80, 1, 2 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 0, 0, 90, 3, 0 },
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 1, -1, 80, 4, 5 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 1, 2, 90, 6, 0 },
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 2, -1, 80, 7, 8 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 2, 4, 90, 9, 0 },
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 3, -1, 80, 10, 11 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 3, 6, 90, 12, 0 },
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 4, -1, 80, 13, 14 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 4, 8, 90, 15, 0 },
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 5, -1, 80, 16, 17 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 5, 10, 90, 18, 0 },
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 6, -1, 80, 19, 20 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 6, 12, 90, 21, 0 },
  { 6, 169, IMCSONGOSCTYPE_SQUARE, 7, -1, 80, 22, 23 },
  { 8, 0, IMCSONGOSCTYPE_NOISE, 7, 14, 90, 24, 0 },
};
unsigned char EXPLOSION_ImcSongChannelVol[8] = {99, 99, 99, 99, 99, 99, 99, 99 };
unsigned char EXPLOSION_ImcSongChannelEnvCounter[8] = {0, 0, 0, 0, 0, 0, 0, 0 };
bool EXPLOSION_ImcSongChannelStopNote[8] = {true, true, true, true, true, true, true, true };
TImcSongData imcDataEXPLOSION = {
  0, 0, EXPLOSION_IMCSONG_ENVLISTSIZE, EXPLOSION_IMCSONG_ENVCOUNTERLISTSIZE, EXPLOSION_IMCSONG_OSCLISTSIZE, EXPLOSION_IMCSONG_EFFECTLISTSIZE, EXPLOSION_IMCSONG_VOL,
  NULL, NULL, NULL, EXPLOSION_ImcSongEnvList, EXPLOSION_ImcSongEnvCounterList, EXPLOSION_ImcSongOscillatorList, NULL,
  EXPLOSION_ImcSongChannelVol, EXPLOSION_ImcSongChannelEnvCounter, EXPLOSION_ImcSongChannelStopNote };
ZL_SynthImcTrack imcExplosion = ZL_SynthImcTrack(&imcDataEXPLOSION).Play();
unsigned char imcExplosionCounter = 0;

//-------------------------------------------------------------------------------
