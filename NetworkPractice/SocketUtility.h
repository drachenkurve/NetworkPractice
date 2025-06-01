#pragma once

#include "Platform.h"
#include "SocketAddress.h"
#include "SocketUDP.h"

class FSocketUtility
{
public:
	static bool Startup();
	static void Cleanup();

	static int32_t GetLastErrorCode();
	static void ReportLastErrorCode();

	static std::shared_ptr<FSocketAddress> GetIPv4Address(const std::string& Host, const std::string& Service);
	static std::shared_ptr<FSocketUDP> CreateIPv4UDP();
};
