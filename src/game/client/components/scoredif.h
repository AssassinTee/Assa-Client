/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_ASSAKILLMESSAGES_H
#define GAME_CLIENT_COMPONENTS_ASSAKILLMESSAGES_H
#include <game/client/component.h>

class CAssaMessages : public CComponent
{
public:
	// kill messages
	struct CKillMsg
	{
		int m_KillerID;

		int m_AssaTick;
	};

	enum
	{
		MAX_KILLMSGS = 5,
	};

	CKillMsg m_aKillmsgs[MAX_KILLMSGS];
	int m_KillmsgAssa;
	int m_OldScore;//[MAX_KILLMSGS];
	//int m_CurrentScore[MAX_KILLMSGS];
	int m_ScoreDif[MAX_KILLMSGS];
	int m_CurrentScoreDif;
	//int m_LastOldScore;
	int y2;

	virtual void OnReset();
	virtual void OnRender();
	void OnClientInfo();
	void OnSpectator();
};

#endif
