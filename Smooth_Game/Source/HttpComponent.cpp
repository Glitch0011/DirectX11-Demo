#include <HttpComponent.h>

using namespace SmoothGame;

#include <thread>
using namespace std;

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

HttpComponent::HttpComponent()
{
	this->running = true;

	std::thread([=]()
	{
		WSADATA wsaData;

		auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) 
		{
			printf("WSAStartup failed: %d\n", iResult);
			return;
		}

		struct addrinfo *result = NULL, *ptr = NULL, hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(NULL, "80", &hints, &result);
		if (iResult != 0) 
		{
			WSACleanup();
			return;
		}

		SOCKET ListenSocket = INVALID_SOCKET;
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			WSACleanup();
			return;
		}

		iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return;
		}

		freeaddrinfo(result);

		if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(ListenSocket);
			WSACleanup();
			return;
		}

		SOCKET ClientSocket = INVALID_SOCKET;

		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			closesocket(ListenSocket);
			WSACleanup();
			return;
		}

		do 
		{
			char recvbuf[512];
			int iResult = recv(ClientSocket, recvbuf, 512, 0);
			if (iResult > 0) 
			{
				printf("Bytes received: %d\n", iResult);

				int iSendResult = send(ClientSocket, recvbuf, iResult, 0);
				if (iSendResult == SOCKET_ERROR)
				{
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return;
				}
				printf("Bytes sent: %d\n", iSendResult);
			}
			else if (iResult == 0)
			{
				printf("Connection closing...\n");
			}
			else 
			{
				printf("recv failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return;
			}

		} while (iResult > 0);

	}).detach();

	this->functions[L"stop"] = [=](Params params)
	{
		this->running = false;
		return S_OK;
	};
}