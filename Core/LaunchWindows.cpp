#include "LaunchWindows.h"

void LaunchWindowsStartup(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* pCmdLine, int32_t nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#ifdef USE_SUBSYSTEM_CONSOLE
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

	try
	{
		GuardedMain();
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << '\n';
	}
}

void LaunchWindowsShutdown()
{
	_CrtDumpMemoryLeaks();
}

i32 WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ char* pCmdLine, _In_ INT32 nCmdShow)
{
	LaunchWindowsStartup(hInstance, hPrevInstance, pCmdLine, nCmdShow);
	LaunchWindowsShutdown();
}
