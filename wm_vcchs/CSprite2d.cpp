#include "CSprite2d.h"

injector::hook_back<void(__fastcall *)(CSprite2d *, int, const char *, const char *)>
CSprite2d::fpSetTexture;

injector::hook_back<void(__fastcall *)(const CSprite2d *, int)>
CSprite2d::fpSetRenderState;

injector::hook_back<void(__fastcall *)(CSprite2d *, int)>
CSprite2d::fpDelete;

injector::hook_back<void(__fastcall *)(const CSprite2d *, int, const CRect &rect, const CRGBA &color)>
CSprite2d::fpDraw;

injector::hook_back<void(*)(const CRect &rect, const CRGBA &color)>
CSprite2d::fpDrawRect;

injector::hook_back<void(*)(const CRect &rect, const CRGBA &color, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)>
CSprite2d::fpAddToBuffer;

injector::hook_back<void(*)()>
CSprite2d::fpRenderVertexBuffer;
