#include "Connection.h"

FConnection::FConnection(const std::shared_ptr<FTcpSocket>& InSocket, std::string_view InUserName, const FSocketAddress& InAddress) :
	Socket{InSocket}, UserName{InUserName}, Address{InAddress}, bConnected{true}
{
}
