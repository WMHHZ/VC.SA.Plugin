#include "FontPatch.h"
#include <plugin.h>

class PluginSA
{
  public:
    PluginSA()
    {
        plugin::Events::initGameEvent += []() { FontPatch::Init(); };
    }

} plugin_sa;
