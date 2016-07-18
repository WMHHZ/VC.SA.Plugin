#include "CSprite.h"
#include "../include/injector/injector.hpp"

void(__cdecl *CSprite::FlushSpriteBuffer)();

void CSprite::Init10U()
{
	FlushSpriteBuffer = injector::raw_ptr(0x70CF20).get();
}

void CSprite::InitSteam()
{
	FlushSpriteBuffer = injector::aslr_ptr(0x753AE0).get();
}
