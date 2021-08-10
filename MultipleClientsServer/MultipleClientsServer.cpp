#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ws2tcpip.h>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

int main()
{
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		std::cerr << "Cannot initialize winsock... Quitting..." << std::endl;
		return 0;
	}

	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Cannot create socket... Quitting..." << std::endl;
		return 0;
	}

	int port = 54000;

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	listen(listening, SOMAXCONN);

	std::cout << "Server is waiting for connections..." << std::endl;

	fd_set master;
	FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true)
	{
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				sockaddr_in clientAddr;
				int clientSize = sizeof(clientAddr);

				SOCKET client = accept(listening, (sockaddr*)&clientAddr, &clientSize);

				char host[NI_MAXHOST]; // Client's remote name
				char service[NI_MAXSERV]; // Port the client is connected on

				ZeroMemory(host, NI_MAXHOST);
				ZeroMemory(service, NI_MAXSERV);

				if (getnameinfo((sockaddr*)&clientAddr, clientSize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
				{
					std::cout << host << " connected on port " << service << std::endl;
				}
				else
				{
					inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
					std::cout << host << " connected on port " << ntohs(clientAddr.sin_port) << std::endl;
				}

				FD_SET(client, &master);

				std::string welcomeMsg = "Welcome to the Chat Server\r\n\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size()+1, 0);
				

			}
			else
			{
				const int bufSz = 4096;
				char buf[bufSz];
				ZeroMemory(buf, bufSz);
				int bytesIn = recv(sock, buf, bufSz, 0);

				if (bytesIn <= 0)
				{
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock)
						{
							std::ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							std::string strOut = ss.str();


							send(outSock, strOut.c_str(), strOut.size()+1, 0);
						
						}
					}
				}
			}
		}
	}

	WSACleanup();
}