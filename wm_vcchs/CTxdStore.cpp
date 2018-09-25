#include "CTxdStore.h"

injector::hook_back<int(*)(const char *name)>
CTxdStore::fpAddTxdSlot;

injector::hook_back<void(*)(int slot, const char *filename)>
CTxdStore::fpLoadTxd;

injector::hook_back<void(*)(int slot)>
CTxdStore::fpAddRef;

injector::hook_back<void(*)()>
CTxdStore::fpPushCurrentTxd;

injector::hook_back<void(*)()>
CTxdStore::fpPopCurrentTxd;

injector::hook_back<void(*)(int slot)>
CTxdStore::fpSetCurrentTxd;

injector::hook_back<int(*)(const char *name)>
CTxdStore::fpFindTxdSlot;

injector::hook_back<void(*)(int slot)>
CTxdStore::fpRemoveTxdSlot;
