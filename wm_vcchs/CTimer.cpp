#include "CTimer.h"
#include "../deps/selector/AddressSelector.h"

unsigned __int32 *CTimer::m_nTimeInMilliseconds;

CTimer::CTimer()
{
	m_nTimeInMilliseconds = AddressSelectorVC::SelectAddress<0x974B2C, 0x0, 0x973B34, unsigned __int32>();
}

static CTimer instance;
