/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <engine/shared/config.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include "scoredif.h"
#include "menus.h"

void CAssaMessages::OnReset()
{
	m_KillmsgAssa = 0;
	m_OldScore = 0;
	for(int i = 0; i < MAX_KILLMSGS; i++)
	{
		m_aKillmsgs[i].m_AssaTick = -50*3;
		m_ScoreDif[i] = 0;
	}
}

void CAssaMessages::OnSpectator()
{
    if(m_aKillmsgs[0].m_AssaTick != -50*3)
    {
        for(int i = 0; i < MAX_KILLMSGS; i++)
        {
            m_aKillmsgs[i].m_AssaTick = -50*3;
        }
    }
}

void CAssaMessages::OnClientInfo()
{
    if(g_Config.m_BrSpecialKillAssa && m_pClient->m_Snap.m_pLocalInfo)
    {
        if(m_pClient->m_Snap.m_pLocalInfo->m_Score - m_OldScore)
        {
            // unpack messages
            CKillMsg Kill;
            Kill.m_KillerID = m_pClient->m_Snap.m_pLocalInfo->m_ClientID;
            Kill.m_AssaTick = Client()->GameTick();

            // add the message
            m_KillmsgAssa = (m_KillmsgAssa+1)%MAX_KILLMSGS;
            m_aKillmsgs[m_KillmsgAssa] = Kill;

            m_CurrentScoreDif = m_pClient->m_Snap.m_pLocalInfo->m_Score - m_OldScore;
        }
    }
}

void CAssaMessages::OnRender()
{
	if (!m_pClient->m_Snap.m_pLocalInfo || m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_SPECTATORS)
		return;

	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width*1.5f, Height*1.5f);
	int r1, g1, b1;
    CMenus::SetColorVar(&r1, &g1, &b1, &g_Config.m_BrSpecialKillAssaColor);

	float y = 0;
	float FontSize = 36.0f;
	for(int i = 0; i < MAX_KILLMSGS; i++)
	{
		//float Alpha = 1.0f;
		int f = (m_KillmsgAssa+i)%MAX_KILLMSGS;

		if(Client()->GameTick() > m_aKillmsgs[f].m_AssaTick+50*3 || m_pClient->m_Snap.m_pLocalInfo->m_ClientID != m_aKillmsgs[f].m_KillerID)
			continue;

		if(m_CurrentScoreDif != 0 && !i)
		{
			m_ScoreDif[f] = m_CurrentScoreDif;
			m_OldScore = m_pClient->m_Snap.m_pLocalInfo->m_Score;
			m_CurrentScoreDif = 0;
		}
        if(!m_ScoreDif[f])
			continue;

		char bBuf[128];
		if(m_ScoreDif[f] > 0)
			str_format(bBuf, sizeof(bBuf), "+%d", m_ScoreDif[f]);
		else
			str_format(bBuf, sizeof(bBuf), "%d", m_ScoreDif[f]);

        TextRender()->TextColor(r1 / 255.0f, g1 / 255.0f, b1 / 255.0f, mix(1.0f, 0.0f,  (Client()->GameTick() - m_aKillmsgs[f].m_AssaTick) / 50.0f / 3.0f));
		TextRender()->Text(0, Width*1.5f/2 - TextRender()->TextWidth(0, FontSize, bBuf, -1)/2, Height*1.5f/2 - FontSize/2 - 110.0f - y, FontSize, bBuf, -1);
		y += 46.0f;
	}
	TextRender()->TextColor(1,1,1,1);
}
