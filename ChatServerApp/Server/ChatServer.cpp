#include "ChatServer.h"


/*

//Put in message handler
2. Routing
if (command == "send")     → broadcast logic

3. Public messages
Anything without command character at the start of message
broadcast public chat

//command parsing layer (move to messageHandler)
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

int Server::InitListeningSocket(uint16_t port, SOCKET& listenSocket, int capacity)
{
	
	//create a socket
	//AF_INET = IPv4, SOCK_STREAM = TCP, IPPROTO_TCP = TCP protocol
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("DEBUG// Socket function incorrect\n");
		return SETUP_ERROR;
	}
	else
	{
		printf("DEBUG// I used the socket function\n");
	}


	//Prepare address struct, sets family to AF_INET (IPv4), address to INADDR_ANY (binds to all available interfaces)
	//passes port in network byte order (htons = host to network short)
	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port); //31337

	//Binds socket to the specified address and port, if it fails, closes the socket and returns an error code
	//Failure might occur if port in use, admin restrictions or invalid address
	//also casts the sockaddr_in struct to a generic sockaddr pointer as required by the bind function, and passes the size of the sockaddr_in struct
	//so that bind knows how much memory to read for the address information
	int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Bind function incorrect\n");
		closesocket(listenSocket);
		return BIND_ERROR;
	}
	else
	{
		printf("DEBUG// I used the bind function\n");
	}

	//Listen
	//turns the socket into a listening socket, allowing it to accept incoming connection requests. 
	// The second parameter (1) specifies the maximum length of the queue of pending connections. 
	// If it fails, closes the socket and returns an error code. 
	// Reasons this could fail include if the socket is not properly bound, if the socket is already listening, 
	// or if there are insufficient system resources to listen on the socket.
	result = listen(listenSocket, capacity);
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Listen function incorrect\n");
		closesocket(listenSocket);
		return CONNECT_ERROR;
	}
	else
	{
		printf("DEBUG// I used the listen function\n");
	}


	return SUCCESS;
}

void Server::DisconnectClient(SOCKET client, fd_set& masterSet, int& currentClients)
{
	Stop(client);

	//select no longer needs to monitor this socket for activity, so it's removed from the master set of sockets
	FD_CLR(client, &masterSet);

	if (currentClients > 0)
	{
		currentClients--;
	}

	printf("Client disconnected\n");
}

void Server::Stop(SOCKET socket)
{
	if (socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}
}

void Server::ServerCode(void)
{

	uint16_t port;
	int capacity;
	char commandChar;

	std::cout << "Port: ";
	std::cin >> port;

	std::cout << "Capacity: ";
	std::cin >> capacity;

	std::cout << "Command character: ";
	std::cin >> commandChar;

	//declare sockets for functions
	SOCKET listenSocket = INVALID_SOCKET;

	int result = Server::InitListeningSocket(port, listenSocket, capacity);


	//handle errors with listening socket setup, bind and listen
	if (result != SUCCESS)
	{
		switch (result)
		{
		case SETUP_ERROR:
			MessageBoxA(NULL, "Socket setup failed.", "Server Error", MB_OK | MB_ICONERROR);
			break;

		case BIND_ERROR:
			MessageBoxA(NULL, "Bind failed. Port may already be in use.", "Server Error", MB_OK | MB_ICONERROR);
			break;

		case CONNECT_ERROR:
			MessageBoxA(NULL, "Listen failed. Unable to accept connections.", "Server Error", MB_OK | MB_ICONERROR);
			break;

		default:
			MessageBoxA(NULL, "Unknown server error.", "Server Error", MB_OK | MB_ICONERROR);
			break;
		}

		return;
	}

	
	fd_set masterSet;
	fd_set readySet;

	//clear set
	FD_ZERO(&masterSet);
	//adds listening socket to the master set
	FD_SET(listenSocket, &masterSet);

	//initially, the ready set is the same as the master set, but it will be modified by select to indicate which sockets are ready for reading
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

		//make binary IP address human-readable
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

	//freeaddrinfo is called to free the memory allocated by getaddrinfo for the linked list of address information
	freeaddrinfo(addressResult);


	int currentClients = 0;
	while (true)
	{
		readySet = masterSet;

		//blocking. waits for activity on any socket in the ready set, or new connections.
		int socketCount = select(0, &readySet, nullptr, nullptr, nullptr);

		SOCKET clientSocket = INVALID_SOCKET;

		//loop through ready sockets and check if activity is on listening socket (new connection) or client socket (incoming message)
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET currentSocket = readySet.fd_array[i];

			if (currentSocket == listenSocket)
			{
				//check for server full, accept then close if server full
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

				//accept new connection, add to master set and increment client count
				clientSocket = accept(listenSocket, NULL, NULL);

				if (clientSocket != INVALID_SOCKET)
				{
					FD_SET(clientSocket, &masterSet);
					currentClients++;

					//send welcome message to client
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
				//read message from client, handle disconnects and shutdowns, pass valid messages to message handler
				char buffer[256] = {};

				int result = TCPFraming::readFrame(currentSocket, buffer, sizeof(buffer));


				if (result == PARAMETER_ERROR)
				{
					printf("Invalid frame received\n");
					continue;
				}


				//Handle graceful shutdowns and connection errors
				if (result == SHUTDOWN || result == DISCONNECT)
				{
					DisconnectClient(currentSocket, masterSet, currentClients);
					AuthManager::Logout(currentSocket);
					continue;
				}

				MessageHandler::HandleCommand(currentSocket, listenSocket, masterSet, commandChar, buffer);

			}
		}
	}


}

