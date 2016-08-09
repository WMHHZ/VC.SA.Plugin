#pragma once
#include "../include/func_wrapper/func_wrapper.hpp"

class CSprite
{
public:
	static void(__cdecl * FlushSpriteBuffer)();

	static void Init10U();
	static void InitSteam();
};
