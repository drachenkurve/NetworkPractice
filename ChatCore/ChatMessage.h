#pragma once

#include "Platform.h"

enum class EChatMessageType : u8
{
	None,
	Login,
	Logout,
	Data,
	System,
};

struct FChatMessage
{
public:
	FChatMessage();
	FChatMessage(EChatMessageType InType, const std::string& InUserName, const std::string& InData);

public:
	EChatMessageType Type;
	u32 UserNameSize;
	u32 DataSize;
	char UserName[32];
	char Data[1024];
};
