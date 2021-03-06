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
#define DEFAULT_PORT "27020"

int table[9]; // 0: empty, 1: cross, 2: circle
int iResult;
int iSendResult;

const int BOARD_X = 3;
const int BOARD_Y = 3;

bool gameOver();
bool ThreeInARow(int position, int symbol);

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;


	bool validInput(int input);
	void showTable();

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


	do { // We break from the game loop to get back here to the lobby loop

		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN;

		//*****************************************************************************************************
		// LOBBY LOOP
		do {

			cout << "Which room would you like to join? (1 or 2):\n";

			string message = "";

			cin.clear();
			cin.ignore(cin.rdbuf()->in_avail());

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

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			int msg = atoi(recvbuf);
			if (msg == 1) { // We can join the room
				cout << "You have joined room " << message;
				break;
			}
			else {  // We cannot join the room
				cout << "Room cannot be joined" << endl;
				continue;
			}
			//--------------------------------------------------------------------------
		} while (true);
		// END LOBBY LOOP
		//*****************************************************************************************************

		//----------------------------------------------------
		// Receive a player number from server
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		int playerNumber = atoi(recvbuf);
		bool myTurn = (playerNumber == 1 ? true : false);
		cout << " as player " << playerNumber << endl;
		//----------------------------------------------------

		cout << "Waiting for game to start...\n\n";
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);// Receiving something means the game has started
		cout << "The game has started\n\n";

		cout << "Table is:\n";
		cout << "[1][2][3]\n";
		cout << "[4][5][6]\n";
		cout << "[7][8][9]\n\n";
		//*****************************************************************************************************
		// GAME LOOP
		do // Each loop is a turn
		{
			//----------------------------------------------------------------------------------
			// Wait for opponent if it is not our turn

			if (!myTurn) {
				cout << "Waiting for opponent...\n\n";
				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

				//------------------------------------------------
				// Check if we have lost

				if (string(recvbuf) == "L") {
					iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
					table[atoi(recvbuf)] = (playerNumber == 1 ? 2 : 1);
					showTable();
					cout << "You lost! Returning to lobby...\n";
					break;
				}

				//------------------------------------------------
				table[atoi(recvbuf)] = (playerNumber == 1 ? 2 : 1); // Update with the opponent's turn
				cout << "The opponent has made his move\n\n";
				showTable();
			}

			//----------------------------------------------------------------------------------

			//--------------------------------------------------------------------------
			// Get user's input and send it to server

			cout << "Where would you like to place your symbol? (from 1 to 9)\n";
			cout << "--> ";

			int input = 0;

			do {

				cin.clear();
				cin.ignore(cin.rdbuf()->in_avail());

				cin >> input;
				input--; // Because array index starts at 0

				if (!validInput(input)) {
					cout << "Please enter a valid input\n";
					continue; // Get input again
				}
				if (table[input] != 0) {
					cout << "Position already in use\n";
					continue; // Get input again
				}
				break; // No errors found 
			} while (true);

			//--------------------------------------------------------------------------
			// Update and show the table

			table[input] = playerNumber; // Update
			showTable(); // Show
			if (gameOver()) { // Game over after our turn means we won
				cout << "You won! Returning to lobby...\n";
				// Send to server that we won
				char msg[2] = "W";
				iSendResult = send(ConnectSocket, msg, sizeof(msg), 0);
				// Send last input to server
				char input_char[2];
				sprintf_s(input_char, "%d", input);
				iSendResult = send(ConnectSocket, input_char, (int)strlen(input_char), 0);
				break;
			}
			//--------------------------------------------------------------------------
			// We have a valid input, send it to server
			char input_char[2];
			sprintf_s(input_char, "%d", input);
			iSendResult = send(ConnectSocket, input_char, (int)strlen(input_char), 0);
			//--------------------------------------------------------------------------



			myTurn = false;

		} while (true); // <-- End of turn, start over
		// END GAME LOOP
		//*****************************************************************************************************

		// Out of game, reset our table and loop back to lobby
		memset(table, 0, sizeof(table));

	} while (true); // Loops back to lobby

	//shutdown the connection since no more data will be sent...
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

bool validInput(int input) {

	if (input >= 0 && input <= (BOARD_X * BOARD_Y) - 1) {
		return true;
	}
	return false;
}

void showTable() {

	for (int i = 0; i < BOARD_X * BOARD_Y; i++) {
		if (i % BOARD_X == 0) {
			cout << "\n";
		}
		if (table[i] == 0) {
			cout << "[ ]";
		}
		else if (table[i] == 1) {
			cout << "[X]";
		}
		else if (table[i] == 2) {
			cout << "[O]";
		}
		else {
			cout << "error";
		}
	}
	cout << "\n";
}

bool gameOver() {
	// Return true if we have three in a row

	//     [x,y]
	//[0,0][1,0][2,0]
	//[0,1][1,1][2,1]
	//[0,2][1,2][2,2]

	//-------------------------------------------------------------------------
	// Search downwards where y = 0
	for (int x = 0; x < BOARD_X; x++) {
		if (table[x] == 0) {
			continue; // No symbol means there wont be 3 in a row, go to next x
		}
		if (ThreeInARow(x, table[x])) {
			return true;
		}
	}
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// Search to the right where x = 0
	for (int y = 1; y < BOARD_Y; y++) {
		if (table[BOARD_Y * y] == 0) {
			continue; // No symbol means there wont be 3 in a row, go to next y
		}
		if (ThreeInARow(BOARD_Y * y, table[BOARD_Y * y])) {
			return true;
		}
	}
	//-------------------------------------------------------------------------
	return false; // 3 in a row wasn't found, game hasn't ended
}

bool ThreeInARow(int position, int symbol) {

	// Info: Either x is 0 or y is 0 or both are 0

	// TODO: replace '3' and '2' with constants
	int x = position % 3;
	int y = position / 3;

	if (y == 0) {
		// Vertical check
		if (table[x + 3 * (y + 1)] == symbol && table[x + 3 * (y + 2)] == symbol) {
			return true;
		}
	}
	if (x == 0) {
		// Horizontal check
		if (table[(x + 1) + 3 * y] == symbol && table[(x + 2) + 3 * y] == symbol) {
			return true;
		}
	}

	if (y == 0 && x == 0) {
		// Diagonal check
		if (table[(x + 1) + 3 * (y + 1)] == symbol && table[(x + 2) + 3 * (y + 2)] == symbol) {
			return true;
		}
	}

	if (y == 2 && x == 0) {
		// Diagonal check
		if (table[(x + 1) + 3 * (y - 1)] == symbol && table[(x + 2) + 3 * (y - 2)] == symbol) {
			return true;
		}
	}
	return false;
}