#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <ranges>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

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
