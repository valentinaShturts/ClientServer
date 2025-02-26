#define WIN32_LEAN_AND_MEAN 

#include <iostream>
#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015" 
#define PAUSE 0

int main()
{
    system("title SERVER SIDE");

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

    do {

        char message[DEFAULT_BUFLEN];

        iResult = recv(ClientSocket, message, DEFAULT_BUFLEN, 0); 
        message[iResult] = '\0';

        if (iResult > 0) {
            cout << "the client process sent a message: " << message << "\n";
            const char* answer;

            if (strcmp(message, "hello") == 0) answer = "yo, client!";
            else if (strcmp(message, "how is it going") == 0) answer = "fine, client!";
            else if (strcmp(message, "is the pizza delicious?") == 0) answer = "sure is, client!";
            else if (strcmp(message, "goodbye") == 0) answer = "bye, client!";
            else
            {
                answer = "what are you talking about, client?";
            }
            cout << "the server process sends a response: " << answer << "\n";

            int iSendResult = send(ClientSocket, answer, strlen(answer), 0); 

            if (iSendResult == SOCKET_ERROR) {
                closesocket(ClientSocket);
                WSACleanup();
                return 7;
            }
            else {
                //cout << "bytes sent: " << iSendResult << "\n";
            }
        }
        else if (iResult == 0) {
            cout << "the connection is closed...\n";
            Sleep(PAUSE);
        }
        else {
            cout << "recv failed with error: " << WSAGetLastError() << "\n";
            cout << "oops, receipt (recv) of the corresponding message did not occur ((\n";
            closesocket(ClientSocket);
            WSACleanup();
            return 8;
        }

    } while (iResult > 0);

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