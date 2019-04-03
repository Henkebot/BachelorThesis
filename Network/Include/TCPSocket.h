#pragma once
#include "Socket.h"
#include "IPAddress.h"
#include "Packet.h"

class TCPListener;
class TCPSocket : public Socket
{
public:
	/*
		Default Constructor
	*/
	TCPSocket();

	SockStatus Connect(const IPAddress& _remoteadr, const char* _remotePort);

	SockStatus Send(const void* data, unsigned int size, unsigned int& sent);

	SockStatus Send(Packet& packet);
	
	SockStatus Receive(void* data, std::size_t size, std::size_t& received);
	
	SockStatus Receive(Packet& packet);

	void Disconnect();
private:
	friend class TCPListener;

	struct PendingPacket
	{
		PendingPacket();

		UINT32 Size;
		std::size_t SizeReceived;
		std::vector<char> Data;
	};

	PendingPacket m_pendingPacket;

};

