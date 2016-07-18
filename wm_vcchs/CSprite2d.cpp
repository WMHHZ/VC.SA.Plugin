#include "CSprite2d.h"
#include "../include/hooking/Hooking.Patterns.h"
#include "../include/injector/injector.hpp"
#include "../include/injector/calling.hpp"

void(__cdecl *CSprite2d::AddToBuffer)(const CRect &rect, const CRGBA &color, float a3, float a4, float a5, float a6, float a7, float a8, float a9, float a10);
void *CSprite2d::fpDelete;
void(__cdecl *CSprite2d::DrawRect)(const CRect &rect, const CRGBA &color);
void(__cdecl *CSprite2d::RenderVertexBuffer)();
void *CSprite2d::fpSetRenderState;

void CSprite2d::Delete()
{
	injector::thiscall<void(CSprite2d *)>::call(fpDelete, this);
}

void CSprite2d::SetRenderState()
{
	injector::thiscall<void(CSprite2d *)>::call(fpSetRenderState, this);
}

void CSprite2d::GetAddresses()
{
	AddToBuffer = injector::raw_ptr(hook::pattern("8B 15 ? ? ? ? 8B 4C 24 08 8B 44 24 04").get(0).get()).get();
	fpDelete = injector::raw_ptr(hook::pattern("53 89 CB 8B 03 85 C0").get(0).get()).get();
	DrawRect = injector::raw_ptr(hook::pattern("8B 44 24 04 53 8B 5C 24 0C 53 53 53 53").get(0).get()).get();
	RenderVertexBuffer = injector::raw_ptr(hook::pattern("83 3D ? ? ? ? 00 7E 3F 6A 02 6A 09").get(0).get()).get();
	fpSetRenderState = injector::raw_ptr(hook::pattern("8B 11 85 D2 75 0A").get(0).get()).get();
}
