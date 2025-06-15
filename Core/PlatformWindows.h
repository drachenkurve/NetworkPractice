#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// Windows
#include "Windows.h"

// WinSock2
#include "WinSock2.h"

// WS2tcpip
#include "WS2tcpip.h"

// MSWSock
#include "MSWSock.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// I'm lazy to type int32_t. thanks to Rust for naming...
struct FWindowsPlatformTypes
{
	using i8 = signed char;
	using i16 = signed short;
	using i32 = signed int;
	using i64 = signed long long;

	using u8 = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned int;
	using u64 = unsigned long long;

#if _WIN64
	using ssize = signed long long;
	using usize = unsigned long long;
#else
	using ssize = signed int;
	using usize = unsigned int;
#endif

	// TODO: what about character types?
};

using FPlatformTypes = FWindowsPlatformTypes;
