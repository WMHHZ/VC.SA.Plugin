#pragma once
#include "stdinc.h"
#include "game.h"

class CSprite2d
{
private:
    RwTexture *m_pRwTexture;

public:
    static injector::hook_back<void(__fastcall *)(CSprite2d *, int, const char *, const char *)>
        fpSetTexture;

    static injector::hook_back<void(__fastcall *)(const CSprite2d *, int)>
        fpSetRenderState;

    static injector::hook_back<void(__fastcall *)(CSprite2d *, int)>
        fpDelete;

    static injector::hook_back<void(__fastcall *)(const CSprite2d *, int, const CRect &rect, const CRGBA &color)>
        fpDraw;

    static injector::hook_back<void(*)(const CRect &rect, const CRGBA &color)>
        fpDrawRect;

    static injector::hook_back<void(*)(const CRect &rect, const CRGBA &color, float u1, float v1, float u2, float v2, float u3, float v3, float u4, float v4)>
        fpAddToBuffer;

    static injector::hook_back<void(*)()>
        fpRenderVertexBuffer;
};
VALIDATE_SIZE(CSprite2d, 4)
