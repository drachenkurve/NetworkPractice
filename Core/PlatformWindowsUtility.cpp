#include "PlatformWindowsUtility.h"

i32 FPlatformWindowsUtility::GetLogicalCoreCount()
{
	SYSTEM_INFO Info;
	GetSystemInfo(&Info);

	return Info.dwNumberOfProcessors;
}

i32 FPlatformWindowsUtility::GetPhysicalCoreCount()
{
	DWORD Length = 0;

	if (GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &Length) == FALSE)
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			return -1;
		}
	}

	std::unique_ptr<u8[]> Buffer = std::make_unique<u8[]>(Length);

	if (GetLogicalProcessorInformationEx(RelationProcessorCore, reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(Buffer.get()), &Length) == FALSE)
	{
		return -1;
	}

	i32 PhysicalCoreCount = 0;

	u8* BufferPtr = Buffer.get();
	u8* BufferEnd = BufferPtr + Length;

	while (BufferPtr < BufferEnd)
	{
		SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* Information = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(BufferPtr);

		if (Information->Relationship == RelationProcessorCore)
		{
			++PhysicalCoreCount;
		}

		BufferPtr += Information->Size;
	}

	return PhysicalCoreCount;
}
