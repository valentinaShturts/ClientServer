#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 1

// спроба підключитися до адреси, поки не вдасться
SOCKET ConnectSocket = INVALID_SOCKET;

DWORD WINAPI Sender(void* param)
{
	while (true)
	{
		// відправити початковий буфер
		char* message = new char[200];
		cout << "Please enter your message for server: ";
		cin.getline(message, 199);

		int iResult = send(ConnectSocket, message, (int)strlen(message), 0);
		if (iResult == SOCKET_ERROR) {
			cout << "error of sending: " << WSAGetLastError() << "\n";
			closesocket(ConnectSocket);
			WSACleanup();
			return 15;
		}
		else {
			cout << "data successfully sent to the server: " << message << "\n";
			// cout << "байтів з клієнта надіслано: " << iResult << "\n";
			delete[] message;
			Sleep(PAUSE);
		}
	}
	return 0;
}

DWORD WINAPI Receiver(void* param)
{
	while (true)
	{
		// приймати дані, поки співрозмовник не закриє з'єднання
		char answer[DEFAULT_BUFLEN];

		int iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);
		answer[iResult] = '\0';

		if (iResult > 0) {
			cout << "\nThe server process sent the message: " << answer << "\n";
			// cout << "байтів отримано: " << iResult << "\n";
		}
		else if (iResult == 0)
			cout << "the connection to the server is closed.\n";
		else
			cout << "reception error: " << WSAGetLastError() << "\n";
	}
	return 0;
}

int main()
{
	SetConsoleOutputCP(1251);
	system("title CLIENT SIDE");


	// ініціалізація Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "failed to initialize Winsock: " << iResult << "\n";
		return 11;
	}
	else {
		// cout << "підключення Winsock.dll пройшло успішно!\n";
		Sleep(PAUSE);
	}

	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	const char* ip = "localhost"; // за замовчуванням, обидва додатки, і клієнт, і сервер, працюють на одній машині

	addrinfo* result = NULL;
	iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

	if (iResult != 0) {
		cout << "getaddrinfo failed: " << iResult << "\n";
		WSACleanup();
		return 12;
	}
	else {
		// cout << "отримання адреси і порту клієнта пройшло успішно!\n";
		Sleep(PAUSE);
	}



	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) { // серверів може бути кілька, тому не завадить цикл

		// створення SOCKET для підключення до сервера
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET) {
			cout << "failed to create socket: " << WSAGetLastError() << "\n";
			WSACleanup();
			return 13;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		Sleep(PAUSE);

		break;
	}


	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << "unable to connect to server!\n";
		WSACleanup();
		return 14;
	}
	else {;
		Sleep(PAUSE);
	}


	CreateThread(0, 0, Sender, 0, 0, 0);

	CreateThread(0, 0, Receiver, 0, 0, 0);

	Sleep(INFINITE);
}