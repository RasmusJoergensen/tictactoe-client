// Client.cpp : Defines the entry point for the console application..
//
#include "stdafx.h"
#include <iostream>
#include <string>

//fedt man spa

using namespace std;

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "27015"

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
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
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	//printf("Bytes Sent: %ld\n", iResult);

	// iSendResult = send(ConnectSocket, msg, (int)strlen(msg), 0);	// Send
	// iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);	// Receive
	// sprintf_s(a_char_array, "%d", an_integer);				// Convert int to char array
	// int input_int = atoi(input);								// Convert from char* to int
	// const char* char = a_string.c_str();						// Convert from string to char*

	// Receive until the peer closes the connection
	do {
		cout << "Which room would you like to join? (1 or 2):\n";

		string message = "";

		getline(cin, message);

		//-------------------------------------------------------------------------
		// Make sure that the client's input is either 1 or 2.

		if (message == "1" || message == "2") {
			const char* message_char = message.c_str();
			iSendResult = send(ConnectSocket, message_char, (int)strlen(message_char), 0);

		}
		else {
			cout << "Wrong input, try again\n";
			continue;
		}

		//-------------------------------------------------------------------------

	} while (iResult > 0);

	//shutdown the connection since no more data will be sent.
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	// cleanup
	WSACleanup();
	closesocket(ConnectSocket);

	return 0;
}











