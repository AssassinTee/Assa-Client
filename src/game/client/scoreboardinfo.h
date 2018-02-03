/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_SCOREBOARDI_H
#define GAME_CLIENT_SCOREBOARDI_H

#include <base/vmath.h>
#include <engine/client.h>
#include <engine/console.h>
#include <game/layers.h>
#include <game/gamecore.h>

class CScoreboardInfo
{
public:
	struct CScoreboard
	{
		int Selfkills;
		int Flaggraps;
		int Hammerkills;
		int Gunkills;
		int Shotgunkills;
		int Grenadekills;
		int Riflekills;
	};
	CScoreboard m_ScoreboardInfo[MAX_CLIENTS];
};

#endif
