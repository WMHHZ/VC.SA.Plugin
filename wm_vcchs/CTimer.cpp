#include "CTimer.h"
#include "../deps/selector/asnew.hpp"

unsigned __int32 *CTimer::m_nTimeInMilliseconds;

CTimer::CTimer()
{
	m_nTimeInMilliseconds = addr_sel::vc::select_address<unsigned __int32>({ 0x974B2C, 0x0, 0x973B34 });
}

static CTimer instance;
