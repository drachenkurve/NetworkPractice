#include "LaunchWindows.h"

// Project Headers
#include "ChatServer.h"
#include "PlatformWindowsUtility.h"
#include "SocketUtility.h"

HANDLE StopEvent = INVALID_HANDLE_VALUE;

BOOL WINAPI HandleConsoleCtrl(DWORD Signal)
{
	if (Signal == CTRL_C_EVENT)
	{
		std::cout << "Ctrl+C 감지\n";

		if (StopEvent != INVALID_HANDLE_VALUE)
		{
			SetEvent(StopEvent);
		}

		return TRUE;
	}

	return FALSE;
}

void GuardedMain()
{
	FSocketUtility::Startup();

	const i32 PhysicalCoreCount = FPlatformWindowsUtility::GetPhysicalCoreCount();
	const i32 LogicalCoreCount = FPlatformWindowsUtility::GetLogicalCoreCount();

	std::cout << "물리 코어: " << PhysicalCoreCount << ", 논리 코어: " << LogicalCoreCount << '\n';

	StopEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
	SetConsoleCtrlHandler(HandleConsoleCtrl, TRUE);

	FChatServer Server{};
	Server.Startup(7777, PhysicalCoreCount, LogicalCoreCount);

	WaitForSingleObject(StopEvent, INFINITE);

	Server.Cleanup();

	CloseHandle(StopEvent);

	FSocketUtility::Cleanup();
}
