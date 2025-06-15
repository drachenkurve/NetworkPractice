#include "ChatMessage.h"

FChatMessage::FChatMessage():
	Type{EChatMessageType::None}, UserNameSize{0}, DataSize{0}, UserName{}, Data{}
{
}

FChatMessage::FChatMessage(EChatMessageType InType, const std::string& InUserName, const std::string& InData):
	Type{InType}, UserNameSize{static_cast<u32>(InUserName.size())}, DataSize{static_cast<u32>(InData.size())}, UserName{}, Data{}
{
	strncpy_s(UserName, InUserName.c_str(), sizeof(UserName) - 1);
	strncpy_s(Data, InData.c_str(), sizeof(Data) - 1);
}
