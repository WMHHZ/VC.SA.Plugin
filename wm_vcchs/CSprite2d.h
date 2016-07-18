#pragma once

struct RwTexture;

class CRect;
class CRGBA;

class CSprite2d
{
public:
	RwTexture *m_pRwTexture;

public:
	CSprite2d()
	{
		m_pRwTexture = nullptr;
	}

	static void (__cdecl *AddToBuffer)(const CRect &rect, const CRGBA &color, float a3, float a4, float a5, float a6, float a7, float a8, float a9, float a10);

	void Delete();
	static void *fpDelete;

	static void (__cdecl *DrawRect)(const CRect &rect, const CRGBA &color);
	static void (__cdecl *RenderVertexBuffer)();

	void SetRenderState();
	static void *fpSetRenderState;

	static void GetAddresses();

private:
};
