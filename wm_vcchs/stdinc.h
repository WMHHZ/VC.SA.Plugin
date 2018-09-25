#pragma once
#include "../deps/injector/assembly.hpp"
#include "../deps/injector/injector.hpp"
#include "../deps/injector/hooking.hpp"
#include "../deps/injector/calling.hpp"
#include "../deps/injector/utility.hpp"
#include "../deps/utf8cpp/utf8.h"
#include "rw/rwcore.h"
#include <filesystem>
#include <array>
#include <cstdio>
#include <windows.h>

#define VALIDATE_SIZE(type,size) static_assert(sizeof(type)==size, "Type size error.");
