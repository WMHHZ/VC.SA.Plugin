#include "CTxdStore.h"
#include "../include/selector/AddressSelector.h"

void *CTxdStore::fpAddTxdSlot;
void *CTxdStore::fpLoadTxd;
void *CTxdStore::fpAddRef;
void *CTxdStore::fpPushCurrentTxd;
void *CTxdStore::fpPopCurrentTxd;
void *CTxdStore::fpSetCurrentTxd;
void *CTxdStore::fpFindTxdSlot;
void *CTxdStore::fpRemoveTxdSlot;

int CTxdStore::AddTxdSlot(const char *name)
{
	return ((int(__cdecl *)(const char *))(fpAddTxdSlot))(name);
}

void CTxdStore::LoadTxd(int slot, const char *filename)
{
	((void(__cdecl *)(int, const char *))(fpLoadTxd))(slot, filename);
}

void CTxdStore::AddRef(int slot)
{
	((void(__cdecl *)(int))(fpAddRef))(slot);
}

void CTxdStore::PushCurrentTxd()
{
	((void(__cdecl *)())(fpPushCurrentTxd))();
}

void CTxdStore::PopCurrentTxd()
{
	((void(__cdecl *)())(fpPopCurrentTxd))();
}

void CTxdStore::SetCurrentTxd(int slot)
{
	((void(__cdecl *)(int))(fpSetCurrentTxd))(slot);
}

int CTxdStore::FindTxdSlot(const char *name)
{
	return ((int(__cdecl *)(const char *))(fpFindTxdSlot))(name);
}

void CTxdStore::RemoveTxdSlot(int slot)
{
	((void(__cdecl *)(int))(fpRemoveTxdSlot))(slot);
}

CTxdStore::CTxdStore()
{
	fpAddTxdSlot = AddressSelectorLC::SelectAddress<0x5274E0, 0x5276B0>();
	fpLoadTxd = AddressSelectorLC::SelectAddress<0x5276B0, 0x527880>();
	fpAddRef = AddressSelectorLC::SelectAddress<0x527930, 0x527B00>();
	fpPushCurrentTxd = AddressSelectorLC::SelectAddress<0x527900, 0x527AD0>();
	fpSetCurrentTxd = AddressSelectorLC::SelectAddress<0x5278C0, 0x527A90>();
	fpPopCurrentTxd = AddressSelectorLC::SelectAddress<0x527910, 0x527AE0>();
	fpFindTxdSlot = AddressSelectorLC::SelectAddress<0x5275D0, 0x5277A0>();
	fpRemoveTxdSlot = AddressSelectorLC::SelectAddress<0x527520, 0x5276F0>();
}

static CTxdStore instance;
