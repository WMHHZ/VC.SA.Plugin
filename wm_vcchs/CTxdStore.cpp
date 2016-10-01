#include "CTxdStore.h"
#include "../deps/selector/asnew.hpp"

cdecl_func_wrapper<int(const char *name)>
CTxdStore::fpAddTxdSlot;

cdecl_func_wrapper<void(int slot, const char *filename)>
CTxdStore::fpLoadTxd;

cdecl_func_wrapper<void(int slot)>
CTxdStore::fpAddRef;

cdecl_func_wrapper<void()>
CTxdStore::fpPushCurrentTxd;

cdecl_func_wrapper<void()>
CTxdStore::fpPopCurrentTxd;

cdecl_func_wrapper<void(int slot)>
CTxdStore::fpSetCurrentTxd;

cdecl_func_wrapper<int(const char *name)>
CTxdStore::fpFindTxdSlot;

cdecl_func_wrapper<void(int slot)>
CTxdStore::fpRemoveTxdSlot;

CTxdStore::CTxdStore()
{
	fpAddTxdSlot = addr_sel::vc::select_address<0x580F00, 0x0, 0x580D30>();
	fpLoadTxd = addr_sel::vc::select_address<0x580CD0, 0x0, 0x580B00>();
	fpAddRef = addr_sel::vc::select_address<0x580A60, 0x0, 0x580890>();
	fpPushCurrentTxd = addr_sel::vc::select_address<0x580AC0, 0x0, 0x5808F0>();
	fpPopCurrentTxd = addr_sel::vc::select_address<0x580AA0, 0x0, 0x5808D0>();
	fpSetCurrentTxd = addr_sel::vc::select_address<0x580AD0, 0x0, 0x580900>();
	fpFindTxdSlot = addr_sel::vc::select_address<0x580D70, 0x0, 0x580BA0>();
	fpRemoveTxdSlot = addr_sel::vc::select_address<0x580E90, 0x0, 0x580CC0>();
}

static CTxdStore instance;
