#define WIN32_LEAN_AND_MEAN 

#include <iostream>
#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
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

        if (iResult > 0) {
            message[iResult] = '\0';
            cout << "the client process sent a message: " << message << "\n";

            if (strcmp(message, "exit") == 0) break;

            int receivedNumber = atoi(message);  
            receivedNumber++; 

            char answer[DEFAULT_BUFLEN];
            _itoa_s(receivedNumber, answer, 10);  

            cout << "the server process sends a response: " << answer << "\n";

            int iSendResult = send(ClientSocket, answer, (int)strlen(answer), 0);

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
        }
        else {
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