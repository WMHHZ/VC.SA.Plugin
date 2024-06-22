#include "FontPatch.h"
#include <plugin.h>

class PluginVC
{
  public:
    PluginVC()
    {
        plugin::Events::initRwEvent += []() { FontPatch::Init(); };
        plugin::Events::shutdownRwEvent += []() { FontPatch::Shutdown(); };
    }

} plugin_vc;
