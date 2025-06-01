#include "SocketUtility.h"

void GuardedMain()
{
	FSocketUtility::Startup();

	{
		FSocketUtility::CreateIPv4TCP();
		FSocketUtility::CreateIPv4UDP();
	}

	FSocketUtility::Cleanup();
}
