#pragma once

#include "Platform.h"

class FSocketAddress
{
	friend class FSocketUtility;

public:
	FSocketAddress();
	FSocketAddress(const sockaddr_storage& InStorage);

	void Clear();

	// set IPv4 address from string, such as "192.30.252.0:80"
	void FromIPv4String(std::string_view String);

	// get IPv4 address from storage, such as "192.30.252.0:80"
	std::string ToIPv4String(bool bIncludePort) const;

	void SetIPv4(u32 Addr);
	u32 GetIPv4() const;

	// set IPv4 address from bytes, bytes should be network byte order
	void SetRawIPv4(const std::vector<u8>& RawSockAddr);

	// set IPv4 address from bytes, bytes are stored as-is
	std::vector<u8> GetRawIPv4() const;

	void SetPort(u16 Port);
	u16 GetPort() const;

	i32 GetStorageSize() const;

public:
	sockaddr_storage Storage;
};
