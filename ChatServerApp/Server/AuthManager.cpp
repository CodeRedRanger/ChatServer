#include "AuthManager.h"


namespace AuthManager
{
	//declare public variables in cpp file to avoid multiple definitions error, and to allow for encapsulation of user data within the AuthManager namespace
    std::unordered_map<std::string, std::string> users;
    std::unordered_map<SOCKET, std::string> loggedInUsers;

}

void AuthManager::AddUser(const std::string& username, const std::string& password)
{
	//creates an entry in users map with username as key and password as value, effectively registering the user
    AuthManager::users[username] = password;
}

bool AuthManager::UserExists(const std::string& username)
{
	//username is found before end of users map, returns true, otherwise false
    return AuthManager::users.find(username) != users.end();
}

bool AuthManager::CheckPassword(const std::string& username, const std::string& password)
{
    //finds the user in the users map, returns false if not found, otherwise compares stored password with provided password
    auto it = AuthManager::users.find(username);
    if (it == users.end())
        return false;
	return it->second == password;
}

void AuthManager::SetLoggedIn(SOCKET client, const std::string& username)
{
	//creates an entry in loggedInUsers map with client socket as key and username as value, effectively marking the user as logged in on that socket
    AuthManager::loggedInUsers[client] = username;
}


bool AuthManager::IsLoggedIn(SOCKET client)
{
	//checks if client socket is found before end of loggedInUsers map, returns true if found (indicating that a client is logged in for that socket), otherwise false
    return AuthManager::loggedInUsers.find(client) != AuthManager::loggedInUsers.end();
}

bool AuthManager::IsUserLoggedInAnywhere(const std::string& username)
{
	//checks if the provided username is associated with any client socket in the loggedInUsers map, returns true if found, otherwise false
    for (const auto& pair : loggedInUsers)
    {
        if (pair.second == username)
            return true;
    }
    return false;
}

AuthManager::LogoutResult AuthManager::logoutUser(SOCKET client)
{


    if (!AuthManager::IsLoggedIn(client))
    {
        return LogoutResult::NOT_LOGGEDIN;
    }

    //removes the entry for the client socket from the loggedInUsers map, effectively logging out the user associated with that socket
	//since you can only be logged in on one socket at a time, this will log out the user regardless of which socket they are logged in on
    loggedInUsers.erase(client);

    //need other return scenarios
	return LogoutResult::SUCCESS;
}


 AuthManager::RegisterResult AuthManager::registerUser(const std::string& username, const std::string& password, int capacity, SOCKET client)
 {
     if (AuthManager::IsLoggedIn(client))
     {
         return RegisterResult::MUST_LOGOUT_FIRST; 
	 }

     if (users.size() >= capacity)
     {
         return RegisterResult::SERVER_FULL; 
	 }

	 //nothing put in one or both fields
     if (username.empty() || password.empty())
     {
         return RegisterResult::MISSING_FIELDS;
     }


    // 1. check if username exists
    if (users.find(username) != users.end())
    {
       return RegisterResult::USER_TAKEN;
    }

    // 2. validate password length
    if (password.length() < 5)
    {
       return RegisterResult::PASSWORD_INVALID;
    }

    // 3. store user
    AuthManager::AddUser(username, password);       
    return RegisterResult::SUCCESS;
  }

 AuthManager::LoginResult AuthManager::loginUser(const std::string& username, const std::string& password, SOCKET client)
 {

     if (AuthManager::IsLoggedIn(client))
     {
         return LoginResult::MUST_LOGOUT_FIRST;
     }

     if (username.empty() || password.empty())
     {
         return LoginResult::MISSING_FIELDS;
     }

	 // check if user exists
     if (!AuthManager::UserExists(username))
     {
         return LoginResult::USER_DOES_NOT_EXIST;
      
     }
     
     // check if password is correct
     if (!AuthManager::CheckPassword(username, password))
     {
		 return LoginResult::PASSWORD_INVALID;
     }

	 // check if user is already logged in on any socket
     if (AuthManager::IsUserLoggedInAnywhere(username))
     {
         return LoginResult::ALREADY_LOGGEDIN;
       
     }

     // SUCCESS
     AuthManager::SetLoggedIn(client, username);
	 return LoginResult::SUCCESS;
 }

 std::vector<std::string> AuthManager::GetLoggedInUsers()
 {
     std::vector<std::string> userList;
     for (const auto& pair : loggedInUsers)
     {
         userList.push_back(pair.second);
     }
     return userList;
 }
