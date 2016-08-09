#include "rwFunc.h"
#include "../include/selector/AddressSelector.h"

__int32 *rwFunc::RsGlobalW;
__int32 *rwFunc::RsGlobalH;

cdecl_func_wrapper<__int32(RwRenderState state, void *value)>
rwFunc::fpRwRenderStateSet;

rwFunc::rwFunc()
{
	RsGlobalW = AddressSelectorVC::SelectAddress<0x9B48E4, 0x0, 0x9B38EC, __int32>();
	RsGlobalH = RsGlobalW + 1;

	fpRwRenderStateSet = AddressSelectorVC::SelectAddress<0x649BA0, 0x0, 0x648B50>();
}

static rwFunc instance;
