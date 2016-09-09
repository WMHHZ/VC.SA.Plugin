#include "CSprite2d.h"
#include "../deps/selector/AddressSelector.h"

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

bool CSprite2d::Valid() const
{
	return (this->m_pRwTexture != nullptr);
}

void CSprite2d::SetRwTexture(RwTexture *texture)
{
	if (m_pRwTexture)
	{
		fpDelete(this);
	}

	m_pRwTexture = texture;
}

CSprite2d::CSprite2d(int)
{
	fpAddToBuffer = AddressSelectorVC::SelectAddress<0x578830, 0x0, 0x578720>();
	fpDelete = AddressSelectorVC::SelectAddress<0x578A20, 0x0, 0x578910>();
	fpDraw = AddressSelectorVC::SelectAddress<0x578710, 0x0, 0x0>();
	fpDrawRect = AddressSelectorVC::SelectAddress<0x577B00, 0x0, 0x5779F0>();
	fpRenderVertexBuffer = AddressSelectorVC::SelectAddress<0x5787E0, 0x0, 0x5786D0>();
	fpSetRenderState = AddressSelectorVC::SelectAddress<0x577B90, 0x0, 0x577A80>();
	fpSetTexture = AddressSelectorVC::SelectAddress<0x5789B0, 0x0, 0x5788A0>();
}

static CSprite2d instance(0);
