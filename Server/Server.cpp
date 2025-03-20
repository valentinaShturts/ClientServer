#define WIN32_LEAN_AND_MEAN 

#include <iostream>
#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <vector>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015" 
#define PAUSE 0


struct User 
{
    string username;
    string password;
};


string LoginUser(vector<User>& users, const string& username, const string& password) 
{
    for (const auto& user : users) 
    {
        if (user.username == username) 
        {
            if (user.password == password) 
            {
                return "Welcome";
            }
            else {
                return "ERROR: Incorrect password";
            }
        }
    }
    return "ERROR: Invalid username";
}

string RegisterUser(vector<User>& users, const string& username, const string& password)
{
    for (const auto& user : users) 
    {
        if (user.username == username) 
        {
            return "ERROR: Username already exists";
        }
    }
    users.push_back({ username, password });
    return "Welcome"; 
}


int main()
{
    system("title SERVER SIDE");

    vector<User> users;
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);


    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;


    struct addrinfo* result = NULL;
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        return 2;
    }


    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();

        return 3;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 4;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        closesocket(ListenSocket);
        WSACleanup();
        return 5;
    }
    else {
        cout << "listening to information from the client begins. please launch the client! (client.exe)\n";
    }

    SOCKET ClientSocket = INVALID_SOCKET;
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        closesocket(ListenSocket);
        WSACleanup();
        return 6;
    }
    else {
        cout << "the connection with the client program has been established successfully!\n";
    }

    closesocket(ListenSocket);
    char recvbuf[DEFAULT_BUFLEN];
    while (true) {
        ZeroMemory(recvbuf, DEFAULT_BUFLEN);
        iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult <= 0) break;

        string request(recvbuf);
        string response;

        if (request.rfind("REGISTER ", 0) == 0) {
            size_t space = request.find(' ', 9);
            if (space != string::npos) {
                string username = request.substr(9, space - 9);
                string password = request.substr(space + 1);
                response = RegisterUser(users, username, password);
            }
        }
        else if (request.rfind("LOGIN ", 0) == 0) {
            size_t space = request.find(' ', 6);
            if (space != string::npos) {
                string username = request.substr(6, space - 6);
                string password = request.substr(space + 1);
                response = LoginUser(users, username, password);
            }
        }
        else {
            response = "ERROR: Unknown command";
        }

        send(ClientSocket, response.c_str(), response.length(), 0);
    }

    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        closesocket(ClientSocket);
        WSACleanup();
        return 9;
    }
    else {
        cout << "server process stops working! to new launches! :)\n";
    }

    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}