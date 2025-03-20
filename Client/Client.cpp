#include <iostream>
#include <winsock2.h>
using namespace std;

#pragma comment(lib,"ws2_32.lib") // подключение библиотеки winsock
#pragma warning(disable:4996)

#define SERVER "127.0.0.1" // ip-адрес сервера (локальный хост)
#define BUFLEN 512 // максимальная длина ответа
#define PORT 8888 // порт для приема данных

class UDPClient {
public:
    UDPClient() {
        // инициализация winsock
        cout << "initializing winsock...\n";
        if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
            cout << "initialization error. error code:" << WSAGetLastError() << "\n";
            exit(EXIT_FAILURE);
        }
        cout << "initialization complete.\n";

        // создание сокета
        if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
            cout << "socket creation error. error code: " << WSAGetLastError() << "\n";
            exit(EXIT_FAILURE);
        }

        // настройка структуры адреса сервера
        memset((char*)&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.S_un.S_addr = inet_addr(SERVER);
    }

    ~UDPClient() {
        // закрытие сокета и очистка winsock
        closesocket(client_socket);
        WSACleanup();
    }

    void start() {
        while (true) {
            char message[BUFLEN];
            cout << "enter message: ";
            cin.getline(message, BUFLEN);

            // отправка сообщения серверу
            if (sendto(client_socket, message, strlen(message), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR) {
                cout << "sending error. error code: " << WSAGetLastError() << "\n";
                exit(EXIT_FAILURE);
            }
            cout << "wait for a response from the server...\n";

            // получение ответа от сервера
            char answer[BUFLEN] = {};
            int slen = sizeof(sockaddr_in);
            int answer_length;

            if ((answer_length = recvfrom(client_socket, answer, BUFLEN, 0, (sockaddr*)&server, &slen)) == SOCKET_ERROR) {
                cout << "data retrieval error. error code:" << WSAGetLastError() << "\n";
                exit(EXIT_FAILURE);
            }

            cout << "server response: " << answer << "\n";
        }
    }

private:
    WSADATA ws; // данные winsock
    SOCKET client_socket; // сокет клиента
    sockaddr_in server; // адрес сервера
};

int main() {
    system("title UDP CLIENT SIDE");
    setlocale(0, "");

    UDPClient udpClient;
    udpClient.start();
}
