#include "CTxdStore.h"
#include "../include/hooking/Hooking.Patterns.h"
#include "../include/injector/injector.hpp"

void(__cdecl *CTxdStore::PopCurrentTxd)();
void(__cdecl *CTxdStore::RemoveTxdSlot)(int slot);

void CTxdStore::GetAddresses()
{
	RemoveTxdSlot = injector::raw_ptr(hook::pattern("89 D8 8D 0C C0 8D 0C 49 01 C1 03 0E").get(0).get(-0x20)).get();
	PopCurrentTxd = injector::raw_ptr(hook::pattern("A1 ? ? ? ? 50 E8 ? ? ? ? C7 05 ? ? ? ? 00 00 00 00 59 C3").get(1).get()).get(); //Unreliable
}
