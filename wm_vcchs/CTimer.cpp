#include "CTimer.h"
#include "../include/hooking/Hooking.Patterns.h"

unsigned __int32 *CTimer::m_nTimeInMilliseconds;

void CTimer::GetAddresses()
{
	m_nTimeInMilliseconds = *hook::pattern("8B 15 ? ? ? ? C7 44 24 14 00 00 00 00").get(0).get<unsigned __int32 *>(2);
}
