/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include "entskins.h"

int CentSkins::SkinScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CentSkins *pSelf = (CentSkins *)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".png") != 0)
		return 0;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "entities/%s", pName);
	CImageInfo Info;
	if(!pSelf->Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load entities from %s", pName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return 0;
	}

	CentSkin Skin;
	Skin.m_Texture = pSelf->Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);

	// set skin data
	str_copy(Skin.m_aName, pName, min((int)sizeof(Skin.m_aName),l-3));
	if(g_Config.m_Debug)
	{
		str_format(aBuf, sizeof(aBuf), "load entities %s", Skin.m_aName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}
	pSelf->m_aSkins.add(Skin);

	return 0;
}


void CentSkins::OnInit()
{
	// load skins
	m_aSkins.clear();
	
	// load Default 
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "entities.png");
	CImageInfo Info;
	if(!Graphics()->LoadPNG(&Info, aBuf,IStorage::TYPE_ALL))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load default entities");
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}
	
	CentSkin DefaultSkin;
	DefaultSkin.m_Texture = Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	
	// set Default skin data
	str_format(DefaultSkin.m_aName, sizeof(DefaultSkin.m_aName), "!default");
	if(g_Config.m_Debug)
	{
		str_format(aBuf, sizeof(aBuf), "load default entities", DefaultSkin.m_aName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}
	m_aSkins.add(DefaultSkin);
	
	Storage()->ListDirectory(IStorage::TYPE_ALL, "entities", SkinScan, this);
		
	if(!m_aSkins.size())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load textures. folder='entities/'");
		CentSkin DummySkin;
		DummySkin.m_Texture = -1;
		str_copy(DummySkin.m_aName, "dummy", sizeof(DummySkin.m_aName));
		m_aSkins.add(DummySkin);
	}
}

int CentSkins::Num()
{
	return m_aSkins.size();
}

const CentSkins::CentSkin *CentSkins::Get(int Index)
{
	return &m_aSkins[max(0, Index%m_aSkins.size())];
}

int CentSkins::Find(const char *pName)
{
	for(int i = 0; i < m_aSkins.size(); i++)
	{
		if(str_comp(m_aSkins[i].m_aName, pName) == 0)
			return i;
	}
	return -1;
}

vec3 CentSkins::GetColorV3(int v)
{
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
}

vec4 CentSkins::GetColorV4(int v)
{
	vec3 r = GetColorV3(v);
	return vec4(r.r, r.g, r.b, 1.0f);
}
