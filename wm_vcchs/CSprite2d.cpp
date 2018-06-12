#include "CSprite2d.h"

thiscall_func_wrapper<void(CSprite2d *, const char *, const char *)>
CSprite2d::fpSetTexture;

thiscall_func_wrapper<void(const CSprite2d *)>
CSprite2d::fpSetRenderState;

thiscall_func_wrapper<void(CSprite2d *)>
CSprite2d::fpDelete;

thiscall_func_wrapper<void(const CSprite2d *, const CRect &rect, const CRGBA &color)>
CSprite2d::fpDraw;

cdecl_func_wrapper<void(const CRect &rect, const CRGBA &color)>
CSprite2d::fpDrawRect;

cdecl_func_wrapper<void(const CRect &rect, const CRGBA &color, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)>
CSprite2d::fpAddToBuffer;

cdecl_func_wrapper<void()>
CSprite2d::fpRenderVertexBuffer;

CSprite2d::CSprite2d() :m_pRwTexture(nullptr) {}

CSprite2d::CSprite2d(int)
{
	fpAddToBuffer = addr_sel::vc::select_address({0x578830, 0x0, 0x578720});
	fpDelete = addr_sel::vc::select_address({0x578A20, 0x0, 0x578910});
	fpDraw = addr_sel::vc::select_address({0x578710, 0x0, 0x0});
	fpDrawRect = addr_sel::vc::select_address({0x577B00, 0x0, 0x5779F0});
	fpRenderVertexBuffer = addr_sel::vc::select_address({0x5787E0, 0x0, 0x5786D0});
	fpSetRenderState = addr_sel::vc::select_address({0x577B90, 0x0, 0x577A80});
	fpSetTexture = addr_sel::vc::select_address({0x5789B0, 0x0, 0x5788A0});
}

static CSprite2d instance(0);
