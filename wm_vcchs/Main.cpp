#include "FontPatch.h"
#include <plugin.h>

class PluginVC
{
  public:
    PluginVC()
    {
        plugin::Events::initGameEvent += []() { FontPatch::Init(); };
    }

} plugin_vc;
