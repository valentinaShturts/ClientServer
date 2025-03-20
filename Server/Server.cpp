#include <iostream>
#include <winsock2.h>
using namespace std;

#pragma comment(lib,"ws2_32.lib") // библиотека winsock
#pragma warning(disable:4996) 

#define BUFLEN 512 // размер буфера
#define PORT 8888 // порт сервера

class UDPServer {
public:
    UDPServer() {
        // инициализация winsock
        cout << "initializing winsock...\n";
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cout << "initialization error: " << WSAGetLastError() << "\n";
            exit(0);
        }
        cout << "initialization complete.\n";

        // создание сокета
        if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
            cout << "failed to create socket: " << WSAGetLastError() << "\n";
            exit(EXIT_FAILURE);
        }
        cout << "the socket has been created.\n";

        // настройка структуры sockaddr_in
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(PORT);

        // привязка сокета
        if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
            cout << "binding error: " << WSAGetLastError() << "\n";
            exit(EXIT_FAILURE);
        }
        cout << "binding completed.\n";
    }

    ~UDPServer() {
        closesocket(server_socket);
        WSACleanup();
    }

    void start() {
        while (!exitRequested) {
            cout << "wait for data from the client...\n";
            char message[BUFLEN] = {};

            // получение данных (блокирующий вызов)
            int message_len;
            int slen = sizeof(sockaddr_in);
            if ((message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen)) == SOCKET_ERROR) {
                cout << "recvfrom() error: " << WSAGetLastError() << "\n";
                exit(0);
            }

            // вывод информации о клиенте и полученных данных
            cout << "received a package from" << inet_ntoa(client.sin_addr) << " " << ntohs(client.sin_port) << "\n";
            cout << "data: " << message << "\n";

            cout << "enter the answer ('exit' to exit): ";
            cin.getline(message, BUFLEN);

            // отправка ответа клиенту
            if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR) {
                cout << "sendto() error: " << WSAGetLastError() << "\n";
                exit(EXIT_FAILURE);
            }

            if (strcmp(message, "exit") == 0) {
                cout << "shutting down the server...\n";
                exitRequested = true;
                break;
            }
        }
    }

private:
    WSADATA wsa{};
    SOCKET server_socket = 0;
    sockaddr_in server{}, client{};
    bool exitRequested = false;
};

int main() {
    system("title UDP SERVER SIDE");
    setlocale(0, "");

    UDPServer udpServer;
    udpServer.start();
}