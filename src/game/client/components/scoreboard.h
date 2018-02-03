/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SCOREBOARD_H
#define GAME_CLIENT_COMPONENTS_SCOREBOARD_H
#include <game/client/component.h>

class CScoreboard : public CComponent
{
	class CColumn
	{
	public:
		enum
		{
			ALIGN_NOTEXT=-1,
			ALIGN_LEFT,
			ALIGN_RIGHT,
			ALIGN_MIDDLE,
		};

		CColumn(const char* pTitle, float Width, float Offset, int RenderAlign);

		const char* m_pTitle;
		float m_Width;
		float m_Offset;
		bool m_Active;
		int m_RenderAlign;
	};

	static CColumn ms_Scoreboard[];
	static CColumn ms_Spectatorboard[];

	void SetActiveColumns();

	void RenderGoals(CUIRect View, float Scroll);
	void RenderSpectators(float x, float y, float Width, float Height, int NumSpectators, bool TeamPlay);
	void RenderScoreboard(float x, float y, float Width, float Height, int Team, const char *pTitle, bool TeamPlay);

	static void ConKeyScoreboard(IConsole::IResult *pResult, void *pUserData);

	const char *GetClanName(int Team);

	bool m_Active;
	vec4 m_ScoreboardPosition;
	float m_ScoreboardScroll;

public:
	CScoreboard();
	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual bool OnInput(IInput::CEvent Event);
	virtual void OnRender();
	virtual void OnRelease();

	bool Active();
	vec4 GetScoreboardPosition();
};

#endif
