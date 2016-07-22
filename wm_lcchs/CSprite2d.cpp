#include "CSprite2d.h"
#include "rwFunc.h"
#include "../include/selector/AddressSelector.h"

void *CSprite2d::fpSetTexture;
void *CSprite2d::fpDelete;
void *CSprite2d::fpDrawRect;
void *CSprite2d::fpSetVertices;

void CSprite2d::Delete()
{
	((void(__thiscall *)(CSprite2d *))(fpDelete))(this);
}

void CSprite2d::SetTexture(const char *texturename, const char *maskname)
{
	((void(__cdecl *)(const char *, const char *))(fpSetTexture))(texturename, maskname);
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
	((void(__cdecl *)(RwD3D8Vertex *, const CRect &, const CRGBA &, const CRGBA &, const CRGBA &, const CRGBA &, float, float, float, float, float, float, float, float))(fpSetVertices))(pVertices, rect, color1, color2, color3, color4, u1, v1, u2, v2, u3, v3, u4, v4);
}

void CSprite2d::GetAddresses()
{
	AddressSelectorLC selector;

	fpDelete = selector.SelectAddress<0x51EA00, 0x51EBC0>();
	fpDrawRect = selector.SelectAddress<0x51F970, 0x51FB30>();
	fpSetVertices = selector.SelectAddress<0x51F720, 0x51F8E0>();
	fpSetTexture = selector.SelectAddress<0x51EA70, 0x51EC30>();
}
