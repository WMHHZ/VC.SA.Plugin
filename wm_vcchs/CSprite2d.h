#pragma once
#include "../include/func_wrapper/func_wrapper.hpp"

struct RwTexture;

class CRect;
class CRGBA;

class CSprite2d
{
public:
	RwTexture *m_pRwTexture;

	CSprite2d()
	{
		m_pRwTexture = nullptr;
	}

public:

	static thiscall_func_wrapper<void()>
		fpSetRenderState;

	static thiscall_func_wrapper<void()>
		fpDelete;

	static cdecl_func_wrapper<void(const CRect &rect, const CRGBA &color)>
		fpDrawRect;

	static cdecl_func_wrapper<void(const CRect &rect, const CRGBA &color, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)>
		fpAddToBuffer;

	static cdecl_func_wrapper<void()>
		fpRenderVertexBuffer;

	CSprite2d(int);
};
