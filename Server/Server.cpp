#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

SOCKET server_socket;

vector<string> history;         // история сообщений
vector<sockaddr_in> clients;    // список адресов клиентов



void SetConsoleColor(int colorCode)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorCode);
}

int checkLeftChat(const string& msg) 
{
    if (msg.find("left the chat") != string::npos) 
    {
        return 1; 
    }
    return 0;
}

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

int main() {
    setlocale(0, "");
    system("title UDP SERVER SIDE");

    puts("Starting the server... DONE.");
    WSADATA wsa;
    // инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Initialization error. Error code: %d", WSAGetLastError());
        return 1;
    }

    // создание UDP-сокета
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        printf("Failed to create socket: %d", WSAGetLastError());
        return 2;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // привязка сокета к адресу и порту
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Binding error with code: %d", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 3;
    }

    puts("The server is waiting for incoming UDP messages...\nStart one or more clients.");

    fd_set readfds;
    char client_message[DEFAULT_BUFLEN];
    sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    while (true) {
        // очистка набора сокетов
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        // ожидание активности на сокете
        if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
            printf("Select function error with code: %d", WSAGetLastError());
            closesocket(server_socket);
            WSACleanup();
            return 4;
        }

        if (FD_ISSET(server_socket, &readfds)) {
            // получение сообщения от клиента
            int client_message_length = recvfrom(server_socket, client_message, DEFAULT_BUFLEN, 0, (sockaddr*)&client_addr, &addrlen);
            if (client_message_length < 0) {
                printf("recvfrom error with code: %d", WSAGetLastError());
                continue;
            }
            client_message[client_message_length] = '\0';
            string received_msg = client_message;
            string message_text = extractMessage(received_msg);
            int color = extractColor(received_msg);

            string formatted_message = message_text;
            printf("Received from %s:%d %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), formatted_message.c_str());

            // проверка, есть ли клиент в списке
            bool client_exists = false;
            for (const auto& addr : clients) {
                if (addr.sin_addr.s_addr == client_addr.sin_addr.s_addr && addr.sin_port == client_addr.sin_port) {
                    client_exists = true;
                    break;
                }
            }

            // добавление нового клиента и отправка истории
            if (!client_exists && clients.size() < MAX_CLIENTS) {
                clients.push_back(client_addr);
                printf("A new client has been added. Total clients: %zu\n", clients.size());

                for (const auto& msg : history) {
                    sendto(server_socket, msg.c_str(), msg.size(), 0, (sockaddr*)&client_addr, addrlen);
                }
            }

            // обработка отключения клиента
            if (checkLeftChat(formatted_message)) 
            {
                printf("Client with ip: %s, port: %d disconnected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                for (auto it = clients.begin(); it != clients.end(); ++it) {
                    if (it->sin_addr.s_addr == client_addr.sin_addr.s_addr && it->sin_port == client_addr.sin_port) {
                        clients.erase(it);
                        break;
                    }
                }
                for (const auto& addr : clients) 
                {
                    if (!(addr.sin_addr.s_addr == client_addr.sin_addr.s_addr && addr.sin_port == client_addr.sin_port)) {
                        sendto(server_socket, received_msg.c_str(), received_msg.size(), 0, (sockaddr*)&addr, sizeof(addr));
                    }
                }
            }
            else {
                // сохранение сообщения в историю
                history.push_back(received_msg);

                // рассылка сообщения всем клиентам, кроме отправителя
                for (const auto& addr : clients) {
                    if (!(addr.sin_addr.s_addr == client_addr.sin_addr.s_addr && addr.sin_port == client_addr.sin_port)) {
                        sendto(server_socket, received_msg.c_str(), received_msg.size(), 0, (sockaddr*)&addr, sizeof(addr));
                    }
                }
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}