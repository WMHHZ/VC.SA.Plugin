#include "AddressSelector.h"

AddressSelectorBase::GameVersion AddressSelectorLC::ms_gv;
AddressSelectorBase::GameVersion AddressSelectorVC::ms_gv;
AddressSelectorBase::GameVersion AddressSelectorSAUS::ms_gv;

static AddressSelectorLC lcinstance;
static AddressSelectorVC vcinstance;
static AddressSelectorSAUS sausinstance;
