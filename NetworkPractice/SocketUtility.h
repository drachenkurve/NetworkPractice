#pragma once

#include "Platform.h"

class FSocketAddress;
class FTCPSocket;
class FUDPSocket;

class FSocketUtility
{
public:
	static bool Startup();
	static void Cleanup();

	static i32 GetLastErrorCode();
	static void ReportErrorCode(i32 ErrorCode);
	static void ReportLastErrorCode();

	static std::shared_ptr<FSocketAddress> GetIPv4Address(const std::string& Host, const std::string& Service);

	static std::shared_ptr<FTCPSocket> CreateIPv4TCP();
	static std::shared_ptr<FUDPSocket> CreateIPv4UDP();
};
