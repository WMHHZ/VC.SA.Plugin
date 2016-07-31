#pragma once
#include "../include/func_wrapper/func_wrapper.hpp"

struct RwTexture;
struct RwD3D8Vertex;

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

	static cdecl_func_wrapper<void(RwD3D8Vertex *vertices, const CRect &rect, const CRGBA &color1, const CRGBA &color2, const CRGBA &color3, const CRGBA &color4, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)>
		fpSetVertices;

	static cdecl_func_wrapper<void()>
		fpRenderVertexBuffer;

	CSprite2d(int);
};
