#include "CSprite2d.h"
#include "rwFunc.h"
#include "../include/hooking/Hooking.Patterns.h"

void *CSprite2d::fpAddSpriteToBank;
void *CSprite2d::fpDelete;
void *CSprite2d::fpDrawRect;

void CSprite2d::Delete()
{
	((void(__thiscall *)(CSprite2d *))(fpDelete))(this);
}

void CSprite2d::SetRwTexture(RwTexture *texture)
{
	this->Delete();
	m_pRwTexture = texture;
}

RwTexture *CSprite2d::GetRwTexture()
{
	return m_pRwTexture;
}

void CSprite2d::DrawRect(const CRect &rect, const CRGBA &color)
{
	((void(__cdecl *)(const CRect &, const CRGBA &))(fpDrawRect))(rect, color);
}

void CSprite2d::SetVertices(RwD3D8Vertex *pVertices, const CRect &rect, const CRGBA &color1, const CRGBA &color2, const CRGBA &color3, const CRGBA &color4, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)
{
	((void(__cdecl *)(RwD3D8Vertex *pVertices, const CRect &rect, const CRGBA &color1, const CRGBA &color2, const CRGBA &color3, const CRGBA &color4, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4))(fpSetVertices))(pVertices, rect, color1, color2, color3, color4, u1, v1, u2, v2, u3, v3, u4, v4);
}

void CSprite2d::GetAddresses()
{
	fpDelete = hook::pattern("53 89 CB 8B 03 85 C0 74 0D 50 E8 ? ? ? ? C7 03 00 00 00 00 59 5B C3").get(0).get();
	fpDrawRect = hook::pattern("8B 44 24 04 53 8B 5C 24 0C 6A 00").get(0).get();
	fpSetVertices = hook::pattern("53 56 57 55 83 EC 08 8B 74 24 1C 8B 7C 24 2C").get(0).get();
}
