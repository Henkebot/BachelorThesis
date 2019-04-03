#pragma once
#include "../pch.h"

/*
	Status codes that may be returned by socket functions
*/
enum class SockStatus
{
	Done,
	NotReady,
	Partial,
	Disconnected,
	Error
};


class Socket
{
public:
	/*
		Set the blocking state of the socket

	*/
	void SetBlocking(bool _blocking);

	virtual bool operator==(const Socket& other);

protected:
	enum Type
	{
		TCP,	// TCP protcol
		UDP		// UDP protcol
	};

	Socket(Type _type);

	void create(LPADDRINFO _addr);

	void create(SOCKET handle);

	void close();

	SockStatus createAddress(const char* _address, const char* _port, LPADDRINFO& _addr);

	SOCKET getHandle() const;

	SockStatus getErrorStatus() const;


private:
	Type			m_type;
	SOCKET	m_socket;
	bool			m_isBlocking;
};