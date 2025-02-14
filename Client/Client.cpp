
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
// #include <winsock2.h>
#include <ws2tcpip.h>
using namespace std;

// потрібно зв'язати з Ws2_32.lib, Mswsock.lib та Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 1000

int main(int argc, char** argv) // ім'я сервера при бажанні можна буде вказати через параметри командного рядка
{
    // я намагався максимально спростити запуск клієнтського додатку, тому кількість параметрів командного рядка не перевіряється!
    // перевірка параметрів якщо треба
    // if (argc != 2) {
    //    printf("usage: %s server-name\n", argv[0]);
    //    return 10;
    // }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    setlocale(0, "Ukrainian");
    system("title CLIENT SIDE");
    cout << "the client process is started!\n";
    Sleep(PAUSE);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // iніціалізація Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed with error: " << iResult << "\n";
        return 11;
    }
    else {
        cout << "connection to Winsock.dll was successful!\n";
        Sleep(PAUSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // отримання адреси сервера та порту
    const char* ip = "localhost"; // за замовчуванням, обидва додатки, і клієнт, і сервер, працюють на одній і тій самій машині

    // iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result); // раскоментувати, якщо потрібно буде зчитувати ім'я сервера з командного рядка
    addrinfo* result = NULL;
    iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << "\n";
        WSACleanup();
        return 12;
    }
    else {
        cout << "obtaining the client address and port was successful!\n";
        Sleep(PAUSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // спроба підключення до адреси до успіху
    SOCKET ConnectSocket = INVALID_SOCKET;

    for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) { // серверів може бути кілька, тому не завадить цикл

        // створення SOCKET для підключення до сервера
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
            cout << "socket failed to create with error: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 13;
        }

        // підключення до сервера
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        cout << "creation of a socket on the client was successful!\n";
        Sleep(PAUSE);

        break;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "it is impossible to connect to the server. check if the server process is running!\n";
        WSACleanup();
        return 14;
    }
    else {
        cout << "connection to the server was successful!\n";
        Sleep(PAUSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // відправка початкового буфера
    const char* message = "hello from client!";
    iResult = send(ConnectSocket, message, (int)strlen(message), 0);
    if (iResult == SOCKET_ERROR) {
        cout << "send failed with error: " << WSAGetLastError() << "\n";
        closesocket(ConnectSocket);
        WSACleanup();
        return 15;
    }
    else {
        cout << "data has been successfully sent to the server: " << message << "\n";
        cout << "bytes sent from the client: " << iResult << "\n";
        Sleep(PAUSE);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // закриття з'єднання, оскільки більше даних не буде відправлено
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed with error: " << WSAGetLastError() << "\n";
        closesocket(ConnectSocket);
        WSACleanup();
        return 16;
    }
    else {
        cout << "the client process initiates closing the connection with the server.\n";
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // отримання даних до закриття з'єднання зі сторони сервера
    char answer[DEFAULT_BUFLEN];

    do {

        iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);
        answer[iResult] = '\0';

        if (iResult > 0) {
            cout << "the server process sent a response: " << answer << "\n";
            cout << "bytes received: " << iResult << "\n";
        }
        else if (iResult == 0)
            cout << "the connection to the server is closed.\n";
        else
            cout << "recv failed with error: " << WSAGetLastError() << "\n";

    } while (iResult > 0);

    // очистка
    closesocket(ConnectSocket);
    WSACleanup();

    cout << "the client process completes its work!\n";

    return 0;
}