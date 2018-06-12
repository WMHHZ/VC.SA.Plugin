#include "rwFunc.h"

__int32 *rwFunc::RsGlobalW;
__int32 *rwFunc::RsGlobalH;

cdecl_func_wrapper<RwBool(RwRenderState state, void *value)>
rwFunc::fpRwRenderStateSet;

rwFunc::rwFunc()
{
	RsGlobalW = addr_sel::vc::select_address<__int32>({0x9B48E4, 0x0, 0x9B38EC});
	RsGlobalH = RsGlobalW + 1;

	fpRwRenderStateSet = addr_sel::vc::select_address({0x649BA0, 0x0, 0x648B50});
}

static rwFunc instance;
