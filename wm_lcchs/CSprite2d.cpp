#include "CSprite2d.h"
#include "rwFunc.h"
#include "../deps/selector/AddressSelector.hpp"

void *CSprite2d::fpSetTexture;
void *CSprite2d::fpDelete;
void *CSprite2d::fpSetVertices;

void CSprite2d::Delete()
{
	((void(__thiscall *)(CSprite2d *))(fpDelete))(this);
}

void CSprite2d::SetTexture(const char *texturename, const char *maskname)
{
	((void(__thiscall *)(CSprite2d *, const char *, const char *))(fpSetTexture))(this, texturename, maskname);
}

RwTexture *CSprite2d::GetRwTexture()
{
	return m_pRwTexture;
}

void CSprite2d::SetVertices(RwD3D8Vertex *pVertices, const CRect &rect, const CRGBA &color1, const CRGBA &color2, const CRGBA &color3, const CRGBA &color4, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)
{
	((void(__cdecl *)(RwD3D8Vertex *, const CRect &, const CRGBA &, const CRGBA &, const CRGBA &, const CRGBA &, float, float, float, float, float, float, float, float))(fpSetVertices))(pVertices, rect, color1, color2, color3, color4, u1, v1, u2, v2, u3, v3, u4, v4);
}

CSprite2d::CSprite2d(int)
{
	fpDelete = AddressSelectorLC::SelectAddress<0x51EA00, 0x0, 0x51EBC0>();
	fpSetVertices = AddressSelectorLC::SelectAddress<0x51F720, 0x0, 0x51F8E0>();
	fpSetTexture = AddressSelectorLC::SelectAddress<0x51EA70, 0x0, 0x51EC30>();
}

static CSprite2d instance(0);
