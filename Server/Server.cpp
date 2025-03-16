#define WIN32_LEAN_AND_MEAN // для пришвидшення процесу компіляції: https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h> // тип WSADATA; функції WSAStartup, WSACleanup та багато інших
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015" // порт — це логічна конструкція, що ідентифікує конкретний процес або тип мережевої служби - https://en.wikipedia.org/wiki/Port_(computer_networking)

#define PAUSE 1

// прийом клієнтського сокета
SOCKET ClientSocket = INVALID_SOCKET;

DWORD WINAPI Sender(void* param)
{
	while (true) {
		// надаємо відповідь відправнику
		char* answer = new char[200];
		cout << "Please enter a message for the client: ";
		cin.getline(answer, 199);

		cout << "the server process sends a response: " << answer << "\n";

		int iSendResult = send(ClientSocket, answer, strlen(answer), 0); // функція send відправляє дані по з'єднаному сокету: https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send

		Sleep(PAUSE);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (iSendResult == SOCKET_ERROR) {
			cout << "sending failed with error: " << WSAGetLastError() << "\n";
			cout << "oops, sending (send) the corresponding message did not take place ((\n";
			closesocket(ClientSocket);
			WSACleanup();
			return 7;
		}
		else {
			//cout << "байтів відправлено: " << iSendResult << "\n";
			Sleep(PAUSE);
		}
	}

	return 0;
}

DWORD WINAPI Receiver(void* param)
{
	while (true) {
		char message[DEFAULT_BUFLEN];

		int iResult = recv(ClientSocket, message, DEFAULT_BUFLEN, 0); // функція recv використовується для читання вхідних даних: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recv
		message[iResult] = '\0';

		if (iResult > 0) {
			cout << "\nthe client process sent the message: " << message << "\n";
			Sleep(PAUSE);
			//cout << "байтів отримано: " << iResult << "\n";
			Sleep(PAUSE);
		}
	}
	return 0;
}

int main()
{
	SetConsoleOutputCP(1251);
	system("title SERVER SIDE");
	// cout << "процес сервера запущено!\n";
	Sleep(PAUSE);


	// ініціалізація Winsock
	WSADATA wsaData; // структура WSADATA містить інформацію про реалізацію Windows Sockets: https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-wsadata
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // функція WSAStartup ініціалізує використання DLL Winsock процесом: https://firststeps.ru/mfc/net/socket/r.php?2
	if (iResult != 0) {
		cout << "WSAStartup failed with the error: " << iResult << "\n";
		cout << "Winsock.dll connection failed!\n";
		return 1;
	}
	else {
		// cout << "підключення Winsock.dll пройшло успішно!\n";
		Sleep(PAUSE);
	}


	struct addrinfo hints; // структура addrinfo використовується функцією getaddrinfo для зберігання інформації про хост-адресу: https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-addrinfoa
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // сімейство адрес Інтернет-протоколу версії 4 (IPv4)
	hints.ai_socktype = SOCK_STREAM; // забезпечує послідовні, надійні, двосторонні потоки з'єднань з механізмом передачі даних
	hints.ai_protocol = IPPROTO_TCP; // протокол TCP (Transmission Control Protocol). Це можливе значення, коли член ai_family є AF_INET або AF_INET6 і член ai_socktype є SOCK_STREAM
	hints.ai_flags = AI_PASSIVE; // адреса сокета буде використовуватися в виклику функції "bind"

	// отримання адреси та порту сервера
	struct addrinfo* result = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed with error: " << iResult << "\n";
		cout << "getting server address and port failed!\n";
		WSACleanup(); // функція WSACleanup завершує використання DLL Winsock 2 (Ws2_32.dll): https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
		return 2;
	}
	else {
		// cout << "отримання адреси та порту сервера пройшло успішно!\n";
		Sleep(PAUSE);
	}


	// створення SOCKET для підключення до сервера
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		cout << "socket failed to create with error: " << WSAGetLastError() << "\n";
		cout << "socket creation failed!\n";
		freeaddrinfo(result);
		WSACleanup();

		return 3;
	}
	else {
		// cout << "створення сокета на сервері пройшло успішно!\n";
		Sleep(PAUSE);
	}


	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen); // функція bind асоціює локальну адресу з сокетом: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind
	if (iResult == SOCKET_ERROR) {
		cout << "bind failed with error: " << WSAGetLastError() << "\n";
		cout << "socket insertion by IP address failed!\n";
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}
	else {
		// cout << "внедрення сокета за IP-адресою пройшло успішно!\n";
		Sleep(PAUSE);
	}

	freeaddrinfo(result);


	iResult = listen(ListenSocket, SOMAXCONN); 
	if (iResult == SOCKET_ERROR) {
		cout << "listen failed with error: " << WSAGetLastError() << "\n";
		cout << "listening from the client has not started. something went wrong!\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 5;
	}
	else {
		cout << "please run client.exe\n";
		// тут можна було б запустити якийсь прелоадер в окремому потоці
	}


	ClientSocket = accept(ListenSocket, NULL, NULL); 
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept failed with error: " << WSAGetLastError() << "\n";
		cout << "connection with the client application is not established! sadly!\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 6;
	}
	else {
		// cout << "з'єднання з клієнтським додатком встановлено успішно!\n";
	}


	CreateThread(0, 0, Receiver, 0, 0, 0);

	CreateThread(0, 0, Sender, 0, 0, 0);


	Sleep(INFINITE);
}