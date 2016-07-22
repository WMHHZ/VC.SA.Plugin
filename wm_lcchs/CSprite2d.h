#pragma once
#include "game.h"

struct RwTexture;
struct RwD3D8Vertex;

class CSprite2d
{
public:
	CSprite2d()
	{
		m_pRwTexture = nullptr;
	}

	void Delete();
	void SetTexture(const char *texturename, const char *maskname);
	RwTexture *GetRwTexture();

	static void DrawRect(const CRect &rect, const CRGBA &color);
	static void SetVertices(RwD3D8Vertex *pVertices, const CRect &rect, const CRGBA &color1, const CRGBA &color2, const CRGBA &color3, const CRGBA &color4, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4);	
	
	static void GetAddresses();

private:
	RwTexture *m_pRwTexture;

	static void *fpSetTexture;
	static void *fpDelete;
	static void *fpDrawRect;
	static void *fpSetVertices;
};
