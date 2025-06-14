#include "SocketUtility.h"

#include "SocketAddress.h"
#include "TcpSocket.h"
#include "UdpSocket.h"

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

std::shared_ptr<FTcpSocket> FSocketUtility::CreateIPv4TCP()
{
	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket != INVALID_SOCKET)
	{
		return std::make_shared<FTcpSocket>(Socket);
	}

	ReportLastErrorCode();
	return nullptr;
}

std::shared_ptr<FUdpSocket> FSocketUtility::CreateIPv4UDP()
{
	SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Socket != INVALID_SOCKET)
	{
		return std::make_shared<FUdpSocket>(Socket);
	}

	ReportLastErrorCode();
	return nullptr;
}

i32 FSocketUtility::Select(const std::vector<std::shared_ptr<FTcpSocket>>& ReadSockets, std::vector<std::shared_ptr<FTcpSocket>>& OutReadSockets,
	const std::vector<std::shared_ptr<FTcpSocket>>& WriteSockets, std::vector<std::shared_ptr<FTcpSocket>>& OutWriteSockets,
	const std::vector<std::shared_ptr<FTcpSocket>>& ExceptSockets, std::vector<std::shared_ptr<FTcpSocket>>& OutExceptSockets,
	std::chrono::microseconds Timeout)
{
	// TODO: should I check timeout overflow?

	fd_set ReadSet;
	fd_set WriteSet;
	fd_set ExceptSet;

	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	FD_ZERO(&ExceptSet);

	for (const std::shared_ptr<FTcpSocket>& ReadSocket : ReadSockets)
	{
		FD_SET(ReadSocket->Socket, &ReadSet);
	}

	for (const std::shared_ptr<FTcpSocket>& WriteSocket : WriteSockets)
	{
		FD_SET(WriteSocket->Socket, &WriteSet);
	}

	for (const std::shared_ptr<FTcpSocket>& ExceptSocket : ExceptSockets)
	{
		FD_SET(ExceptSocket->Socket, &ExceptSet);
	}

	timeval Timeval;
	Timeval.tv_sec = static_cast<i32>(Timeout.count()) / 1000000;
	Timeval.tv_usec = static_cast<i32>(Timeout.count()) % 1000000;

	// nfds is ignored in Windows
	const i32 ReturnCode = select(0, &ReadSet, &WriteSet, &ExceptSet, &Timeval);

	if (ReturnCode > 0)
	{
		OutReadSockets.clear();
		OutReadSockets.reserve(ReadSockets.size());

		for (const std::shared_ptr<FTcpSocket>& ReadSocket : ReadSockets)
		{
			if (FD_ISSET(ReadSocket->Socket, &ReadSet))
			{
				OutReadSockets.emplace_back(ReadSocket);
			}
		}

		OutWriteSockets.clear();
		OutWriteSockets.reserve(WriteSockets.size());

		for (const std::shared_ptr<FTcpSocket>& WriteSocket : WriteSockets)
		{
			if (FD_ISSET(WriteSocket->Socket, &WriteSet))
			{
				OutWriteSockets.emplace_back(WriteSocket);
			}
		}

		OutExceptSockets.clear();
		OutExceptSockets.reserve(ExceptSockets.size());

		for (const std::shared_ptr<FTcpSocket>& ExceptSocket : ExceptSockets)
		{
			if (FD_ISSET(ExceptSocket->Socket, &ExceptSet))
			{
				OutExceptSockets.emplace_back(ExceptSocket);
			}
		}
	}

	return ReturnCode;
}
