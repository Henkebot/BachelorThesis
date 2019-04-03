// Sandbox.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Network/Include/TCPSocket.h>
#include <Network/Include/TCPListener.h>

const char* G_PORT = "27015";

void Server()
{
	TCPListener listener;
	SockStatus status;

	status = listener.Listen(G_PORT);
	if (status == SockStatus::Done)
		std::cout << "Bound on (" << G_PORT << ")\n";
	else
		return;

	std::vector<TCPSocket> sockets;

	listener.SetBlocking(false);
	while (true)
	{
		TCPSocket newSock;

		status = listener.Accept(newSock);
		if (status == SockStatus::Done)
		{
			std::cout << "new socket accepted!" << std::endl;
			newSock.SetBlocking(false);
			sockets.push_back(newSock);
		}

		for (int i = 0; i < sockets.size(); i++)
		{
			Packet p;
			status = sockets[i].Receive(p);
			if (status == SockStatus::Done)
			{
				for (int j = 0; j < sockets.size(); j++)
				{
					if (j != i)
					{
						sockets[j].Send(p);
					}
				}
			}

		}

	}
}

void Client()
{
	TCPSocket s;

	if (s.Connect(IPAddress::Localhost, G_PORT) == SockStatus::Done)
	{
		s.SetBlocking(false);
		std::string str;
		do
		{
			Packet p;
			s.Receive(p);
			if (p.GetDataSize())
			{
				p >> str;
				std::cout << str << std::endl;
			}

			std::getline(std::cin, str);
			if (str.size())
			{
				p << str;
				s.Send(p);
			}
		} while (str[0] != 'q');
	}

}
int main()
{
	int val;
	std::cin >> val;
	std::cin.ignore();
	if (val == 1)
	{
		Server();
	}
	else
	{
		Client();
	}

	return 0;

	system("pause");
}