#include "SocketAddress.h"
#include "SocketUtility.h"

void GuardedMain()
{
	FSocketUtility::Startup();

	std::shared_ptr<FSocketAddress> address = FSocketUtility::CreateIPv4Address();

	address->FromIPv4String("92.30.252.0");

	std::cout << address->GetIPv4() << '\n';
	std::cout << address->GetPort() << '\n';

	address->SetPort(80);
	std::cout << address->GetPort() << '\n';

	std::cout << address->ToIPv4String(false) << '\n';
	std::cout << address->ToIPv4String(true) << '\n';

	address->FromIPv4String("92.30.252.0:80");

	std::cout << address->GetIPv4() << '\n';
	std::cout << address->GetPort() << '\n';
	std::cout << address->ToIPv4String(false) << '\n';
	std::cout << address->ToIPv4String(true) << '\n';

	FSocketUtility::Cleanup();
}
