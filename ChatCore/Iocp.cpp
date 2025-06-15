#include "Iocp.h"

#include "TcpSocket.h"
#include "UdpSocket.h"

FIocp::FIocp(i32 InThreadCount)
{
	Handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, InThreadCount);
	ThreadCount = InThreadCount;
}

FIocp::~FIocp()
{
	if (Handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(Handle);
		Handle = INVALID_HANDLE_VALUE;
	}
}

bool FIocp::IsValid() const
{
	return Handle != INVALID_HANDLE_VALUE;
}

// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppMemberFunctionMayBeStatic
bool FIocp::Send(const std::shared_ptr<FTcpSocket>& Socket, OVERLAPPED* Overlapped,
	const u8* Data, u32 Size, u32& ByteCount, u32 Flags)
{
	WSABUF Buffer;
	Buffer.buf = reinterpret_cast<CHAR*>(const_cast<u8*>(Data));
	Buffer.len = Size;

	const i32 ErrorCode = WSASend(Socket->GetNative(), &Buffer, 1, reinterpret_cast<DWORD*>(&ByteCount), Flags,
		Overlapped, nullptr);

	if (ErrorCode == 0)
	{
		return true;
	}

	if (ErrorCode == SOCKET_ERROR && FSocketUtility::GetLastErrorCode() == WSA_IO_PENDING)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppMemberFunctionMayBeStatic
bool FIocp::Recv(const std::shared_ptr<FTcpSocket>& Socket, OVERLAPPED* Overlapped,
	u8* Data, u32 Size, u32& ByteCount, u32 Flags)
{
	WSABUF Buffer;
	Buffer.buf = reinterpret_cast<CHAR*>(Data);
	Buffer.len = Size;

	const i32 ErrorCode = WSARecv(Socket->GetNative(), &Buffer, 1, reinterpret_cast<DWORD*>(&ByteCount), reinterpret_cast<DWORD*>(&Flags),
		Overlapped, nullptr);

	if (ErrorCode == 0)
	{
		return true;
	}

	if (ErrorCode == SOCKET_ERROR && FSocketUtility::GetLastErrorCode() == WSA_IO_PENDING)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppMemberFunctionMayBeStatic
bool FIocp::SendTo(const std::shared_ptr<FUdpSocket>& Socket, OVERLAPPED* Overlapped,
	const u8* Data, u32 Size, u32& ByteCount, const FSocketAddress& Address, u32 Flags)
{
	WSABUF Buffer;
	Buffer.buf = reinterpret_cast<CHAR*>(const_cast<u8*>(Data));
	Buffer.len = Size;

	const i32 ErrorCode = WSASendTo(Socket->GetNative(), &Buffer, 1, reinterpret_cast<DWORD*>(&ByteCount), Flags,
		reinterpret_cast<const sockaddr*>(&Address.Storage), Address.GetStorageSize(), Overlapped, nullptr);

	if (ErrorCode == 0)
	{
		return true;
	}

	if (ErrorCode == SOCKET_ERROR && FSocketUtility::GetLastErrorCode() == WSA_IO_PENDING)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppMemberFunctionMayBeStatic
bool FIocp::RecvFrom(const std::shared_ptr<FUdpSocket>& Socket, OVERLAPPED* Overlapped,
	u8* Data, u32 Size, u32& ByteCount, FSocketAddress& Address, u32 Flags)
{
	WSABUF Buffer;
	Buffer.buf = reinterpret_cast<CHAR*>(Data);
	Buffer.len = Size;

	i32 Length = sizeof(sockaddr_storage);

	const i32 ErrorCode = WSARecvFrom(Socket->GetNative(), &Buffer, 1, reinterpret_cast<DWORD*>(&ByteCount), reinterpret_cast<DWORD*>(&Flags),
		reinterpret_cast<sockaddr*>(&Address.Storage), &Length, Overlapped, nullptr);

	if (ErrorCode == 0)
	{
		return true;
	}

	if (ErrorCode == SOCKET_ERROR && FSocketUtility::GetLastErrorCode() == WSA_IO_PENDING)
	{
		return true;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FIocp::Dequeue(std::vector<OVERLAPPED_ENTRY>& OverlappedEntries, u32& EntryCount, std::chrono::milliseconds Timeout)
{
	if (GetQueuedCompletionStatusEx(Handle, OverlappedEntries.data(), static_cast<ULONG>(OverlappedEntries.size()),
		reinterpret_cast<ULONG*>(&EntryCount), static_cast<ULONG>(Timeout.count()), FALSE))
	{
		return true;
	}

	if (GetLastError() == WAIT_TIMEOUT)
	{
		return false;
	}

	FSocketUtility::ReportLastErrorCode();
	return false;
}
