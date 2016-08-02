#include "CSprite2d.h"
#include "../include/hooking/Hooking.Patterns.h"

thiscall_func_wrapper<void()>
CSprite2d::fpSetRenderState;

thiscall_func_wrapper<void()>
CSprite2d::fpDelete;

cdecl_func_wrapper<void(const CRect &rect, const CRGBA &color)>
CSprite2d::fpDrawRect;

cdecl_func_wrapper<void(const CRect &rect, const CRGBA &color, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)>
CSprite2d::fpAddToBuffer;

cdecl_func_wrapper<void()>
CSprite2d::fpRenderVertexBuffer;

CSprite2d::CSprite2d(int)
{
	fpAddToBuffer = hook::pattern("8B 15 ? ? ? ? 8B 4C 24 08 8B 44 24 04").get(0).get();
	fpDelete = hook::pattern("53 89 CB 8B 03 85 C0").get(0).get();
	fpDrawRect = hook::pattern("8B 44 24 04 53 8B 5C 24 0C 53 53 53 53").get(0).get();
	fpRenderVertexBuffer = hook::pattern("83 3D ? ? ? ? 00 7E 3F 6A 02 6A 09").get(0).get();
	fpSetRenderState = hook::pattern("8B 11 85 D2 75 0A").get(0).get();
}

static CSprite2d instance(0);
