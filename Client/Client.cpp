
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 0


bool SendMessageToServer(SOCKET ConnectSocket, const string& message) 
{
    int iResult = send(ConnectSocket, message.c_str(), message.length(), 0);
    if (iResult == SOCKET_ERROR) 
    {
        cout << "Error sending data!\n";
        return false;
    }

    char answer[DEFAULT_BUFLEN];
    iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);
    if (iResult > 0) 
    {
        answer[iResult] = '\0';
        cout << "Server response: " << answer << endl;
        return true;
    }
    else 
    {
        cout << "The connection with the server is broken!\n";
        return false;
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



    while (true) 
    {
        cout << "\nMenu:\n";
        cout << "1 - Log in\n";
        cout << "2 - Sign in\n";
        cout << "3 - Exit\n";
        cout << "Enter option: ";

        int choice;
        cin >> choice;
        cin.ignore(); 

        string username, password;
        if (choice == 1 || choice == 2) {
            cout << "Enter login: ";
            getline(cin, username);
            cout << "Enter password: ";
            getline(cin, password);

            string message = (choice == 1 ? "LOGIN " : "REGISTER ") + username + " " + password;
            if (!SendMessageToServer(ConnectSocket, message)) {
                break;
            }
        }
        else if (choice == 3) {
            cout << "Exit...\n";
            break;
        }
        else {
            cout << "Uknown command!\n";
        }
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