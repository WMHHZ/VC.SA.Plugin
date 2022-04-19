#pragma once
#include "rw/rwcore.h"
//#include "../deps/injector/assembly.hpp"
#include "../deps/injector/injector.hpp"
#include "../deps/injector/hooking.hpp"
#include "../deps/injector/calling.hpp"
#include "../deps/injector/utility.hpp"
#include <utf8cpp/utf8.h>
#include <filesystem>
#include <array>
#include <cstdio>
#include <windows.h>

#define VALIDATE_SIZE(type,size) static_assert(sizeof(type)==size, "Type size error.");
