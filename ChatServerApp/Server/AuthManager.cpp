#include "AuthManager.h"


namespace AuthManager
{
    std::unordered_map<std::string, std::string> users;
    std::unordered_map<SOCKET, std::string> loggedInUsers;

}

void AuthManager::AddUser(const std::string& username, const std::string& password)
{
    AuthManager::users[username] = password;
}

bool AuthManager::UserExists(const std::string& username)
{
    return AuthManager::users.find(username) != users.end();
}

bool AuthManager::CheckPassword(const std::string& username, const std::string& password)
{
    auto it = AuthManager::users.find(username);
    if (it == users.end())
        return false;
	return it->second == password;
}


void AuthManager::SetLoggedIn(SOCKET client, const std::string& username)
{
    AuthManager::loggedInUsers[client] = username;
}


bool AuthManager::IsLoggedIn(SOCKET client)
{
    return AuthManager::loggedInUsers.find(client) != AuthManager::loggedInUsers.end();
}

bool AuthManager::IsUserLoggedInAnywhere(const std::string& username)
{
    for (const auto& pair : loggedInUsers)
    {
        if (pair.second == username)
            return true;
    }
    return false;
}

void AuthManager::Logout(SOCKET client)
{
    loggedInUsers.erase(client);
}


 AuthManager::RegisterResult AuthManager::registerUser(const std::string& username, const std::string& password)
 {

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
       //users[username] = password;
        AuthManager::AddUser(username, password);       

        return RegisterResult::SUCCESS;
  }

 AuthManager::LoginResult AuthManager::loginUser(const std::string& username, const std::string& password, SOCKET client)
 {
     if (username.empty() || password.empty())
     {
         return LoginResult::MISSING_FIELDS;
     }

     if (!AuthManager::UserExists(username))
     {
         return LoginResult::USER_DOES_NOT_EXIST;
      
     }

     if (!AuthManager::CheckPassword(username, password))
     {
		 return LoginResult::PASSWORD_INVALID;
     }

     if (AuthManager::IsUserLoggedInAnywhere(username))
     {
         return LoginResult::ALREADY_LOGGEDIN;
       
     }

     // SUCCESS
     AuthManager::SetLoggedIn(client, username);
	 return LoginResult::SUCCESS;
 }

