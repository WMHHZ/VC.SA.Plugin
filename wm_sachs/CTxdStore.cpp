#include "CTxdStore.h"
#include "../include/injector/injector.hpp"

void(__cdecl *CTxdStore::PopCurrentTxd)();
void(__cdecl *CTxdStore::RemoveTxdSlot)(int slot);

void CTxdStore::Init10U()
{
	PopCurrentTxd = injector::raw_ptr(0x7316B0).get();
	RemoveTxdSlot = injector::raw_ptr(0x731CD0).get();
}

void CTxdStore::InitSteam()
{
	PopCurrentTxd = injector::aslr_ptr(0x765CE0).get();
	RemoveTxdSlot = injector::aslr_ptr(0x766310).get();
}
