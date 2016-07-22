#pragma once

class CTxdStore
{
public:
	static int AddTxdSlot(const char *name);
	static void LoadTxd(int slot, const char *filename);
	static void AddRef(int slot);
	static void PushCurrentTxd();
	static void PopCurrentTxd();
	static void SetCurrentTxd();
	static int FindTxdSlot(const char *name);
	static void RemoveTxdSlot(int slot);

	static void GetAddresses();

private:
	static void *fpAddTxdSlot;
	static void *fpLoadTxd;
	static void *fpAddRef;
	static void *fpPushCurrentTxd;
	static void *fpPopCurrentTxd;
	static void *fpSetCurrentTxd;
	static void *fpFindTxdSlot;
	static void *fpRemoveTxdSlot;
};
