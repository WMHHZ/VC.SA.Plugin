#pragma once
#include "../deps/func_wrapper/func_wrapper.hpp"
#include "../deps/selector/asnew.hpp"
#include "rw/rwcore.h"

class rwFunc
{
public:
	static __int32 *RsGlobalW;
	static __int32 *RsGlobalH;

	static cdecl_func_wrapper<RwBool(RwRenderState state, void *value)>
		fpRwRenderStateSet;

	rwFunc();
};
