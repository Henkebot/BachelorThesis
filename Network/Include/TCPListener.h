#pragma once
#include "TCPSocket.h"

class TCPListener : public Socket
{
public:

	TCPListener();

	SockStatus Listen(const char* _port);

	SockStatus Accept(TCPSocket& _socket);
};