#include "SocketUtility.h"

#include "SocketAddress.h"
#include "TCPSocket.h"
#include "UDPSocket.h"

bool FSocketUtility::Startup()
{
	WSADATA WSAData;

	const i32 ErrorCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ErrorCode == 0)
	{
		std::cout << std::format("WinSock Version {}.{}\n", WSAData.wVersion >> 8, WSAData.wVersion & 0xFF);
		return true;
	}

	ReportErrorCode(ErrorCode);
	return false;
}

void FSocketUtility::Cleanup()
{
	WSACleanup();
}

i32 FSocketUtility::GetLastErrorCode()
{
	return WSAGetLastError();
}

void FSocketUtility::ReportErrorCode(i32 ErrorCode)
{
	char* Buffer = nullptr;

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		ErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<char*>(&Buffer),
		0, nullptr);

	if (!Buffer)
	{
		return;
	}

	std::cout << std::format("Error {}: {}\n", ErrorCode, Buffer);

	LocalFree(Buffer);
}

void FSocketUtility::ReportLastErrorCode()
{
	ReportErrorCode(GetLastErrorCode());
}

std::shared_ptr<FSocketAddress> FSocketUtility::CreateIPv4Address()
{
	return std::make_shared<FSocketAddress>();
}

std::shared_ptr<FSocketAddress> FSocketUtility::GetIPv4AddressFromString(std::string_view Host)
{
	sockaddr_storage Storage{};
	sockaddr_in* IPv4Address = reinterpret_cast<sockaddr_in*>(&Storage);

	IPv4Address->sin_family = AF_INET;

	const i32 ReturnCode = inet_pton(AF_INET, Host.data(), &IPv4Address->sin_addr);
	if (ReturnCode == 1)
	{
		return std::make_shared<FSocketAddress>(Storage);
	}

	ReportLastErrorCode();
	return nullptr;
}

//std::shared_ptr<FSocketAddress> FSocketUtility::GetIPv4AddressFromDomain(const std::string& Host, const std::string& Service)
//{
//	addrinfo HintAddrInfo{};
//	HintAddrInfo.ai_family = AF_INET;
//
//	addrinfo* AddrInfo = nullptr;
//
//	const i32 ErrorCode = getaddrinfo(Host.c_str(), Service.c_str(), &HintAddrInfo, &AddrInfo);
//	if (ErrorCode != 0)
//	{
//		ReportErrorCode(ErrorCode);
//
//		if (AddrInfo)
//		{
//			freeaddrinfo(AddrInfo);
//		}
//
//		return nullptr;
//	}
//
//	addrinfo* AddrInfoHead = AddrInfo;
//
//	while (!AddrInfo->ai_addr && AddrInfo->ai_next)
//	{
//		AddrInfo = AddrInfo->ai_next;
//	}
//
//	if (!AddrInfo || AddrInfo->ai_addr)
//	{
//		freeaddrinfo(AddrInfoHead);
//		return nullptr;
//	}
//
//	const std::shared_ptr<FSocketAddress> SocketAddress = std::make_shared<FSocketAddress>(*AddrInfo->ai_addr);
//
//	freeaddrinfo(AddrInfoHead);
//
//	return SocketAddress;
//}

std::shared_ptr<FTCPSocket> FSocketUtility::CreateIPv4TCP()
{
	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket != INVALID_SOCKET)
	{
		return std::make_shared<FTCPSocket>(Socket);
	}

	ReportLastErrorCode();
	return nullptr;
}

std::shared_ptr<FUDPSocket> FSocketUtility::CreateIPv4UDP()
{
	SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Socket != INVALID_SOCKET)
	{
		return std::make_shared<FUDPSocket>(Socket);
	}

	ReportLastErrorCode();
	return nullptr;
}
