#include <pch.h>
#include <Include/TCPSocket.h>


////////////////////////////////////////////////////
TCPSocket::TCPSocket() :
	Socket(TCP)
{

}

////////////////////////////////////////////////////
SockStatus TCPSocket::Connect(const IPAddress & _remoteadr, const char* _remotePort)
{
	// Disconnect the socket if it is already connected
	Disconnect();

	LPADDRINFO addr_srv = nullptr;
	SockStatus status = Socket::createAddress(_remoteadr.ToStr().c_str(), _remotePort, addr_srv);
	if (status != SockStatus::Done)
	{
		return Socket::getErrorStatus();
	}

	Socket::create(addr_srv);


	if (connect(getHandle(), addr_srv->ai_addr, addr_srv->ai_addrlen) == -1)
	{
		return Socket::getErrorStatus();
	}

	return SockStatus::Done;
}

////////////////////////////////////////////////////
SockStatus TCPSocket::Send(const void * data, unsigned int size, unsigned int & sent)
{
	if (!data || (size == 0))
	{
		return SockStatus::Error;
	}

	int result = 0;
	for (sent = 0; sent < size; sent += result)
	{
		result = send(Socket::getHandle(), static_cast<const char*>(data) + sent, static_cast<int>(size - sent), 0);

		if (result < 0)
		{
			SockStatus status = Socket::getErrorStatus();

			if ((status == SockStatus::NotReady) && sent)
			{
				return SockStatus::Partial;
			}
			return status;
		}
	}
	return SockStatus::Done;
}

////////////////////////////////////////////////////
SockStatus TCPSocket::Send(Packet & packet)
{
	unsigned int size = 0;

	const void* data = packet.onSend(size);

	UINT32 packetSize = htonl(static_cast<UINT32>(size));

	std::vector<char> blockToSend(sizeof(packetSize) + size);

	std::memcpy(&blockToSend[0], &packetSize, sizeof(packetSize));
	if (size > 0)
		std::memcpy(&blockToSend[0] + sizeof(packetSize), data, size);

	unsigned int sent;

	SockStatus status = Send(&blockToSend[0] + packet.m_sendPos, blockToSend.size() - packet.m_sendPos, sent);

	if (status == SockStatus::Partial)
	{
		packet.m_sendPos += sent;
	}
	else if (status == SockStatus::Done)
	{
		packet.m_sendPos = 0;
	}

	return status;
}

////////////////////////////////////////////////////
SockStatus TCPSocket::Receive(void * data, std::size_t size, std::size_t & received)
{
	received = 0;

	if (!data)
	{
		std::cerr << __LINE__ << std::endl;
		return SockStatus::Error;
	}

	int sizeReceived = recv(getHandle(), static_cast<char*>(data), static_cast<int>(size), 0);

	if (sizeReceived > 0)
	{
		received = static_cast<std::size_t>(sizeReceived);
		return SockStatus::Done;
	}
	else if (sizeReceived == 0)
	{
		return SockStatus::Disconnected;
	}
	else
	{
		return Socket::getErrorStatus();
	}
}

////////////////////////////////////////////////////
SockStatus TCPSocket::Receive(Packet & packet)
{
	packet.Clear();

	UINT32 packetSize = 0;
	std::size_t received = 0;
	if (m_pendingPacket.SizeReceived < sizeof(m_pendingPacket.Size))
	{
		while (m_pendingPacket.SizeReceived < sizeof(m_pendingPacket.Size))
		{
			char* data = reinterpret_cast<char*>(&m_pendingPacket.Size) + m_pendingPacket.SizeReceived;
			SockStatus status = Receive(data, sizeof(m_pendingPacket.Size) - m_pendingPacket.SizeReceived, received);
			m_pendingPacket.SizeReceived += received;

			if (status != SockStatus::Done)
			{
				return status;
			}

		}

		packetSize = ntohl(m_pendingPacket.Size);
	}
	else
	{
		packetSize = ntohl(m_pendingPacket.Size);
	}

	char buffer[1024];
	while (m_pendingPacket.Data.size() < packetSize)
	{
		std::size_t sizeToGet = (std::min)(static_cast<std::size_t>(packetSize - m_pendingPacket.Data.size()), sizeof(buffer));
		SockStatus status = Receive(buffer, sizeToGet, received);
		if (status != SockStatus::Done)
			return status;

		if (received > 0)
		{
			m_pendingPacket.Data.resize(m_pendingPacket.Data.size() + received);
			char* begin = &m_pendingPacket.Data[0] + m_pendingPacket.Data.size() - received;
			std::memcpy(begin, buffer, received);
		}
	}

	if (!m_pendingPacket.Data.empty())
	{
		packet.onReceive(&m_pendingPacket.Data[0], m_pendingPacket.Data.size());
	}

	m_pendingPacket = PendingPacket();

	return SockStatus::Done;
}

////////////////////////////////////////////////////
void TCPSocket::Disconnect()
{
	Socket::close();
}

////////////////////////////////////////////////////
TCPSocket::PendingPacket::PendingPacket() :
	Size(0),
	SizeReceived(0),
	Data()
{
}