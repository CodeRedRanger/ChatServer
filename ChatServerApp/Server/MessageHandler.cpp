#include "MessageHandler.h"

void MessageHandler::HandleLogin(SOCKET client, const std::string& msg)
{

	std::istringstream iss(msg);

	std::string command;
	std::string username;
	std::string password;

	iss >> command >> username >> password;

	if (!AuthManager::UserExists(username))
	{
		std::string noUser = "User does not exist.";
		TCPFraming::sendFrame(client, noUser.c_str(), noUser.size());
		return;
	}

	if (!AuthManager::CheckPassword(username, password))
	{
		std::string invalidPassword = "Invalid password.";
		TCPFraming::sendFrame(client, invalidPassword.c_str(), invalidPassword.size());
		return;
	}

	if (AuthManager::IsUserLoggedInAnywhere(username))
	{
		std::string alreadyLoggedIn = "That user is already logged in.";
		TCPFraming::sendFrame(client, alreadyLoggedIn.c_str(), alreadyLoggedIn.size());
		return;
	}
	
	// SUCCESS
	AuthManager::SetLoggedIn(client, username);
	std::string successMsg = "Login successful";
	TCPFraming::sendFrame(client, successMsg.c_str(), successMsg.size());
	
}


void MessageHandler::HandleRegister(SOCKET client, const std::string& msg)
{
	std::istringstream iss(msg);

	std::string command;
	std::string username;
	std::string password;

	iss >> command >> username >> password;


	auto result = AuthManager::registerUser(username, password);


	switch (result)
	{
	case AuthManager::RegisterResult::SUCCESS:
	{
		const char* msg = "Username registration succeeded. Please log in.";

		//create wrapper for below so code doesn't have to be repeated below x 2
		std::string Msg = msg;
		TCPFraming::sendFrame(client, Msg.c_str(), (uint16_t)Msg.size());
		break;
	}

	case AuthManager::RegisterResult::USER_TAKEN:
	{
		const char* msg = "username taken";
		std::string Msg = msg;
		TCPFraming::sendFrame(client, Msg.c_str(), (uint16_t)Msg.size());
		break;
	}
	case AuthManager::RegisterResult::PASSWORD_INVALID:
	{
		const char* msg = "password invalid (must be at least 5 characters)";
		std::string Msg = msg;
		TCPFraming::sendFrame(client, Msg.c_str(), (uint16_t)Msg.size());
		break;
	}
	}
}


void MessageHandler::HandleCommand(SOCKET client, SOCKET listenSocket, fd_set& masterSet, const char cmdChar, const char* msg)
{

	if (!AuthManager::IsLoggedIn(client))
	{
		// allow only register/login
		if (strncmp(msg, "$register", 9) == 0 || strncmp(msg, "$login", 6) == 0)
		{
			//continue
		}
		else
		{
			std::string mustLogin = "You must register or login first. Use " + std::string(1, cmdChar) + "register or " + std::string(1, cmdChar) + "login.";
			TCPFraming::sendFrame(client, mustLogin.c_str(), mustLogin.size());
			return;
		}
	}

	if (!msg || msg[0] != cmdChar)
	{
	
		for (int i = 0; i < masterSet.fd_count; i++)
		{
			SOCKET s = masterSet.fd_array[i];

			if (s == listenSocket || s == client)
				continue;

			if (!AuthManager::IsLoggedIn(s))
				continue; // block non-logged-in users from receiving chat

			std::string Msg = msg; 
			TCPFraming::sendFrame(s, Msg.c_str(), (uint16_t)Msg.size());
		
		}


		/*
			broadcastMessage(msg);
		*/

		//add this to messages that are broadcast to clients instead of just printing on server console
		printf("Public message: %s\n", msg);
		return;
	}

	std::string registerCmd = std::string(1, cmdChar) + "register";
	std::string loginCmd = std::string(1, cmdChar) + "login";
	std::string getlistCmd = std::string(1, cmdChar) + "getlist";

	if (strncmp(msg, registerCmd.c_str(), registerCmd.length()) == 0)
	{
		printf("REGISTER COMMAND RECEIVED\n");
		MessageHandler::HandleRegister(client, msg);
	}
	else if (strncmp(msg, loginCmd.c_str(), loginCmd.length()) == 0)
	{
		printf("LOGIN COMMAND RECEIVED\n");
		MessageHandler::HandleLogin(client, msg);
	}
	else if (strncmp(msg, getlistCmd.c_str(), getlistCmd.length()) == 0)
	{
		printf("GETLIST COMMAND\n");
		//need to print all commands
	}
	else
	{
		printf("UNKNOWN COMMAND\n");
	}
}




