
#include <plugin.h>

class CPlugin
{
  public:
    CPlugin()
    {
        plugin::Events::initGameEvent += []() {};
    }
} plugin;
