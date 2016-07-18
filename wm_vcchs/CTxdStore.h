#pragma once

class CTxdStore
{
public:
	static void(__cdecl *PopCurrentTxd)();
	static void(__cdecl *RemoveTxdSlot)(int slot);

	static void GetAddresses();
};
