#include "SocketUDP.h"

#include "SocketAddress.h"
#include "SocketUtility.h"

FSocketUDP::FSocketUDP(SOCKET InSocket) :
	Socket(InSocket)
{
}

FSocketUDP::~FSocketUDP()
{
	closesocket(Socket);
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FSocketUDP::Bind(const FSocketAddress& Address)
{
	const int32_t ErrorCode = bind(Socket, &Address.SockAddr, sizeof(Address.SockAddr));
	if (ErrorCode == 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FSocketUDP::SendTo(const uint8_t* Data, int32_t Count, int32_t& ByteCount, const FSocketAddress& ToAddress)
{
	ByteCount = sendto(Socket, reinterpret_cast<const char*>(Data), Count, 0, &ToAddress.SockAddr, sizeof(ToAddress.SockAddr));

	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FSocketUDP::RecvFrom(uint8_t* Data, int32_t Count, int32_t& ByteCount, FSocketAddress& FromAddress)
{
	int32_t Length = sizeof(FromAddress.SockAddr);

	ByteCount = recvfrom(Socket, reinterpret_cast<char*>(Data), Count, 0, &FromAddress.SockAddr, &Length);

	if (ByteCount >= 0)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}
