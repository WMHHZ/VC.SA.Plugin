#pragma once
#include "../deps/func_wrapper/func_wrapper.hpp"

class CTxdStore
{
public:
	static void(__cdecl *PopCurrentTxd)();
	static void(__cdecl *RemoveTxdSlot)(int slot);

	static void Init10U();
	static void InitSteam();
};
