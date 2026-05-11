#pragma once
#include <string>
#include <unordered_map>
#include <ws2tcpip.h>
#include "../User.h"


namespace AuthManager
{

    enum class RegisterResult
    {
        SUCCESS,
		SERVER_FULL,
        MISSING_FIELDS,
        USER_TAKEN,
        PASSWORD_INVALID,
        MUST_LOGOUT_FIRST
       
    };

    enum class LoginResult
    {
        SUCCESS,
        MISSING_FIELDS,
        USER_DOES_NOT_EXIST,
        PASSWORD_INVALID,
        ALREADY_LOGGEDIN,
		MUST_LOGOUT_FIRST
    };

    enum class LogoutResult
    {
        SUCCESS,
		NOT_LOGGEDIN
    };

    RegisterResult registerUser(const std::string& username, const std::string& password, int capacity, SOCKET client);
	LoginResult loginUser(const std::string& username, const std::string& password, SOCKET client);
    void AddUser(const std::string& username, const std::string& password);
    bool UserExists(const std::string& username); 
    LogoutResult logoutUser(SOCKET client);

    bool CheckPassword(const std::string& username, const std::string& password);
    void SetLoggedIn(SOCKET client, const std::string& username);
    bool IsLoggedIn(SOCKET client);
    bool IsUserLoggedInAnywhere(const std::string& username); 

    std::vector<std::string> GetLoggedInUsers();


    extern std::unordered_map<std::string, std::string> users;
    extern std::unordered_map<SOCKET, std::string> loggedInUsers;
    extern size_t maxUsers;

};

