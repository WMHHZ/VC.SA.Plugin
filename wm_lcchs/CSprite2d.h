#pragma once
#include "game.h"

struct RwTexture;

class CSprite2d
{
public:
	CSprite2d()
	{
		m_pRwTexture = nullptr;
	}

	void Delete();
	void SetRwTexture(RwTexture *texture);
	RwTexture *GetRwTexture();

	static void __cdecl DrawRect(const CRect &rect, const CRGBA &color);
	static void __cdecl SetVertices(RwD3D8Vertex *pVertices, const CRect &rect, const CRGBA &color1, const CRGBA &color2, const CRGBA &color3, const CRGBA &color4, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4);
	
	static void GetAddresses();

private:
	RwTexture *m_pRwTexture;

	static void *fpDelete;
	static void *fpDrawRect;
	static void *fpSetVertices;
};
