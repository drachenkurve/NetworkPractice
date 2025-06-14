#include "SocketAddress.h"

#include "SocketUtility.h"

FSocketAddress::FSocketAddress()
{
	Clear();
}

FSocketAddress::FSocketAddress(const sockaddr_storage& InStorage)
{
	Storage = InStorage;
}

void FSocketAddress::Clear()
{
	memset(&Storage, 0, sizeof(Storage));
}

void FSocketAddress::FromIPv4String(std::string_view String)
{
	std::string Host{};
	std::string Port{};

	const usize Colon = String.find_last_of(':');

	if (Colon != std::string::npos)
	{
		Host = String.substr(0, Colon);
		Port = String.substr(Colon + 1, String.size());
	}
	else
	{
		Host = String;
	}

	std::shared_ptr<FSocketAddress> NewSockAddr = FSocketUtility::GetIPv4AddressFromString(Host);
	if (!NewSockAddr)
	{
		return;
	}

	SetRawIPv4(NewSockAddr->GetRawIPv4());

	if (Port.empty())
	{
		return;
	}

	SetPort(static_cast<u16>(std::atoi(Port.c_str())));
}

std::string FSocketAddress::ToIPv4String(bool bIncludePort) const
{
	std::string String{};

	if (Storage.ss_family == AF_INET)
	{
		char Buffer[NI_MAXHOST];

		const i32 ErrorCode = getnameinfo(reinterpret_cast<const sockaddr*>(&Storage), sizeof(sockaddr_in),
			Buffer, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);

		if (ErrorCode == 0)
		{
			if (bIncludePort)
			{
				String = std::format("{}:{}", Buffer, GetPort());
			}
			else
			{
				String = std::format("{}", Buffer);
			}
		}
		else
		{
			FSocketUtility::ReportLastErrorCode();
		}
	}

	return String;
}

void FSocketAddress::SetIPv4(u32 Addr)
{
	sockaddr_in* IPv4Addr = reinterpret_cast<sockaddr_in*>(&Storage);

	IPv4Addr->sin_family = AF_INET;
	IPv4Addr->sin_addr.s_addr = htonl(Addr);
}

u32 FSocketAddress::GetIPv4() const
{
	if (Storage.ss_family != AF_INET)
	{
		return 0;
	}

	const sockaddr_in* IPv4Addr = reinterpret_cast<const sockaddr_in*>(&Storage);
	const u32 IPv4AddrValue = IPv4Addr->sin_addr.s_addr;

	return ntohl(IPv4AddrValue);
}

void FSocketAddress::SetRawIPv4(const std::vector<u8>& RawSockAddr)
{
	if (RawSockAddr.size() == 4)
	{
		sockaddr_in* IPv4Addr = reinterpret_cast<sockaddr_in*>(&Storage);

		IPv4Addr->sin_family = AF_INET;
		IPv4Addr->sin_addr.s_addr = RawSockAddr[0] << 0 | RawSockAddr[1] << 8 | RawSockAddr[2] << 16 | RawSockAddr[3] << 24;
	}
	else
	{
		Clear();
	}
}

std::vector<u8> FSocketAddress::GetRawIPv4() const
{
	std::vector<u8> RawSockAddr{};

	if (Storage.ss_family == AF_INET)
	{
		const sockaddr_in* IPv4Addr = reinterpret_cast<const sockaddr_in*>(&Storage);
		const u32 IPv4AddrValue = IPv4Addr->sin_addr.s_addr;

		RawSockAddr.reserve(4);
		RawSockAddr.emplace_back(IPv4AddrValue >> 0 & 0xFF);
		RawSockAddr.emplace_back(IPv4AddrValue >> 8 & 0xFF);
		RawSockAddr.emplace_back(IPv4AddrValue >> 16 & 0xFF);
		RawSockAddr.emplace_back(IPv4AddrValue >> 24 & 0xFF);
	}

	return RawSockAddr;
}

void FSocketAddress::SetPort(u16 Port)
{
	reinterpret_cast<sockaddr_in*>(&Storage)->sin_port = htons(Port);
}

u16 FSocketAddress::GetPort() const
{
	return ntohs(reinterpret_cast<const sockaddr_in*>(&Storage)->sin_port);
}

i32 FSocketAddress::GetStorageSize() const
{
	if (Storage.ss_family == AF_INET)
	{
		return sizeof(sockaddr_in);
	}

	return 0;
}
