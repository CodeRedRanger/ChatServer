#include "ChatServer.h"


/*

//Put in message handler
/*
//inside message handler file
1. Command detection
if (msg[0] == '~')

2. Routing
if (command == "register") → AuthManager
if (command == "login")    → AuthManager
if (command == "send")     → broadcast logic

3. Public messages
Anything without command character at the start of message
broadcast public chat

//command parsing layer (move to messageHandler)
//registration logic (move to authManager)
//authentication state tracking (move to authManager)
//routing between commands vs chat messages (move to messageHandler)


//other files
Component	Responsibility
ChatServer	networking only IN PROGRESS
MessageHandler	command parsing + routing
AuthManager	users + login state DONE
Logger	file writing TO DO LATER
TcpFraming	send/recv safety  DONE

*/

void Server::ServerCode(void)
{

	uint16_t port;
	int capacity;
	char commandChar;

	std::cout << "Port: ";
	std::cin >> port;

	std::cout << "Capacity: ";
	std::cin >> capacity;
	//NEED TO PASS THIS SOMEWHERE



	std::cout << "Command character: ";
	std::cin >> commandChar;


	//Socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("DEBUG// Socket function incorrect\n");
		return;
	}
	else
	{
		printf("DEBUG// I used the socket function\n");
	}

	//Bind
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port); //31337	

	int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Bind function incorrect\n");
		return;
	}
	else
	{
		printf("DEBUG// I used the bind function\n");
	}

	//Listen
	result = listen(listenSocket, 1);
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Listen function incorrect\n");
		return;
	}
	else
	{
		printf("DEBUG// I used the listen function\n");
	}

	fd_set masterSet;
	fd_set readySet;

	FD_ZERO(&masterSet);
	FD_SET(listenSocket, &masterSet);

	readySet = masterSet;

	char host[256];
	gethostname(host, sizeof(host));
	printf("Hostname: %s\n", host);

	addrinfo hints = {};
	addrinfo* addressResult = nullptr;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(host, NULL, &hints, &addressResult);

	for (addrinfo* ptr = addressResult; ptr != NULL; ptr = ptr->ai_next)
	{
		char ipString[INET6_ADDRSTRLEN];

		void* address;

		if (ptr->ai_family == AF_INET)
		{
			sockaddr_in* ipv4 = (sockaddr_in*)ptr->ai_addr;
			address = &(ipv4->sin_addr);
		}
		else
		{
			sockaddr_in6* ipv6 = (sockaddr_in6*)ptr->ai_addr;
			address = &(ipv6->sin6_addr);
		}

		inet_ntop(ptr->ai_family, address, ipString, sizeof(ipString));

		//printf("IP Address: %s\n", ipString);

		if (ptr->ai_family == AF_INET)
		{
			printf("IPv4 Address: %s\n", ipString);
		}
		else if (ptr->ai_family == AF_INET6)
		{
			printf("IPv6 Address: %s\n", ipString);
		}

	}

	freeaddrinfo(addressResult);


	int currentClients = 0;
	while (true)
	{
		readySet = masterSet;

		int socketCount = select(0, &readySet, nullptr, nullptr, nullptr);

		SOCKET clientSocket = INVALID_SOCKET;

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET currentSocket = readySet.fd_array[i];

			if (currentSocket == listenSocket)
			{
				if (currentClients >= capacity)
				{
					SOCKET temp = accept(listenSocket, NULL, NULL);
					if (temp != INVALID_SOCKET)
					{
						printf("Server full - rejecting client\n");
						closesocket(temp);
					}
					continue;
				}

				clientSocket = accept(listenSocket, NULL, NULL);

				if (clientSocket != INVALID_SOCKET)
				{
					FD_SET(clientSocket, &masterSet);
					currentClients++;



					std::string welcome =
						std::string("Welcome to the chat server. Please use ")
						+ commandChar +
						" before all commands. Use "
						+ commandChar +
						"help for command list.";

					TCPFraming::sendFrame(clientSocket, welcome.c_str(), welcome.size());

				}
			}
			else
			{
				uint8_t size = 0;

				int result = recv(currentSocket, (char*)&size, 1, 0);

				if (result <= 0)
				{
					closesocket(currentSocket);
					FD_CLR(currentSocket, &masterSet);
					printf("Client disconnected\n");
					currentClients--;
					continue;
				}

				//char buffer[256] = {};
				//prevent overflow of the buffer if the client sends a size larger than 256 when + '\0' for null terminator
				char buffer[257] = {};


				//result = recv(currentSocket, buffer, size, 0);
				result = TCPFraming::tcp_recv_whole(currentSocket, buffer, size);

				if (result <= 0)
				{
					closesocket(currentSocket);
					FD_CLR(currentSocket, &masterSet);
					printf("Client disconnected\n");
					currentClients--;
					continue;
				}

				buffer[size] = '\0';

				if (result > 0)
				{

					//printf("Message: %s\n", buffer); 
					MessageHandler::HandleCommand(currentSocket, listenSocket, masterSet, commandChar, buffer);

				}


			}
		}
	}


	printf("Waiting...\n\n");

	SOCKET ComSocket = accept(listenSocket, NULL, NULL);
	if (ComSocket == INVALID_SOCKET)
	{
		printf("DEBUG// Accept function incorrect\n");
		return;

	}
	else
	{
		printf("DEBUG// I used the accept function\n");
	}

	//Communication
	uint8_t size = 0;

	result = TCPFraming::tcp_recv_whole(ComSocket, (char*)&size, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("DEBUG// recv is incorrect\n");
		return;

	}
	else
	{
		printf("DEBUG// I used the recv function\n");
	}

	char* buffer = new char[size];

	result = TCPFraming::tcp_recv_whole(ComSocket, (char*)buffer, size);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int error = WSAGetLastError();
		printf("DEBUG// recv is incorrect\n");
		return;

	}
	else
	{
		printf("DEBUG// I used the recv function\n");
	}

	printf("DEBUG// I received a message from the client\n");

	printf("\n\n");
	printf(buffer);
	printf("\n\n");

	delete[] buffer;

	// close both sockets
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);

	shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);
}
