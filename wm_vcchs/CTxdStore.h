#pragma once
#include "../include/func_wrapper/func_wrapper.hpp"

class CTxdStore
{
public:
	static cdecl_func_wrapper<void()>
		fpPopCurrentTxd;

	static cdecl_func_wrapper<void(int slot)>
		fpRemoveTxdSlot;

	CTxdStore();
};
