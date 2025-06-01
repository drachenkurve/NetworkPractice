#pragma once

#include <algorithm>
#include <cassert>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <string_view>

#include "PlatformWindows.h"

using i8 = FPlatformTypes::i8;
using i16 = FPlatformTypes::i16;
using i32 = FPlatformTypes::i32;
using i64 = FPlatformTypes::i64;

using u8 = FPlatformTypes::u8;
using u16 = FPlatformTypes::u16;
using u32 = FPlatformTypes::u32;
using u64 = FPlatformTypes::u64;

using ssize = FPlatformTypes::ssize;
using usize = FPlatformTypes::usize;
