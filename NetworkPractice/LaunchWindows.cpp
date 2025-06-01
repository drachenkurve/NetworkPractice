#include "Platform.h"
#include "SocketUtility.h"

void LaunchWindowsStartup(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* pCmdLine, int32_t nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#if defined(DEBUG) | defined(_DEBUG)
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

	try
	{
		FSocketUtility::Startup();
		FSocketUtility::GetIPv4Address("", "");
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << '\n';
	}

	FSocketUtility::Cleanup();
}

void LaunchWindowsShutdown()
{
	_CrtDumpMemoryLeaks();
}

int32_t WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ char* pCmdLine, _In_ INT32 nCmdShow)
{
	LaunchWindowsStartup(hInstance, hPrevInstance, pCmdLine, nCmdShow);
	LaunchWindowsShutdown();
}
