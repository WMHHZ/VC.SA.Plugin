#pragma once
#include "../include/func_wrapper/func_wrapper.hpp"

class CTxdStore
{
public:
	static cdecl_func_wrapper<int(const char *name)>
		fpAddTxdSlot;

	static cdecl_func_wrapper<void(int slot, const char *filename)>
		fpLoadTxd;

	static cdecl_func_wrapper<void(int slot)>
		fpAddRef;

	static cdecl_func_wrapper<void()>
		fpPushCurrentTxd;

	static cdecl_func_wrapper<void()>
		fpPopCurrentTxd;

	static cdecl_func_wrapper<void(int slot)>
		fpSetCurrentTxd;

	static cdecl_func_wrapper<int(const char *name)>
		fpFindTxdSlot;

	static cdecl_func_wrapper<void(int slot)>
		fpRemoveTxdSlot;

	CTxdStore();
};
