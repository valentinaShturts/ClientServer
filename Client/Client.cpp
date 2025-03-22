#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096
#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;
sockaddr_in server_addr;    // адрес сервера
int server_addr_len = sizeof(server_addr);
string nickname;            // ник клиента
int colorCode;


string extractMessage(const string& msg)
{
    size_t pos = msg.find('-');
    if (pos != string::npos)
    {
        return msg.substr(0, pos);  
    }
    return msg;  
}

int extractColor(const string& msg)
{
    size_t pos = msg.find('-');
    if (pos != string::npos) {
        try {
            return stoi(msg.substr(pos + 1));
        }
        catch (const std::invalid_argument& e) {
            printf("Invalid color value in message: %s\n", msg.c_str());
            return -1;  
        }
    }
    return -1;  
}


void SetConsoleColor(int colorCode)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorCode);
}

string getCurrentTime()
{
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &ltm);
    return string(timeStr);
}

DWORD WINAPI Sender(void* param)
{
    // отправка сообщений с ником
    while (true) {
        char query[DEFAULT_BUFLEN];
        cin.getline(query, DEFAULT_BUFLEN);
        string message = "[" + getCurrentTime() + "]" + nickname + ": " + query + "-" + to_string(colorCode);
        int send_result = sendto(client_socket, message.c_str(), message.size(), 0, (sockaddr*)&server_addr, server_addr_len);
        if (send_result == SOCKET_ERROR) {
            printf("Sendto error with code: %d\n", WSAGetLastError());
        }
    }
}

DWORD WINAPI Receiver(void* param)
{
    // прием сообщений от сервера
    while (true) {
        char response[DEFAULT_BUFLEN];
        sockaddr_in from_addr;
        int from_len = sizeof(from_addr);
        int result = recvfrom(client_socket, response, DEFAULT_BUFLEN, 0, (sockaddr*)&from_addr, &from_len);
        if (result > 0) {
            response[result] = '\0';
            string received_msg = response;
            string message_text = extractMessage(received_msg);
            int color = extractColor(received_msg);

            SetConsoleColor(color);
            cout << message_text << endl;
            SetConsoleColor(7);
        }
        else if (result == SOCKET_ERROR) {
            printf("Recvfrom error with code: %d\n", WSAGetLastError());
        }
    }
}

BOOL ExitHandler(DWORD whatHappening)
{
    string message;
    // обработчик закрытия консоли
    switch (whatHappening)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        cout << "Shutdown...\n";
        Sleep(1000);
        message = nickname + " left the chat :(" + "-" + to_string(colorCode);
        sendto(client_socket, message.c_str(), message.size(), 0, (sockaddr*)&server_addr, server_addr_len);
        closesocket(client_socket);
        WSACleanup();
        exit(0);
        return TRUE;
    default:
        return FALSE;
    }
}


int main()
{
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ExitHandler, TRUE);

    setlocale(0, "");
    system("title UDP CLIENT SIDE");

    // запрос ника у пользователя
    cout << "Enter your nickname: ";
    getline(cin, nickname);
    cout << "Enter the color code (1-15): ";
    cin >> colorCode;
    cin.ignore();
    if (colorCode < 1 || colorCode>15) colorCode = 7;

    // инициализация Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Winsock initialization error with code: %d\n", iResult);
        return 1;
    }

    // создание UDP-сокета
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket == INVALID_SOCKET) {
        printf("Error creating socket with code: %ld\n", WSAGetLastError());
        WSACleanup();
        return 2;
    }

    // привязка сокета к локальному адресу с автоматическим выбором порта
    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = 0;

    if (bind(client_socket, (sockaddr*)&client_addr, sizeof(client_addr)) == SOCKET_ERROR) {
        printf("Binding error with code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 3;
    }

    // получение назначенного порта
    int client_addr_len = sizeof(client_addr);
    if (getsockname(client_socket, (sockaddr*)&client_addr, &client_addr_len) == SOCKET_ERROR) {
        printf("getsockname error with code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 4;
    }

    // настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("UDP Client is running. Nick: %s. Bound to port %d. Send to %s:%s\n", nickname.c_str(), ntohs(client_addr.sin_port), SERVER_IP, DEFAULT_PORT);

    // отправка первого сообщения для регистрации и получения истории

    string initial_message = nickname + " joined the chat" + "-" + to_string(colorCode);
    sendto(client_socket, initial_message.c_str(), initial_message.size(), 0, (sockaddr*)&server_addr, server_addr_len);

    // запуск потоков для отправки и получения
    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
    closesocket(client_socket);
    WSACleanup();
    return 0;
}