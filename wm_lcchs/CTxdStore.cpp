#include "CTxdStore.h"
#include "../include/hooking/Hooking.Patterns.h"

void *CTxdStore::fpPopCurrentTxd;
void *CTxdStore::fpRemoveTxdSlot;

void CTxdStore::PopCurrentTxd()
{
	((void(__cdecl *)())(fpPopCurrentTxd))();
}

void CTxdStore::RemoveTxdSlot(int slot)
{
	((void(__cdecl *)(int))(fpRemoveTxdSlot))(slot);
}

void CTxdStore::GetAddresses()
{
	fpPopCurrentTxd = hook::pattern("A1 ? ? ? ? 50 E8 ? ? ? ? C7 05 ? ? ? ? 00 00 00 00 59 C3").get(0).get();
	fpRemoveTxdSlot = hook::pattern("89 D8 8D 0C C0 8D 0C 49 01 C1 03 0E").get(0).get(-0x20);
}
