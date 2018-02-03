/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/keys.h>

#include <game/generated/client_data.h>
#include <game/generated/protocol.h>
#include <game/localization.h>
#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/teecomp.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/motd.h>
#include <game/client/components/skins.h>
#include <game/client/components/teecomp_stats.h>

#include "scoreboard.h"
 
// enums for columns
enum
{
	COLUMN_SCORE_SCORE = 0,
	COLUMN_SCORE_TEE,
	COLUMN_SCORE_NAME,
	COLUMN_SCORE_CLAN,
	COLUMN_SCORE_COUNTRY,
	COLUMN_SCORE_PING,
	COLUMN_SCORE_ID,
	NUM_COLUMNS_SCORE,

	COLUMN_SPEC_SCORE = 0,
	COLUMN_SPEC_TEE,
	COLUMN_SPEC_NAME,
	COLUMN_SPEC_CLAN,
	COLUMN_SPEC_COUNTRY,
	COLUMN_SPEC_ID,
	NUM_COLUMNS_SPEC
};

CScoreboard::CColumn CScoreboard::ms_Scoreboard[] = {CScoreboard::CColumn(Localize("Score"), 60.0f, 10.0f, CScoreboard::CColumn::ALIGN_RIGHT),
																CScoreboard::CColumn(0, 48.0f, 0.0f, CScoreboard::CColumn::ALIGN_NOTEXT),
																CScoreboard::CColumn(Localize("Name"), 250.0f, 0.0f, CScoreboard::CColumn::ALIGN_LEFT),
																CScoreboard::CColumn(Localize("Clan"), 145.0f, 0.0f, CScoreboard::CColumn::ALIGN_MIDDLE),
																CScoreboard::CColumn(0, 72.0f, 0.0f, CScoreboard::CColumn::ALIGN_NOTEXT),
																CScoreboard::CColumn(Localize("Ping"), 66.0f, 0.0f, CScoreboard::CColumn::ALIGN_LEFT),
																CScoreboard::CColumn("ID", 60.0f, 0.0f, CScoreboard::CColumn::ALIGN_MIDDLE)};
CScoreboard::CColumn CScoreboard::ms_Spectatorboard[] = {CScoreboard::CColumn(Localize("Score"), 60.0f, 10.0f, CScoreboard::CColumn::ALIGN_RIGHT),//Added fucking score you bastards... love you assa
																CScoreboard::CColumn(0, 48.0f, 0.0f, CScoreboard::CColumn::ALIGN_NOTEXT),
																CScoreboard::CColumn(Localize("Name"), 250.0f, 0.0f, CScoreboard::CColumn::ALIGN_LEFT),
																CScoreboard::CColumn(Localize("Clan"), 145.0f, 0.0f, CScoreboard::CColumn::ALIGN_MIDDLE),
																CScoreboard::CColumn(0, 72.0f, 0.0f, CScoreboard::CColumn::ALIGN_NOTEXT),
																CScoreboard::CColumn("ID", 60.0f, 0.0f, CScoreboard::CColumn::ALIGN_MIDDLE)};

CScoreboard::CColumn::CColumn(const char* pTitle, float Width, float Offset, int RenderAlign)
: m_pTitle(pTitle), m_Width(Width), m_Offset(Offset), m_RenderAlign(RenderAlign)
{
	m_Active = true;
}

CScoreboard::CScoreboard()
{
	OnReset();
}

void CScoreboard::ConKeyScoreboard(IConsole::IResult *pResult, void *pUserData)
{
	((CScoreboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CScoreboard::OnReset()
{
	m_Active = false;
	m_ScoreboardScroll = 0;
}

void CScoreboard::OnRelease()
{
	m_Active = false;
}

void CScoreboard::OnConsoleInit()
{
	Console()->Register("+scoreboard", "", CFGFLAG_CLIENT, ConKeyScoreboard, this, "Show scoreboard");
}

void CScoreboard::RenderGoals(CUIRect View, float Scroll)
{
	if(!m_pClient->m_Snap.m_pGameInfoObj || Scroll > 0)
		return;

	CUIRect Top, Middle, Bottom;
	View.HSplitTop(40.0f, &Top, &View);
	RenderTools()->DrawUIRect(&Top, g_Config.m_TcScoreboardInfos&TC_SCORE_NOCOLORHEADERS ? vec4(0.0f, 0.0f, 0.0f, 0.5f) : vec4(0.5f, 0.5f, 0.5f, 0.5f), 0, 0.0f);
	RenderTools()->DrawUIRect(&View, vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_B, 40.0f);

	View.HSplitMid(&Middle, &Bottom);

	CUIRect Left, MiddleLeft, MiddleRight, Right;
	Middle.VSplitMid(&Left, &Right);
	Left.VSplitMid(&Left, &MiddleLeft);
	Right.VSplitMid(&MiddleRight, &Right);

	float ServerFontSize = 27.0f;
	float DataFontSize = 22.0f;

	CServerInfo CurrentServerInfo;
	Client()->GetServerInfo(&CurrentServerInfo);

	char aBuf[64];
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
		str_copy(aBuf, CurrentServerInfo.m_aName, sizeof(aBuf));
	else
		DemoPlayer()->GetDemoName(aBuf, sizeof(aBuf));

	float tw = TextRender()->TextWidth(0, ServerFontSize, aBuf, -1);
	if(tw > Top.w-10.0f)
		tw = Top.w-10.0f;
	CTextCursor Cursor;
	TextRender()->SetCursor(&Cursor, Top.x+Top.w/2-tw/2, Top.y, ServerFontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
	Cursor.m_LineWidth = tw;
	TextRender()->TextEx(&Cursor, aBuf, -1);

	// draw middle line
	CUIRect Label;
	Left.HSplitTop(10.0f, 0, &Label);
	if(m_pClient->m_Snap.m_pGameInfoObj->m_ScoreLimit)
		str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Score limit"), m_pClient->m_Snap.m_pGameInfoObj->m_ScoreLimit);
	else
		str_format(aBuf, sizeof(aBuf), "%s: -", Localize("Score limit"));
	UI()->DoLabel(&Label, aBuf, DataFontSize, 0);

	MiddleLeft.HSplitTop(10.0f, 0, &Label);
	if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit)
		str_format(aBuf, sizeof(aBuf), Localize("Time limit: %d min"), m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit);
	else
		str_format(aBuf, sizeof(aBuf), Localize("%s: -"), Localize("Time limit"));
	UI()->DoLabel(&Label, aBuf, DataFontSize, 0);

	MiddleRight.HSplitTop(10.0f, 0, &Label);
	if(m_pClient->m_Snap.m_pGameInfoObj->m_RoundNum && m_pClient->m_Snap.m_pGameInfoObj->m_RoundCurrent)
		str_format(aBuf, sizeof(aBuf), "%s: %d/%d", Localize("Round"), m_pClient->m_Snap.m_pGameInfoObj->m_RoundCurrent, m_pClient->m_Snap.m_pGameInfoObj->m_RoundNum);
	else
		str_format(aBuf, sizeof(aBuf), "%s: -/-", Localize("Round"));
	UI()->DoLabel(&Label, aBuf, DataFontSize, 0);

	Right.HSplitTop(10.0f, 0, &Label);
	str_format(aBuf, sizeof(aBuf), "%s: %d/%d", Localize("Players"), m_pClient->m_Snap.m_NumPlayers, CurrentServerInfo.m_MaxClients);
	UI()->DoLabel(&Label, aBuf, DataFontSize, 0);

	if(m_pClient->DemoRecorder()->IsRecording())
	{
		Bottom.VSplitMid(&Left, &Right);
		Left.VSplitMid(&Left, &MiddleLeft);
		Right.VSplitMid(&MiddleRight, &Right);
	}
	else
	{
		Bottom.VSplitLeft(Bottom.w/3.0f, &Left, &Right);
		Right.VSplitMid(&MiddleLeft, &MiddleRight);
	}

	// draw bottom line
	str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Game type"), Client()->State() == IClient::STATE_DEMOPLAYBACK ? "-" : CurrentServerInfo.m_aGameType);
	tw = TextRender()->TextWidth(0, DataFontSize, aBuf, -1);
	if(tw > Left.w-20.0f)
		tw = Left.w-20.0f;
	TextRender()->SetCursor(&Cursor, Left.x+Left.w/2-tw/2, Left.y, DataFontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
	Cursor.m_LineWidth = tw;
	TextRender()->TextEx(&Cursor, aBuf, -1);
	
	int Time = 0;
	if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
	{
		Time = m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed());

		if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
			Time = 0;
	}
	else
		Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed();
	str_format(aBuf, sizeof(aBuf), "%s: %d:%02d", Localize("Time"), Time/60, Time%60);
	UI()->DoLabel(&MiddleLeft, aBuf, DataFontSize, 0);

	if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
		str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Map"), CurrentServerInfo.m_aMap);
	else
		str_format(aBuf, sizeof(aBuf), "%s: -", Localize("Map"));
	tw = TextRender()->TextWidth(0, DataFontSize, aBuf, -1);
	if(tw > MiddleRight.w-20.0f)
		tw = MiddleRight.w-20.0f;
	TextRender()->SetCursor(&Cursor, MiddleRight.x+MiddleRight.w/2-tw/2, MiddleRight.y, DataFontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
	Cursor.m_LineWidth = tw;
	TextRender()->TextEx(&Cursor, aBuf, -1);

	if(!m_pClient->DemoRecorder()->IsRecording())
		return;

	// get time
	int Seconds = m_pClient->DemoRecorder()->Length();
	str_format(aBuf, sizeof(aBuf), Localize("REC %3d:%02d"), Seconds/60, Seconds%60);
	tw = TextRender()->TextWidth(0, DataFontSize, aBuf, -1);
	tw += 30.0f; // record dot

	//draw the red dot
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);	
	RenderTools()->DrawRoundRect(Right.x+Right.w/2.0f-tw/2.0f, Right.y+5.0f, 20.0f, 20.0f, 10.0f);
	Graphics()->QuadsEnd();

	//draw the text
	tw -= 60.0f;
	TextRender()->SetCursor(&Cursor, Right.x+Right.w/2-tw/2, Right.y, DataFontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
	Cursor.m_LineWidth = tw+30.0f;
	TextRender()->TextEx(&Cursor, aBuf, -1);
}

void CScoreboard::RenderSpectators(float x, float y, float Width, float Height, int NumSpectators, bool TeamPlay)
{
	if(!NumSpectators)
		return;

	float HeadlineFontsize = 22.0f;
	float HeadlineHeight = 30.0f;
	float TitleFontsize = 30.0f;
	float TitleHight = 50.0f;
	float LineHeight = 50.0f;
	float TeeSizeMod = 0.8f;

	// background
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	if(g_Config.m_TcScoreboardInfos&TC_SCORE_NOCOLORHEADERS)
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
	else
		Graphics()->SetColor(0.5f, 0.5f, 0.5f, 0.5f);
	RenderTools()->DrawRoundRectExt(x, y, Width, TitleHight, 15.0f, CUI::CORNER_T);
	if(g_Config.m_TcScoreboardInfos&TC_SCORE_TITLE)
	{
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, g_Config.m_TcScoreboardInfos&TC_SCORE_NOCOLORHEADERS ? 0.5f : 0.25f);
		RenderTools()->DrawRoundRect(x, y+TitleHight, Width, HeadlineHeight, 0.0f);
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
		RenderTools()->DrawRoundRectExt(x, y+TitleHight+HeadlineHeight, Width, Height-TitleHight-HeadlineHeight, 15.0f, CUI::CORNER_B);
		if(TeamPlay && !(g_Config.m_TcScoreboardInfos&TC_SCORE_HIDESEPERATOR))
		{
			Graphics()->SetColor(0.5f, 0.5f, 0.5f, 0.25f);
			RenderTools()->DrawRoundRect(x+Width/2-2.5f, y+TitleHight+HeadlineHeight+5.0f, 5.0f, Height-TitleHight-HeadlineHeight-10.0f, 2.0f);
		}
	}
	else
	{
		Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
		RenderTools()->DrawRoundRectExt(x, y+TitleHight, Width, Height-TitleHight, 15.0f, CUI::CORNER_B);
		if(TeamPlay && !(g_Config.m_TcScoreboardInfos&TC_SCORE_HIDESEPERATOR))
		{
			Graphics()->SetColor(0.5f, 0.5f, 0.5f, 0.25f);
			RenderTools()->DrawRoundRect(x+Width/2-2.5f, y+TitleHight+5.0f, 5.0f, Height-TitleHight-10.0f, 2.0f);
		}
	}
	Graphics()->QuadsEnd();

	float w = 0.0f;
	for(int i = 0; i < NUM_COLUMNS_SPEC; i++)
	{
		if(ms_Spectatorboard[i].m_Active)
			w += ms_Spectatorboard[i].m_Width;
	}

	// Headline
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s (%d)", Localize("Spectators"), NumSpectators);
	float tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
	TextRender()->Text(0, x+Width/2.0f-tw/2.0f, y, TitleFontsize, aBuf, Width-20.0f);

	y += TitleHight;

	// render column titles
	float TmpX = x+Width/2.0f-w/2.0f;
	if(g_Config.m_TcScoreboardInfos&TC_SCORE_TITLE)
	{
		for(int i = 0; i < NUM_COLUMNS_SPEC; i++)
		{
			if(!ms_Spectatorboard[i].m_Active)
				continue;

			if(ms_Spectatorboard[i].m_RenderAlign != CColumn::ALIGN_NOTEXT)
			{
				if(ms_Spectatorboard[i].m_RenderAlign == CColumn::ALIGN_LEFT)
					TextRender()->Text(0, TmpX+ms_Spectatorboard[i].m_Offset, y, HeadlineFontsize, ms_Spectatorboard[i].m_pTitle, -1);
				else if(ms_Spectatorboard[i].m_RenderAlign == CColumn::ALIGN_RIGHT)
				{
					tw = TextRender()->TextWidth(0, HeadlineFontsize, ms_Spectatorboard[i].m_pTitle, -1);
					TextRender()->Text(0, (TmpX+ms_Spectatorboard[i].m_Width)-tw, y, HeadlineFontsize, ms_Spectatorboard[i].m_pTitle, -1);
				}
				else
				{
					tw = TextRender()->TextWidth(0, HeadlineFontsize, ms_Spectatorboard[i].m_pTitle, -1);
					TextRender()->Text(0, TmpX+ms_Spectatorboard[i].m_Offset+ms_Spectatorboard[i].m_Width/2-tw/2, y, HeadlineFontsize, ms_Spectatorboard[i].m_pTitle, -1);
				}
			}

			TmpX += ms_Spectatorboard[i].m_Width;
		}

		y += HeadlineHeight;
	}

	y += 15.0f; // little space

	// render player entries
	float FontSize = 24.0f;
	CTextCursor Cursor;
	int PlayerNum = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paPlayerInfos[i];
		if(!pInfo || pInfo->m_Team != TEAM_SPECTATORS)
			continue;

		if(TeamPlay)
		{
			PlayerNum++;

			if(PlayerNum % 2)
				TmpX = x+Width/4.0f-w/2.0f;
			else
				TmpX = x+Width-Width/4.0f-w/2.0f;
		}
		else
			TmpX = x+Width/2.0f-w/2.0f;

		for (int j = 0; j < NUM_COLUMNS_SPEC; j++)
		{
			if(!ms_Spectatorboard[j].m_Active)
				continue;

			if(j == COLUMN_SPEC_SCORE)
			{
				str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Score, -9999, 99999));
				tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
				TextRender()->SetCursor(&Cursor, TmpX+ms_Spectatorboard[j].m_Offset+ms_Spectatorboard[j].m_Width/2-tw/2, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Spectatorboard[j].m_Width;
				TextRender()->TextEx(&Cursor, aBuf, -1);
			}
			else if(j == COLUMN_SPEC_TEE)
			{
				// tee
				CTeeRenderInfo TeeInfo = m_pClient->m_aClients[pInfo->m_ClientID].m_RenderInfo;
				TeeInfo.m_Size *= TeeSizeMod;
				RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(TmpX+ms_Spectatorboard[j].m_Offset+ms_Spectatorboard[j].m_Width/2, y+LineHeight/2));
			}
			else if(j == COLUMN_SPEC_NAME)
			{
				TextRender()->SetCursor(&Cursor, TmpX+ms_Spectatorboard[j].m_Offset, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Spectatorboard[j].m_Width;
				TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aName, -1);
			}
			else if(j == COLUMN_SPEC_CLAN)
			{
				tw = TextRender()->TextWidth(0, FontSize, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
				TextRender()->SetCursor(&Cursor, tw >= ms_Spectatorboard[j].m_Width ? TmpX+ms_Spectatorboard[j].m_Offset : TmpX+ms_Spectatorboard[j].m_Offset+ms_Spectatorboard[j].m_Width/2-tw/2, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Spectatorboard[j].m_Width;
				TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
			}
			else if(j == COLUMN_SPEC_COUNTRY)
			{
				vec4 Color(1.0f, 1.0f, 1.0f, 0.5f);
				m_pClient->m_pCountryFlags->Render(m_pClient->m_aClients[pInfo->m_ClientID].m_Country, &Color,
													TmpX+ms_Spectatorboard[j].m_Offset, y+5+(TeeSizeMod*5.0f)/2.0f, ms_Spectatorboard[j].m_Width, 40.0f-TeeSizeMod*5.0f);
			}
			else if(j == COLUMN_SPEC_ID)
			{
				str_format(aBuf, sizeof(aBuf), "%d", pInfo->m_ClientID);
				tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
				TextRender()->SetCursor(&Cursor, TmpX + ms_Spectatorboard[j].m_Offset + ms_Spectatorboard[j].m_Width - tw, y, FontSize, TEXTFLAG_RENDER | TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Spectatorboard[j].m_Width;
				TextRender()->TextEx(&Cursor, aBuf, -1);
			}
			TmpX += ms_Spectatorboard[j].m_Width;
		}
		if(!TeamPlay || PlayerNum % 2 == 0)
			y += LineHeight;
	}
}

void CScoreboard::RenderScoreboard(float x, float y, float Width, float Height, int Team, const char *pTitle, bool TeamPlay)
{
	if(Team == TEAM_SPECTATORS)
		return;

	float HeadlineFontsize = 22.0f;
	float HeadlineHeight = 30.0f;
	float TitleFontsize = 30.0f;
	float TitleHight = 50.0f;
	float LineHeight = 50.0f;
	float TeeSizeMod = 0.8f;

	// background
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	if(!TeamPlay)
	{
		if(g_Config.m_TcScoreboardInfos&TC_SCORE_NOCOLORHEADERS)
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
		else
			Graphics()->SetColor(0.5f, 0.5f, 0.5f, 0.5f);
		RenderTools()->DrawRoundRectExt(x, y, Width, TitleHight, 15.0f, CUI::CORNER_T);
		if(g_Config.m_TcScoreboardInfos&TC_SCORE_TITLE)
		{
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, g_Config.m_TcScoreboardInfos&TC_SCORE_NOCOLORHEADERS ? 0.5f : 0.25f);
			RenderTools()->DrawRoundRect(x, y+TitleHight, Width, HeadlineHeight, 0.0f);
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
			RenderTools()->DrawRoundRectExt(x, y+TitleHight+HeadlineHeight, Width, Height-TitleHight-HeadlineHeight, 15.0f, CUI::CORNER_B);
		}
		else
		{
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
			RenderTools()->DrawRoundRectExt(x, y+TitleHight, Width, Height-TitleHight, 15.0f, CUI::CORNER_B);
		}
	}
	else
	{
		if(g_Config.m_TcScoreboardInfos&TC_SCORE_NOCOLORHEADERS)
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
		else
		{
			if(CTeecompUtils::GetForceDmColors(Team, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED))
			{
				if(Team == TEAM_RED)
					Graphics()->SetColor(1.0f, 0.0f, 0.0f, g_Config.m_TcScoreboardInfos&TC_SCORE_HIDEBORDER ? 0.5f : 0.25f);
				else
					Graphics()->SetColor(0.0f, 0.0f, 1.0f, g_Config.m_TcScoreboardInfos&TC_SCORE_HIDEBORDER ? 0.5f : 0.25f);
			}
			else
			{
				vec3 TeamColor = CTeecompUtils::GetTeamColor(Team, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED, g_Config.m_TcColoredTeesTeam1,
										g_Config.m_TcColoredTeesTeam2, g_Config.m_TcColoredTeesMethod);
				Graphics()->SetColor(TeamColor.r, TeamColor.g, TeamColor.b, g_Config.m_TcScoreboardInfos&TC_SCORE_HIDEBORDER ? 0.5f : 0.25f);
			}
		}
		RenderTools()->DrawRoundRectExt(x, y, Width, TitleHight, 15.0f, CUI::CORNER_T);
		if(g_Config.m_TcScoreboardInfos&TC_SCORE_TITLE)
		{
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, g_Config.m_TcScoreboardInfos&TC_SCORE_NOCOLORHEADERS ? 0.5f : 0.25f);
			RenderTools()->DrawRoundRect(x, y+TitleHight, Width, HeadlineHeight, 0.0f);
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
			RenderTools()->DrawRoundRectExt(x, y+TitleHight+HeadlineHeight, Width, Height-TitleHight-HeadlineHeight, 15.0f, CUI::CORNER_B);
		}
		else
		{
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
			RenderTools()->DrawRoundRectExt(x, y+TitleHight, Width, Height-TitleHight, 15.0f, CUI::CORNER_B);
		}
	}
	Graphics()->QuadsEnd();
	
	float w = 0.0f;
	for (int i = 0; i < NUM_COLUMNS_SCORE; i++)
	{
		if(i == COLUMN_SCORE_SCORE)
		{
			ms_Scoreboard[i].m_Width = 135.0f;
			ms_Scoreboard[i].m_RenderAlign = CColumn::ALIGN_MIDDLE;
		}

		if(ms_Scoreboard[i].m_Active)
			w += ms_Scoreboard[i].m_Width;
	}

	// render title
	char aBuf[128];
	if(!pTitle)
	{
		if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
			pTitle = Localize("Game over");
		else
			pTitle = Localize("Score board");
	}
	str_format(aBuf, sizeof(aBuf), "%s (%d)", pTitle, m_pClient->m_Snap.m_aTeamSize[Team]);
	TextRender()->Text(0, x+20.0f, y, TitleFontsize, aBuf, -1);
	
	float tw = 0.0f;
	if(TeamPlay)
	{
		if(m_pClient->m_Snap.m_pGameDataObj)
		{
			int Score = Team == TEAM_RED ? m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed : m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue;
			str_format(aBuf, sizeof(aBuf), "%d", Score);

			tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
			TextRender()->Text(0, x+Width-tw-20.0f, y, TitleFontsize, aBuf, -1);
		}
	}
	else
	{
		if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW &&
			m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID])
		{
			int Score = m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID]->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);

			tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
			TextRender()->Text(0, x+Width-tw-20.0f, y, TitleFontsize, aBuf, -1);
		}
		else if(m_pClient->m_Snap.m_pLocalInfo)
		{
			int Score = m_pClient->m_Snap.m_pLocalInfo->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);

			tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
			TextRender()->Text(0, x+Width-tw-20.0f, y, TitleFontsize, aBuf, -1);
		}
	}

	y += TitleHight;

	// render column titles
	float TmpX = x+Width/2.0f-w/2.0f;
	if(g_Config.m_TcScoreboardInfos&TC_SCORE_TITLE)
	{
		for (int i = 0; i < NUM_COLUMNS_SCORE; i++)
		{
			if(!ms_Scoreboard[i].m_Active)
				continue;

			if(ms_Scoreboard[i].m_RenderAlign != CColumn::ALIGN_NOTEXT)
			{
				if(ms_Scoreboard[i].m_RenderAlign == CColumn::ALIGN_LEFT)
					TextRender()->Text(0, TmpX+ms_Scoreboard[i].m_Offset, y, HeadlineFontsize, ms_Scoreboard[i].m_pTitle, -1);
				else if(ms_Scoreboard[i].m_RenderAlign == CColumn::ALIGN_RIGHT)
				{
					tw = TextRender()->TextWidth(0, HeadlineFontsize, ms_Scoreboard[i].m_pTitle, -1);
					TextRender()->Text(0, (TmpX+ms_Scoreboard[i].m_Width)-tw, y, HeadlineFontsize, ms_Scoreboard[i].m_pTitle, -1);
				}
				else
				{
					tw = TextRender()->TextWidth(0, HeadlineFontsize, ms_Scoreboard[i].m_pTitle, -1);
					TextRender()->Text(0, TmpX+ms_Scoreboard[i].m_Offset+ms_Scoreboard[i].m_Width/2-tw/2, y, HeadlineFontsize, ms_Scoreboard[i].m_pTitle, -1);
				}
			}

			TmpX += ms_Scoreboard[i].m_Width;
		}

		y += HeadlineHeight;
	}

	y += 15.0f; // little space

	// render player entries
	float FontSize = 24.0f;
	CTextCursor Cursor;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// make sure that we render the correct team
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];
		if(!pInfo || pInfo->m_Team != Team)
			continue;
		
		// background so it's easy to find the local player or the followed one in spectator mode
		if(pInfo->m_Local || (m_pClient->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientID == m_pClient->m_Snap.m_SpecInfo.m_SpectatorID))
		{
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			//Dat MUCH and only for color oO
			vec3 Color;
			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
			{
				if(CTeecompUtils::GetForceDmColors(Team, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED))
				{
					if(Team == TEAM_RED)
						Graphics()->SetColor(1.0f, 0.0f, 0.0f, g_Config.m_TcScoreboardInfos&TC_SCORE_HIDEBORDER ? 0.5f : 0.25f);
					else
						Graphics()->SetColor(0.0f, 0.0f, 1.0f, g_Config.m_TcScoreboardInfos&TC_SCORE_HIDEBORDER ? 0.5f : 0.25f);
				}
				else
				{
					Color = CTeecompUtils::GetTeamColor(pInfo->m_Team, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED, 
						g_Config.m_TcColoredTeesTeam1, g_Config.m_TcColoredTeesTeam2, g_Config.m_TcColoredTeesMethod);
				}
			}
			else
			{
				if(m_pClient->m_aClients[pInfo->m_ClientID].m_UseCustomColor)
					Color = m_pClient->m_pSkins->GetColorV3(m_pClient->m_aClients[pInfo->m_ClientID].m_ColorBody);
				else
				{
					const CSkins::CSkin *s = m_pClient->m_pSkins->Get(m_pClient->m_aClients[pInfo->m_ClientID].m_SkinID);
					if(s)
						Color = s->m_BloodColor;
				}
				if(m_pClient->m_Snap.m_aCharacters[pInfo->m_ClientID].m_Cur.m_Weapon == WEAPON_NINJA)//dat funktion, w->x.y.z = v
					Color = vec3(0,0,0);
			}
			Graphics()->SetColor(Color.r, Color.g, Color.b, 0.25f);
			RenderTools()->DrawRoundRect(x+10.0f, y, Width-20.0f, LineHeight, 15.0f);
			Graphics()->QuadsEnd();
		}

		TmpX = x+Width/2.0f-w/2.0f;
		for (int j = 0; j < NUM_COLUMNS_SCORE; j++)
		{
			if(!ms_Scoreboard[j].m_Active)
				continue;

			if(j == COLUMN_SCORE_SCORE)
			{
				str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Score, -9999, 99999));
				tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
				TextRender()->SetCursor(&Cursor, TmpX+ms_Scoreboard[j].m_Offset+ms_Scoreboard[j].m_Width/2-tw/2, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Scoreboard[j].m_Width;
				TextRender()->TextEx(&Cursor, aBuf, -1);
			}
			else if(j == COLUMN_SCORE_TEE)
			{
				// flag
				if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_FLAGS &&
					m_pClient->m_Snap.m_pGameDataObj && (m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed == pInfo->m_ClientID ||
					m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue == pInfo->m_ClientID))
				{
					Graphics()->BlendNormal();
					Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
					Graphics()->QuadsBegin();

					RenderTools()->SelectSprite(pInfo->m_Team==TEAM_RED ? SPRITE_FLAG_BLUE : SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

					if(g_Config.m_TcColoredFlags)
					{
						vec3 Col = CTeecompUtils::GetTeamColor(1-pInfo->m_Team, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED, 
							g_Config.m_TcColoredTeesTeam1, g_Config.m_TcColoredTeesTeam2, g_Config.m_TcColoredTeesMethod);
						Graphics()->SetColor(Col.r, Col.g, Col.b, 1.0f);
					}
					
					float Size = LineHeight;
					IGraphics::CQuadItem QuadItem(TmpX+ms_Scoreboard[j].m_Offset, y-10.0f/2.0f-2.0f, Size/2.0f, Size);
					Graphics()->QuadsDrawTL(&QuadItem, 1);
					Graphics()->QuadsEnd();
				}

				// tee
				CTeeRenderInfo TeeInfo = m_pClient->m_aClients[pInfo->m_ClientID].m_RenderInfo;
				TeeInfo.m_Size *= TeeSizeMod;
				RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(TmpX+ms_Scoreboard[j].m_Offset+ms_Scoreboard[j].m_Width/2, y+LineHeight/2));
			}
			else if(j == COLUMN_SCORE_NAME)
			{
				TextRender()->SetCursor(&Cursor, TmpX+ms_Scoreboard[j].m_Offset, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Scoreboard[j].m_Width;
				TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aName, -1);
			}
			else if(j == COLUMN_SCORE_CLAN)
			{
				tw = TextRender()->TextWidth(0, FontSize, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
				TextRender()->SetCursor(&Cursor, tw >= ms_Scoreboard[j].m_Width ? TmpX+ms_Scoreboard[j].m_Offset : TmpX+ms_Scoreboard[j].m_Offset+ms_Scoreboard[j].m_Width/2-tw/2, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Scoreboard[j].m_Width;
				TextRender()->TextEx(&Cursor, m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, -1);
			}
			else if(j == COLUMN_SCORE_COUNTRY)
			{
				vec4 Color(1.0f, 1.0f, 1.0f, 0.5f);
				m_pClient->m_pCountryFlags->Render(m_pClient->m_aClients[pInfo->m_ClientID].m_Country, &Color,
													TmpX+ms_Scoreboard[j].m_Offset, y+5+(TeeSizeMod*5.0f)/2.0f, ms_Scoreboard[j].m_Width, 40.0f-TeeSizeMod*5.0f);
			}
			else if(j == COLUMN_SCORE_PING)
			{
				str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_Latency, 0, 1000));
				tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
				TextRender()->SetCursor(&Cursor, TmpX+ms_Scoreboard[j].m_Offset+ms_Scoreboard[j].m_Width-tw, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Scoreboard[j].m_Width;
				TextRender()->TextEx(&Cursor, aBuf, -1);
			}
			else if(j == COLUMN_SCORE_ID)
			{
				str_format(aBuf, sizeof(aBuf), "%d", pInfo->m_ClientID);
				tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
				TextRender()->SetCursor(&Cursor, TmpX+ms_Scoreboard[j].m_Offset+ms_Scoreboard[j].m_Width-tw, y, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = ms_Scoreboard[j].m_Width;
				TextRender()->TextEx(&Cursor, aBuf, -1);
			}

			TmpX += ms_Scoreboard[j].m_Width;
		}

		y += LineHeight;
	}
}

bool CScoreboard::OnInput(IInput::CEvent e)
{
	if (!Active())
		return false;
	return false;

}

void CScoreboard::OnRender()
{
	if(!Active())
		return;

	SetActiveColumns();

	// if the score board is active, then we should clear the motd message aswell
	if(m_pClient->m_pMotd->IsActive())
		m_pClient->m_pMotd->Clear();


	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width, Height);
	int NumSpectators = 0;

	if(m_pClient->m_Snap.m_pGameInfoObj)
	{
		float HeadlineHeight = 30.0f;
		float TitleHight = 50.0f;
		float LineHeight = 50.0f;

		bool TeamPlay = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS;

		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paPlayerInfos[i];
			if(pInfo && pInfo->m_Team == TEAM_SPECTATORS)
				NumSpectators++;
		}

		// get scoreboard height
		float ScoreboardHeight = TitleHight+max(m_pClient->m_Snap.m_aTeamSize[TEAM_RED], m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE])*LineHeight+30.0f;
		if(g_Config.m_TcScoreboardInfos&TC_SCORE_TITLE)
			ScoreboardHeight += HeadlineHeight;

		// get spectator height
		float SpectatorHeight = 0.0f;
		if(NumSpectators)
		{
			SpectatorHeight = TitleHight+30.0f;
			if(!TeamPlay)
				SpectatorHeight += NumSpectators * LineHeight;
			else if(NumSpectators > 1)
				SpectatorHeight += (int)((NumSpectators+1)/2.0f) * LineHeight;
			else
				SpectatorHeight += LineHeight;
			if(g_Config.m_TcScoreboardInfos&TC_SCORE_TITLE)
				SpectatorHeight += HeadlineHeight;
		}

		// get scoreboard width
		float ScoreboardWidth = 30.0f;
		for(int i = 0; i < 7; i++)
		{
			if(ms_Scoreboard[i].m_Active)
				ScoreboardWidth += ms_Scoreboard[i].m_Width;
		}

		// render background
		float y = 120.0f;
		float w = ScoreboardWidth+40.0f; // scoreboard width + border
		if(TeamPlay)
			w += ScoreboardWidth+5.0f; // + space between boards
		float h = ScoreboardHeight+40.0f; // spectators + border
		if(NumSpectators)
			h += SpectatorHeight+20.0f; // + team boards + space between
		float x = Width/2.0f-w/2.0f;

		if(!(g_Config.m_TcScoreboardInfos&TC_SCORE_HIDEBORDER))
		{
			Graphics()->BlendNormal();
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
			RenderTools()->DrawRoundRect(x, y+m_ScoreboardScroll, w, h, 40.0f);
			Graphics()->QuadsEnd();
		}

		// set scoreboard position
		m_ScoreboardPosition = vec4(x, y, w, h);

		if(!TeamPlay)
			RenderScoreboard(x+20.0f, y+20.0f+m_ScoreboardScroll, w-40.0f, ScoreboardHeight, 0, 0, false);
		else
		{
			char aText[64];
			const char *pRedClanName = GetClanName(TEAM_RED);
			const char *pBlueClanName = GetClanName(TEAM_BLUE);

			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER && m_pClient->m_Snap.m_pGameDataObj)
			{
				str_copy(aText, Localize("Draw!"), sizeof(aText));
				if(m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed > m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue)
				{
					if(pRedClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pRedClanName);
					else if(CTeecompUtils::GetForceDmColors(TEAM_RED, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED))
						str_copy(aText, Localize("Red team wins!"), sizeof(aText));
					else
					{
						if(m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_BLUE && g_Config.m_TcColoredTeesMethod == 1)
							str_format(aText, sizeof(aText), Localize("%s team wins!"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam2));
						else
							str_format(aText, sizeof(aText), Localize("%s team wins!"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam1));
					}
				}
				else if(m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue > m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed)
				{
					if(pBlueClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pBlueClanName);
					else if(CTeecompUtils::GetForceDmColors(TEAM_BLUE, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED))
						str_copy(aText, Localize("Blue team wins!"), sizeof(aText));
					else
					{
						if(m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_BLUE && g_Config.m_TcColoredTeesMethod == 1)
							str_format(aText, sizeof(aText), Localize("%s team wins!"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam1));
						else
							str_format(aText, sizeof(aText), Localize("%s team wins!"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam2));
					}
				}

				float w = TextRender()->TextWidth(0, 86.0f, aText, -1);
				TextRender()->Text(0, Width/2-w/2, 39, 86.0f, aText, -1);
			}

			if(CTeecompUtils::GetForceDmColors(TEAM_RED, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED))
				str_copy(aText, Localize("Red team"), sizeof(aText));
			else
			{
				if(m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_BLUE && g_Config.m_TcColoredTeesMethod == 1)
					str_format(aText, sizeof(aText), Localize("%s team"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam2));
				else
					str_format(aText, sizeof(aText), Localize("%s team"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam1));
			}
			RenderScoreboard(x+20.0f, y+20.0f+m_ScoreboardScroll, w/2.0f-20.0f-2.5f, ScoreboardHeight, TEAM_RED, pRedClanName ? pRedClanName : Localize(aText), true);
			if(CTeecompUtils::GetForceDmColors(TEAM_BLUE, m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED))
				str_copy(aText, Localize("Blue team"), sizeof(aText));
			else
			{
				if(m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_BLUE && g_Config.m_TcColoredTeesMethod == 1)
					str_format(aText, sizeof(aText), Localize("%s team"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam1));
				else
					str_format(aText, sizeof(aText), Localize("%s team"), CTeecompUtils::RgbToName(g_Config.m_TcColoredTeesTeam2));
			}
			RenderScoreboard(x+20.0f+(w/2.0f-20.0f-2.5f)+5.0f, y+20.0f+m_ScoreboardScroll, w/2.0f-20.0f-2.5f, ScoreboardHeight, TEAM_BLUE, pBlueClanName ? pBlueClanName : Localize(aText), true);
		}

		
		RenderSpectators(x+20.0f, y+20.0f+ScoreboardHeight+20.0f+m_ScoreboardScroll, w-40.0f, SpectatorHeight, NumSpectators, TeamPlay);
		if(g_Config.m_TcScoreboardInfos&TC_SCORE_HIDEBORDER)
		{
			x += 20;
			w -= 40;
		}
		CUIRect Goals = {w < 1050.0f ? Width/2-525.0f : x, 0.0f, w < 1050.0f ? 1050.0f : w, 120.0f};
		RenderGoals(Goals, m_ScoreboardScroll);
	}
	// special handle esc and enter for popup purposes
	if ((Input()->KeyPresses(KEY_MOUSE_WHEEL_UP) && g_Config.m_ScoreboardNatural) || (Input()->KeyPresses(KEY_MOUSE_WHEEL_DOWN) && !g_Config.m_ScoreboardNatural))
	{
		m_ScoreboardScroll -= g_Config.m_ClScoreboardScroll * 50.0f;
		if (m_ScoreboardScroll < -100.0f - (m_pClient->m_Snap.m_aTeamSize[0] + NumSpectators) * 50.0f + 16 * 50.0f)//50.0f
			m_ScoreboardScroll = min(-100.0f - (m_pClient->m_Snap.m_aTeamSize[0] + NumSpectators) * 50.0f + 16 * 50.0f, 0.0f);//Line Hight
	}
	else if ((Input()->KeyPresses(KEY_MOUSE_WHEEL_DOWN) && g_Config.m_ScoreboardNatural) || (Input()->KeyPresses(KEY_MOUSE_WHEEL_UP) && !g_Config.m_ScoreboardNatural))
	{
		m_ScoreboardScroll += g_Config.m_ClScoreboardScroll * 50.0f;
		if (m_ScoreboardScroll > 0) 
			m_ScoreboardScroll = 0;
	}
	
}

void CScoreboard::SetActiveColumns()
{
	ms_Scoreboard[COLUMN_SCORE_CLAN].m_Active = g_Config.m_TcScoreboardInfos&TC_SCORE_CLAN;
	ms_Scoreboard[COLUMN_SCORE_COUNTRY].m_Active = g_Config.m_TcScoreboardInfos&TC_SCORE_COUNTRY;
	ms_Scoreboard[COLUMN_SCORE_PING].m_Active = g_Config.m_TcScoreboardInfos&TC_SCORE_PING;
	ms_Scoreboard[COLUMN_SCORE_ID].m_Active = 1;

	ms_Spectatorboard[COLUMN_SPEC_CLAN].m_Active = g_Config.m_TcScoreboardInfos&TC_SCORE_CLAN;
	ms_Spectatorboard[COLUMN_SPEC_COUNTRY].m_Active = g_Config.m_TcScoreboardInfos&TC_SCORE_COUNTRY;
	ms_Spectatorboard[COLUMN_SPEC_ID].m_Active = 1;
}

bool CScoreboard::Active()
{
	// if statboard active dont show scoreboard
	if(m_pClient->m_pTeecompStats->IsActive())
		return false;

	// if we activly wanna look on the scoreboard	
	if(m_Active)
		return true;

	if(m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
	{
		// we are not a spectator, check if we are dead
		if(!m_pClient->m_Snap.m_pLocalCharacter)
			return true;
	}

	// if the game is over
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return true;

	return false;
}

const char *CScoreboard::GetClanName(int Team)
{
	int ClanPlayers = 0;
	const char *pClanName = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paInfoByScore[i];
		if(!pInfo || pInfo->m_Team != Team)
			continue;

		if(!pClanName)
		{
			pClanName = m_pClient->m_aClients[pInfo->m_ClientID].m_aClan;
			ClanPlayers++;
		}
		else
		{
			if(str_comp(m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, pClanName) == 0)
				ClanPlayers++;
			else
				return 0;
		}
	}

	if(ClanPlayers > 1 && pClanName[0])
		return pClanName;
	else
		return 0;
}

vec4 CScoreboard::GetScoreboardPosition()
{
	return m_ScoreboardPosition;
}
