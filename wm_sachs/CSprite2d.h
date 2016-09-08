#pragma once
#include "../deps/func_wrapper/func_wrapper.hpp"

struct RwTexture;

class CRGBA;
class CRect;

class CSprite2d
{

public:
	RwTexture *m_pRwTexture;

public:
	CSprite2d()
	{
		m_pRwTexture = nullptr;
	}

	static void(__cdecl *AddToBuffer)(CRect  const&, CRGBA  const&, float, float, float, float, float, float, float, float);

	void Delete();
	static void *fpDelete;

	void Draw(CRect const&, CRGBA const&);
	static void *fpDraw;

	static void(__cdecl *DrawRect)(CRect  const&, CRGBA  const&);
	static void(__cdecl *RenderVertexBuffer)();

	void SetRenderState();
	static void *fpSetRenderState;

	static void Init10U();
	static void InitSteam();
};
