#include "rwFunc.h"

int *rwFunc::RsGlobalW;
int *rwFunc::RsGlobalH;

injector::hook_back<RwBool(*)(RwRenderState state, void *value)>
rwFunc::fpRwRenderStateSet;
