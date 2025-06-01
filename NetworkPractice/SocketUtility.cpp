#include "SocketUtility.h"

bool FSocketUtility::Startup()
{
	WSADATA WSAData;

	const int32_t ErrorCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ErrorCode == 0)
	{
		std::cout << std::format("WinSock: Version {}.{} ({}.{}), MaxSocket={}, MaxUdp={}",
			WSAData.wVersion >> 8, WSAData.wVersion & 0xFF,
			WSAData.wHighVersion >> 8, WSAData.wHighVersion & 0xFF,
			WSAData.iMaxSockets, WSAData.iMaxUdpDg);

		return true;
	}

	ReportLastErrorCode();
	return false;
}

void FSocketUtility::Cleanup()
{
	WSACleanup();
}

int32_t FSocketUtility::GetLastErrorCode()
{
	return WSAGetLastError();
}

void FSocketUtility::ReportLastErrorCode()
{
	LPSTR Buffer = nullptr;
	const int32_t ErrorCode = GetLastErrorCode();

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		ErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&Buffer),
		0, nullptr);

	if (!Buffer)
	{
		return;
	}

	std::cout << std::format("Error {}: {}", ErrorCode, Buffer);

	LocalFree(Buffer);
}

std::shared_ptr<FSocketAddress> FSocketUtility::GetIPv4Address(const std::string& Host, const std::string& Service)
{
	addrinfo HintAddrInfo{};
	HintAddrInfo.ai_family = AF_INET;

	addrinfo* AddrInfo = nullptr;

	const int32_t ErrorCode = getaddrinfo(Host.c_str(), Service.c_str(), &HintAddrInfo, &AddrInfo);

	if (ErrorCode != 0)
	{
		ReportLastErrorCode();

		if (AddrInfo)
		{
			freeaddrinfo(AddrInfo);
		}

		return nullptr;
	}

	addrinfo* AddrInfoHead = AddrInfo;

	while (!AddrInfo->ai_addr && AddrInfo->ai_next)
	{
		AddrInfo = AddrInfo->ai_next;
	}

	if (!AddrInfo->ai_addr)
	{
		freeaddrinfo(AddrInfoHead);
	}

	const std::shared_ptr<FSocketAddress> SocketAddress = std::make_shared<FSocketAddress>(*AddrInfo->ai_addr);

	freeaddrinfo(AddrInfoHead);

	return SocketAddress;
}

std::shared_ptr<FSocketUDP> FSocketUtility::CreateIPv4UDP()
{
	SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (!Socket)
	{
		ReportLastErrorCode();
		return nullptr;
	}

	return std::make_shared<FSocketUDP>(Socket);
}
