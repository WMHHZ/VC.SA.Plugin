#pragma once
#include "stdinc.h"

class rwFunc
{
public:
	static int *RsGlobalW;
	static int *RsGlobalH;

	static injector::hook_back<RwBool(*)(RwRenderState state, void *value)>
		fpRwRenderStateSet;
};
