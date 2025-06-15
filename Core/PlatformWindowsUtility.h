#pragma once

#include "Platform.h"

class FPlatformWindowsUtility
{
public:
	static i32 GetLogicalCoreCount();
	static i32 GetPhysicalCoreCount();
};
