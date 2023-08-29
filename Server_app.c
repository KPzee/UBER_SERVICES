#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE  
#define _CRT_NONSTDC_NO_DEPRECATE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define DEFAULT_PORT "6969"
#define HOST_ADRESS "192.168.2.53"

#define DEFAULT_BUFLEN 512

#pragma comment(lib, "Ws2_32.lib")


int main(void)
{
	printf("################################################### PROGRAM STARTED ####################################################\n\n");


	WSADATA wsaData;

	FILE *html_file =  fopen("HTMLPage.html", "r");
	if (html_file == NULL) {
		printf("Error! opening file");
		return EXIT_FAILURE;
	}
	char buff[512];
	fgets(buff, 512, html_file);

	printf("%s", buff);


	int iResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	char recvbuf[DEFAULT_BUFLEN];

	// Her we are initilizing winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup was a failure I know you are probably tight but keep itq Pzee");
		return 1;
	}

	//Creating a socket for the server

	struct addrinfo* S_result = NULL, * S_ptr = NULL, S_hints;

	memset(&S_hints, 0, sizeof(S_hints));
	S_hints.ai_family = AF_INET;
	S_hints.ai_socktype = SOCK_STREAM;
	S_hints.ai_protocol = IPPROTO_TCP;
	S_hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(HOST_ADRESS, DEFAULT_PORT, &S_hints, &S_result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return EXIT_FAILURE;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	ListenSocket = socket(S_result->ai_family, S_result->ai_socktype, S_result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(S_result);
		WSACleanup();
		return EXIT_FAILURE;
	}

	iResult = bind(ListenSocket, S_result->ai_addr, (int)S_result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(S_result);
		closesocket(ListenSocket);
		WSACleanup();
		return EXIT_FAILURE;
	}
	freeaddrinfo(S_result);

	SOCKET ClientSocket = INVALID_SOCKET;

	//Listening on a socket
	while (1) {
		if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
			printf("Listen failed with error: %ld\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return EXIT_FAILURE;
		}

		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return EXIT_FAILURE;
		}

		do {
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("Bytes received: %d\n", iResult);

				char response[] = "HTTP /1.1 200 Ok\r\nServer: Kpzee\r\nAllow: GET, HEAD, PUT\r\nContent-Language: en\r\nContent-Length: 300\r\nContent-Type: text/html; charset=UTF-8\r\nExpires: Thu, 07 Dec 2023 16:00:00 GMT\r\nLast-Modified: Tue, 15 Nov 1994 12:45:26 GMT\r\n\r\n<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Test</title></head><body><h1>this is a Test</h1></body><html>";

				iSendResult = send(ClientSocket, response, iResult, 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return EXIT_FAILURE;
				}
				printf("Bytes set: % d\n", iSendResult);
				printf("%s\n", recvbuf);
			}
			else if (iResult == 0) {
				printf("Connection closing...\n");
			}
			else {
				iResult = shutdown(ClientSocket, SD_SEND);
				if (iResult == SOCKET_ERROR) {
					printf("Shutdown failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return EXIT_FAILURE;
				}
				printf("recv failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return EXIT_FAILURE;
			}
		} while (iResult > 0);
	}