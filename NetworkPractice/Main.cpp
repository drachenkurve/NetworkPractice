#include "SocketUtility.h"

void GuardedMain()
{
	FSocketUtility::Startup();


	FSocketUtility::Cleanup();
}
