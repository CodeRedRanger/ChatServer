#include "MessageHandler.h"

void MessageHandler::HandleLogin(SOCKET client, const std::string& msg)
{

	std::istringstream iss(msg);

	std::string command;
	std::string username;
	std::string password;

	iss >> command >> username >> password;

	auto result = AuthManager::loginUser(username, password, client);

	if (result == AuthManager::LoginResult::MISSING_FIELDS)
	{
		std::string missingFields = "Missing fields. Please provide both username and password.";
		TCPFraming::sendFrame(client, missingFields.c_str(), missingFields.size());
		return;
	}

	if (result == AuthManager::LoginResult::USER_DOES_NOT_EXIST)
	{
		std::string noUser = "User does not exist.";
		TCPFraming::sendFrame(client, noUser.c_str(), noUser.size());
		return;
	}

	if (result == AuthManager::LoginResult::PASSWORD_INVALID)
	{
		std::string invalidPassword = "Invalid password.";
		TCPFraming::sendFrame(client, invalidPassword.c_str(), invalidPassword.size());
		return;
	}

	if (result == AuthManager::LoginResult::ALREADY_LOGGEDIN)
	{
		std::string alreadyLoggedIn = "That user is already logged in.";
		TCPFraming::sendFrame(client, alreadyLoggedIn.c_str(), alreadyLoggedIn.size());
		return;
	}
	
	// SUCCESS
	if (result == AuthManager::LoginResult::SUCCESS)
	{
		std::string successMsg = "Login successful";
		TCPFraming::sendFrame(client, successMsg.c_str(), successMsg.size());
	}
	
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

		//create wrapper for below so code doesn't have to be repeated below x 2
		std::string msg = "Username registration succeeded. Please log in."; 
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}

	case AuthManager::RegisterResult::MISSING_FIELDS:
	{

		//create wrapper for below so code doesn't have to be repeated below x 2
		std::string msg = "Missing fields. Please provide both username and password.";
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}

	case AuthManager::RegisterResult::USER_TAKEN:
	{
		std::string msg = "Username taken.";
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}
	case AuthManager::RegisterResult::PASSWORD_INVALID:
	{
		std::string msg = "Password invalid (must be at least 5 characters)";
		TCPFraming::sendFrame(client, msg.c_str(), (uint16_t)msg.size());
		break;
	}
	}
}

void MessageHandler::HandleGetHelp(SOCKET client, const char cmdChar)
{
	std::string commandList = "To register: ";
	commandList += std::string(1, cmdChar) + "register username password\n";
	commandList += "To login: ";
	commandList += std::string(1, cmdChar) + "login username password\n";
	commandList += "To get help: ";
	commandList += std::string(1, cmdChar) + "help\n";
	TCPFraming::sendFrame(client, commandList.c_str(), (uint16_t)commandList.size());
}

void MessageHandler::HandleCommand(SOCKET client, SOCKET listenSocket, fd_set& masterSet, const char cmdChar, const char* msg)
{

	/*
	if (!AuthManager::IsLoggedIn(client))
	{
		// allow only register/login
		if (strncmp(msg, (std::string(1, cmdChar) + "register").c_str(), 9) == 0 || strncmp(msg, (std::string(1, cmdChar) + "login").c_str(), 6) == 0
			|| strncmp(msg, (std::string(1, cmdChar) + "help").c_str(), 5) == 0)
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
	*/
	
	if (!AuthManager::IsLoggedIn(client))
	{
		std::string cmd(msg);
		cmd.erase(std::remove(cmd.begin(), cmd.end(), '\r'), cmd.end());
		cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n'), cmd.end());

		bool allowed =
			cmd.rfind(std::string(1, cmdChar) + "register", 0) == 0 ||
			cmd.rfind(std::string(1, cmdChar) + "login", 0) == 0 ||
			cmd.rfind(std::string(1, cmdChar) + "help", 0) == 0;


		if (!allowed)
		{
			std::string mustLogin =
				"You must register or login first. Use "
				+ std::string(1, cmdChar)
				+ "register or "
				+ std::string(1, cmdChar)
				+ "login.";

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

			TCPFraming::sendFrame(s, msg, (uint16_t)strlen(msg));
		
		}


		//HERE HERE HERE
		//do I need below and should I move print to broadcast message function? 

		/*
			broadcastMessage(msg);
		*/

		//add this to messages that are broadcast to clients instead of just printing on server console
		printf("Public message: %s\n", msg);
		return;
	}

	std::string registerCmd = std::string(1, cmdChar) + "register";
	std::string loginCmd = std::string(1, cmdChar) + "login";
	std::string helpCmd = std::string(1, cmdChar) + "help";

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
	else if (strncmp(msg, helpCmd.c_str(), helpCmd.length()) == 0)
	{
		printf("GETHELP COMMAND\n");
		MessageHandler::HandleGetHelp(client, cmdChar);
	}
	else
	{
		printf("UNKNOWN COMMAND\n");
	}
}




