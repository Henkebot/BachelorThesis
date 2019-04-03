#include <pch.h>
#include <Include/Socket.h>

struct SocketInit
{
	SocketInit()
	{
		WSADATA init;
		WSAStartup(MAKEWORD(2, 2), &init);
	}

	~SocketInit()
	{
		WSACleanup();
	}
};

SocketInit globalInit;

////////////////////////////////////////////////////
Socket::Socket(Type _type) :
	m_type(_type),
	m_socket(INVALID_SOCKET),
	m_isBlocking(true)
{
}

////////////////////////////////////////////////////
void Socket::create(LPADDRINFO _addr)
{
	if (m_socket == INVALID_SOCKET)
	{
		SOCKET handle = socket(_addr->ai_family, _addr->ai_socktype, _addr->ai_protocol);

		if (handle == INVALID_SOCKET)
		{
			std::cerr << __FILE__ << std::endl;
		}

		create(handle);

	}
}

////////////////////////////////////////////////////
void Socket::create(SOCKET handle)
{
	if (m_socket == INVALID_SOCKET)
	{
		m_socket = handle;

		SetBlocking(m_isBlocking);

		if (m_type == TCP)
		{
			// Disable the Nagle algorithm https://en.wikipedia.org/wiki/Nagle%27s_algorithm
			int yes = 1;
			if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&yes), sizeof(yes)) == -1)
			{
				std::cerr << __FILE__ << std::endl;
			}
		}
		else // UDP
		{
			// Enable broadcast by default for UDP sockets
			int yes = 1;
			if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes)) == -1)
			{
				std::cerr << __FILE__ << std::endl;
			}
		}

	}
}

////////////////////////////////////////////////////
void Socket::close()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

SockStatus Socket::createAddress(const char* _address, const char* _port, LPADDRINFO& _addr)
{
	ADDRINFO hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(_address, _port, &hints, &_addr) != 0)
	{
		return Socket::getErrorStatus();
	}

	if (_addr == nullptr)
	{
		return Socket::getErrorStatus();
	}

	return SockStatus::Done;
}

////////////////////////////////////////////////////
SOCKET Socket::getHandle() const
{
	return m_socket;
}

////////////////////////////////////////////////////
SockStatus Socket::getErrorStatus() const
{
	switch (WSAGetLastError())
	{
	case WSAEWOULDBLOCK:	return SockStatus::NotReady;
	case WSAEALREADY:		return SockStatus::NotReady;
	case WSAECONNABORTED:	return SockStatus::Disconnected;
	case WSAECONNRESET:		return SockStatus::Disconnected;
	case WSAETIMEDOUT:		return SockStatus::Disconnected;
	case WSAENETRESET:		return SockStatus::Disconnected;
	case WSAENOTCONN:		return SockStatus::Disconnected;
	case WSAEISCONN:		return SockStatus::Done;
	default:
		std::cout << WSAGetLastError() << std::endl;
		return SockStatus::Error;
	}
}

bool Socket::operator==(const Socket & other)
{
	return m_socket == other.m_socket;
}


////////////////////////////////////////////////////
void Socket::SetBlocking(bool _blocking)
{
	u_long blocking = _blocking ? 0 : 1;
	ioctlsocket(m_socket, FIONBIO, &blocking);

	m_isBlocking = _blocking;
}
