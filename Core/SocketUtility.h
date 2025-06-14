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

	static std::shared_ptr<FSocketAddress> CreateIPv4Address();

	// get IPv4 address from string, such as "192.30.252.0"
	static std::shared_ptr<FSocketAddress> GetIPv4AddressFromString(std::string_view Host);

	// TODO: this should return array of socket address info result
	// get IPv4 address from domain, such as "github.com", "http"
	//static std::shared_ptr<FSocketAddress> GetIPv4AddressFromDomain(const std::string& Host, const std::string& Service);

	static std::shared_ptr<FTCPSocket> CreateIPv4TCP();
	static std::shared_ptr<FUDPSocket> CreateIPv4UDP();
};
