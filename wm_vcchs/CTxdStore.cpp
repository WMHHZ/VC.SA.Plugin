#include "CTxdStore.h"
#include "../include/hooking/Hooking.Patterns.h"

cdecl_func_wrapper<void()>
CTxdStore::fpPopCurrentTxd;

cdecl_func_wrapper<void(int slot)>
CTxdStore::fpRemoveTxdSlot;

CTxdStore::CTxdStore()
{
	fpRemoveTxdSlot = hook::pattern("89 D8 8D 0C C0 8D 0C 49 01 C1 03 0E").get(0).get(-0x20);
	fpPopCurrentTxd = hook::pattern("A1 ? ? ? ? 50 E8 ? ? ? ? C7 05 ? ? ? ? 00 00 00 00 59 C3").get(1).get();
}

static CTxdStore instance;
