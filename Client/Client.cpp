
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
#include <shellapi.h> 
#include <tlhelp32.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment(lib, "Shell32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define SERVER_EXE L"C:\\Users\\Valentina\\source\\repos\\op\\x64\\Debug\\Server.exe"

#define PAUSE 0


bool IsServerRunning() 
{
    PROCESSENTRY32 pe32;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    do {
        if (wcscmp(pe32.szExeFile, SERVER_EXE) == 0) {
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return false;
}


bool StartServer() 
{
    if (IsServerRunning()) 
    {
        cout << "Server is already running!" << endl;
        return true;
    }

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpFile = SERVER_EXE;
    sei.nShow = SW_SHOWNORMAL;

    if (ShellExecuteEx(&sei)) {
        cout << "Server launched successfully!" << endl;
        return true;
    }
    else {
        cout << "Failed to launch server. Error: " << GetLastError() << endl;
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
    bool serverStarted = false;

    while (ConnectSocket == INVALID_SOCKET) {
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

        if (ConnectSocket == INVALID_SOCKET) {
            if (!serverStarted) {
                cout << "Server is not running. Starting server..." << endl;
                if (!StartServer()) {
                    cout << "Failed to start server!" << endl;
                    WSACleanup();
                    return 14;
                }
                serverStarted = true;
            }
            else {
                cout << "It is impossible to connect to the server. Check if the server process is running!" << endl;
                WSACleanup();
                return 15;
            }
        }
        else {
            cout << "Connection to the server was successful!" << endl;
        }
    }
    


    while (true)
    {
        char message[DEFAULT_BUFLEN];
        cout << "Enter a number (or 'exit' to quit): ";
        cin.getline(message, DEFAULT_BUFLEN);

        iResult = send(ConnectSocket, message, strlen(message), 0);

        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            WSACleanup();
            return 15;
        }

        char answer[DEFAULT_BUFLEN];

        iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);
        answer[iResult] = '\0';

        if (iResult > 0) {
            cout << "the server process sent a response: " << answer << "\n";
        }
        else 
        {
            break;
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