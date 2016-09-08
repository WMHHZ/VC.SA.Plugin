#include "CSprite2d.h"
#include "../deps/injector/injector.hpp"

void(__cdecl *CSprite2d::AddToBuffer)(CRect  const& posn, CRGBA  const& color, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4);
void *CSprite2d::fpDelete;
void *CSprite2d::fpDraw;
void(__cdecl *CSprite2d::DrawRect)(CRect  const& posn, CRGBA  const& color);
void(__cdecl *CSprite2d::RenderVertexBuffer)();
void *CSprite2d::fpSetRenderState;

void CSprite2d::Delete()
{
	((void(__thiscall *)(CSprite2d *))fpDelete)(this);
}

void CSprite2d::Draw(CRect const&pos, CRGBA const&color)
{
	((void(__thiscall *)(CSprite2d*, CRect const&, CRGBA const&))fpDraw)(this, pos, color);
}

void CSprite2d::SetRenderState()
{
	((void(__thiscall *)(CSprite2d *))fpSetRenderState)(this);
}

void CSprite2d::Init10U()
{
	AddToBuffer = injector::raw_ptr(0x728200).get();
	fpDelete = injector::raw_ptr(0x727240).get();
	fpDraw = injector::raw_ptr(0x728350).get();
	DrawRect = injector::raw_ptr(0x727B60).get();
	RenderVertexBuffer = injector::raw_ptr(0x7273D0).get();
	fpSetRenderState = injector::raw_ptr(0x727B30).get();
}

void CSprite2d::InitSteam()
{
	AddToBuffer = injector::aslr_ptr(0x757D90).get();
	fpDelete = injector::aslr_ptr(0x756980).get();
	fpDraw = injector::aslr_ptr(0x757EE0).get();
	DrawRect = injector::aslr_ptr(0x757690).get();
	RenderVertexBuffer = injector::aslr_ptr(0x756B20).get();
	fpSetRenderState = injector::aslr_ptr(0x757660).get();
}
