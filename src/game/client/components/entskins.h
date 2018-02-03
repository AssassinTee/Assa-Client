/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_ENTSKINS_H
#define GAME_CLIENT_COMPONENTS_ENTSKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>

class CentSkins : public CComponent
{
public:
	// do this better and nicer
	struct CentSkin
	{
		int m_Texture;
		char m_aName[24];

		bool operator<(const CentSkin &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	void OnInit();

	vec3 GetColorV3(int v);
	vec4 GetColorV4(int v);
	int Num();
	const CentSkin *Get(int Index);
	int Find(const char *pName);

private:
	sorted_array<CentSkin> m_aSkins;

	static int SkinScan(const char *pName, int IsDir, int DirType, void *pUser);
};
#endif