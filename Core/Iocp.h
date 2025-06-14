#pragma once

#include "Platform.h"

class FSocketAddress;
class FUdpSocket;
class FTcpSocket;

class FIocp
{
public:
	FIocp(i32 InThreadCount);

	FIocp(const FIocp& Other) = delete;
	FIocp(FIocp&& Other) noexcept = delete;

	FIocp& operator=(const FIocp& Other) = delete;
	FIocp& operator=(FIocp&& Other) noexcept = delete;

	~FIocp();

	bool IsValid() const;

	template<typename T>
	bool Register(const std::shared_ptr<T>& Socket, void* Key);

	bool Send(const std::shared_ptr<FTcpSocket>& Socket, OVERLAPPED* Overlapped,
		const u8* Data, u32 Size, u32& ByteCount, u32 Flags = 0);

	bool Recv(const std::shared_ptr<FTcpSocket>& Socket, OVERLAPPED* Overlapped,
		u8* Data, u32 Size, u32& ByteCount, u32 Flags = 0);

	bool SendTo(const std::shared_ptr<FUdpSocket>& Socket, OVERLAPPED* Overlapped,
		const u8* Data, u32 Size, u32& ByteCount, const FSocketAddress& Address, u32 Flags = 0);

	bool RecvFrom(const std::shared_ptr<FUdpSocket>& Socket, OVERLAPPED* Overlapped,
		u8* Data, u32 Size, u32& ByteCount, FSocketAddress& Address, u32 Flags = 0);

	bool Dequeue(std::vector<OVERLAPPED_ENTRY>& OverlappedEntries, u32& EntryCount, std::chrono::microseconds Timeout);

protected:
	HANDLE Handle;
	i32 ThreadCount;
};

#include "Iocp.inl"
