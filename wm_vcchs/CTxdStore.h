#pragma once
#include "stdinc.h"

class CTxdStore
{
public:
    static injector::hook_back<int(*)(const char *name)>
        fpAddTxdSlot;

    static injector::hook_back<void(*)(int slot, const char *filename)>
        fpLoadTxd;

    static injector::hook_back<void(*)(int slot)>
        fpAddRef;

    static injector::hook_back<void(*)()>
        fpPushCurrentTxd;

    static injector::hook_back<void(*)()>
        fpPopCurrentTxd;

    static injector::hook_back<void(*)(int slot)>
        fpSetCurrentTxd;

    static injector::hook_back<int(*)(const char *name)>
        fpFindTxdSlot;

    static injector::hook_back<void(*)(int slot)>
        fpRemoveTxdSlot;
};
