#include "FontPatch.h"
#include <plugin.h>

class PluginSA
{
  public:
    PluginSA()
    {
        plugin::Events::initRwEvent += []() { FontPatch::Init(); };
        plugin::Events::shutdownRwEvent += []() { FontPatch::Shutdown(); };
    }

} plugin_sa;
