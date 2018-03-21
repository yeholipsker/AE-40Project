// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Network\Tcp.h"
#include "json\json.h"
#define SERVER_PORT 4444
#define CHECK_COMMAND "Check"
#define START_COMMAND "Start"
#define STOP_COMMAND "Stop"
#define ACCEPT_FAILURE "failed at accept - "

DWORD WINAPI acceptFunction(LPVOID lpParameter) {

	//initialization
	int iResult = 0;
	Json::Value Root;
	Json::CharReaderBuilder Reader;
	stringstream Stream;
	char recvbuf[1024] = { 0 };
	char sendbuf[] = "recieved";
	bool connection = true;
	SOCKET clientSocket = *(SOCKET *)lpParameter;

	//working loop 
	while (connection) {
		iResult = recv(clientSocket, recvbuf, 1024, 0);
		Stream << recvbuf;
		if (iResult > 0) {
			if (!Json::parseFromStream(Reader, Stream, &Root, NULL)) {
				cout << "Couldn't parse json string" << endl;
				continue;
			}
			else {
				cout << "got Action: " << Root["Action"].asString() << endl;
				cout << "from Ip: " << Root["IP"].asString() << endl;
				cout << "and Port: " << Root["Port"].asInt() << endl;
				cout << recvbuf << endl;
				if (Root["Action"].asString() == CHECK_COMMAND) {
					//Sleep(6000); -For testing purposes
					iResult = send(clientSocket, sendbuf, strlen(sendbuf), 0);
					if (iResult == SOCKET_ERROR) {
						cout << "Something went wrong while sending connection check to client." << endl;
					}
				}
			}
		}
		else {
			cout << "Connection with client ended - closing socket" << endl;
			closesocket(clientSocket);
			WSACleanup();
			connection = false;
		}
	}

	return 0;
}

int main()
{
	//initialization
	SOCKET bindSocket;
	bool binding;
	Socket * sock = new Socket();
	Tcp * tcp = new Tcp();

	cout << "Server Is On" << endl;

	//initialize socket
	bindSocket = sock->initialize();
	if (bindSocket == INVALID_SOCKET){
		cout << "Failed at initializing socket" << endl;
	}
	
	//bind and listen on port
	binding = tcp->listenOnPort(SERVER_PORT, "0.0.0.0", bindSocket);
	if (!binding){
		cout << "Failed at binding/listening" << endl;
	}

	//Loop - Accept a client socket
	//unsigned long b = 1;
	//ioctlsocket(bindSocket, FIONBIO, &b);
	while (true){
		SOCKET clientSocket = accept(bindSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			cout << ACCEPT_FAILURE << WSAGetLastError() << endl;
		}
		else {
			cout << "Connection established with client - " << clientSocket << endl;
			CreateThread(0, 0, acceptFunction, &clientSocket, 0, 0);
		}
	}

	return 0;
}

