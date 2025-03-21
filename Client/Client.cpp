
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "27015"

#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 10


void DrawSmiley(int x, int y) 
{
    for (int i = 0; i < SCREEN_HEIGHT; i++) 
    {
        for (int j = 0; j < SCREEN_WIDTH; j++) 
        {
            if (i == y && j == x)
                cout << ":)"; 
            else
                cout << "  ";
        }
        cout << endl;
    }
}

int main(int argc, char** argv)
{
    system("title CLIENT SIDE");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        return 11;
    }

    

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const char* ip = "localhost";

    addrinfo* result = NULL;
    iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        WSACleanup();
        return 12;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        

        if (ConnectSocket == INVALID_SOCKET) {
            WSACleanup();
            return 13;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);
    
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "it is impossible to connect to the server. check if the server process is running!\n";
        WSACleanup();
        return 14;
    }
    else {

        cout << "connection to the server was successful!\n";
    }



    int x = 5, y = 5; 

    while (true) 
    {
        system("cls");
        DrawSmiley(x, y);

        char key = _getch();  
        if (key == 27) break; 

        switch (key) 
        {
        case 'w': case 'W': case 72: if (y > 0) y--; break;
        case 's': case 'S': case 80: if (y < SCREEN_HEIGHT - 1) y++; break;
        case 'a': case 'A': case 75: if (x > 0) x--; break;
        case 'd': case 'D': case 77: if (x < SCREEN_WIDTH - 1) x++; break;
        }

        char message[10];
        sprintf_s(message, "%03d %03d", x, y);
        send(ConnectSocket, message, (int)strlen(message), 0);
    }

    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) 
    {
        closesocket(ConnectSocket);
        WSACleanup();
        return 16;
    }

    closesocket(ConnectSocket);
    WSACleanup();

    cout << "the client process completes its work!\n";

    return 0;
}