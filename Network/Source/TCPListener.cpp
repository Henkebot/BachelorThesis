#include <pch.h>
#include <Include/TCPListener.h>

////////////////////////////////////////////////////
TCPListener::TCPListener() : Socket(TCP)
{

}

SockStatus TCPListener::Listen(const char* _port)
{
	Socket::close();

	LPADDRINFO addr_srv = nullptr;
	if (Socket::createAddress(IPAddress::Any.ToStr().c_str(), _port, addr_srv) != SockStatus::Done)
	{
		return Socket::getErrorStatus();
	}

	Socket::create(addr_srv);

	if (bind(Socket::getHandle(), addr_srv->ai_addr, static_cast<int>(addr_srv->ai_addrlen)) == -1)
	{
		return Socket::getErrorStatus();
	}

	if (listen(Socket::getHandle(), SOMAXCONN) == -1)
	{
		return SockStatus::Error;
	}
	return SockStatus::Done;
}

SockStatus TCPListener::Accept(TCPSocket & _socket)
{
	if (Socket::getHandle() == INVALID_SOCKET)
	{
		return SockStatus::Error;
	}

	sockaddr_in address;
	int addrLength = sizeof(address);
	SOCKET remote = accept(Socket::getHandle(), reinterpret_cast<sockaddr*>(&address), &addrLength);

	if (remote == INVALID_SOCKET)
	{
		return Socket::getErrorStatus();
	}

	_socket.close();
	_socket.create(remote);

	return SockStatus::Done;
}