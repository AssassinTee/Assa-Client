/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/components/skins.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include "nameplates.h"
#include "controls.h"

#include <game/client/teecomp.h>

void CNamePlates::RenderNameplate(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo
	)
{
	float IntraTick = Client()->IntraGameTick();

	vec2 Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), IntraTick);


	float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;
	// render name plate
	if(!pPlayerInfo->m_Local)
	{
		float a = 1;
		if(g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1-powf(distance(m_pClient->m_pControls->m_TargetPos, Position)/200.0f,16.0f), 0.0f, 1.0f);

		char bBuf[128];
		//Added Score in Name (optional ofc.) TODO Colour
		if(g_Config.m_BrSpecialScore)
			str_format(bBuf, sizeof(bBuf), "%s (%d)", m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_aName, pPlayerInfo->m_Score);
		else
			str_format(bBuf, sizeof(bBuf), "%s", m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_aName);
		
		float tw = TextRender()->TextWidth(0, FontSize, bBuf, -1);

		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.5f*a);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, a);
		vec3 NameColor = vec3(1,1,1);

		//Non-Team mod
		if(g_Config.m_BrSpecialNameplateColor && m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_UseCustomColor && !m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)//Assa Mark TODO
			NameColor = m_pClient->m_pSkins->GetColorV3(m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_ColorBody);
		else if(g_Config.m_BrSpecialNameplateColor && !m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
		{
			const CSkins::CSkin *s = m_pClient->m_pSkins->Get(m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_SkinID);
			if(s)
				NameColor = s->m_BloodColor;
		}
		if(pPlayerChar->m_Weapon == WEAPON_NINJA)
			NameColor = vec3(0,0,0);
		TextRender()->TextColor(NameColor.x, NameColor.y, NameColor.z, a);

		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
		{
			vec3 Col = CTeecompUtils::GetTeamColor(
			m_pClient->m_aClients[pPlayerInfo->m_ClientID].m_Team,
			m_pClient->m_Snap.m_pLocalInfo ? m_pClient->m_Snap.m_pLocalInfo->m_Team : TEAM_RED,
			g_Config.m_TcColoredTeesTeam1,
			g_Config.m_TcColoredTeesTeam2,
			g_Config.m_TcColoredTeesMethod);
			TextRender()->TextColor(Col.r, Col.g, Col.b, a);
		}
		TextRender()->Text(0, Position.x-tw/2.0f, Position.y-FontSize-38.0f, FontSize, bBuf, -1);

		if(g_Config.m_Debug) // render client id when in debug aswell
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf),"%d", pPlayerInfo->m_ClientID);
			TextRender()->Text(0, Position.x, Position.y-90, 28.0f, aBuf, -1);
		}

		TextRender()->TextColor(1,1,1,1);
		TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
	}
}

void CNamePlates::OnRender()
{
	if (!g_Config.m_ClNameplates)
		return;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// only render active characters
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
			continue;

		const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);

		if(pInfo)
		{
			RenderNameplate(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				(const CNetObj_PlayerInfo *)pInfo);
		}
	}
}
