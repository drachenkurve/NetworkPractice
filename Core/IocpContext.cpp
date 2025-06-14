#include "IocpContext.h"

FIocpContext::FIocpContext(EIocpOperationType InOperationType, const std::shared_ptr<FTcpSocket>& InSocket):
	OperationType{InOperationType}, Socket{InSocket}
{
	memset(&Overlapped, 0, sizeof(Overlapped));
}
