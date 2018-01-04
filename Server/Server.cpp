// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Client/Tcp.h"
#include "../Client/Socket.h"

#define SERVER_PORT 4444

int main()
{
	char recvbuf[1024] = { 0 };
	Socket * sock = new Socket();
	SOCKET clientSocket = sock->initialize();
	cout << "socket initialized with value - " << clientSocket << endl;
	Tcp * tcp = new Tcp();
	bool bool1 = tcp->listenOnPort(SERVER_PORT, "0.0.0.0", clientSocket);
	cout << "listen on port returned with : " << bool1 << endl;
	// Accept a client socket
	clientSocket = accept(clientSocket, NULL, NULL);
	cout << "accept returned with : " << clientSocket << endl;
	if (clientSocket == INVALID_SOCKET) {
		cout << "failed at accept" << endl;
		sock->close();
		return 1;
	}

	int iResult = recv(clientSocket, recvbuf, 1024, 0);
	cout << "iResult = " << iResult;
	if (iResult > 0) {
		cout << recvbuf << endl;
	}
	sock->close();
	return 0;
}

