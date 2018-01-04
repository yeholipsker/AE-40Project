// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Tcp.h"
#include "Socket.h"

int main()
{
	char recvbuf[1024] = { 0 };
	Socket * sock = new Socket();
	SOCKET clientSocket = sock->initialize();
	Tcp * tcp = new Tcp();
	tcp->listenOnPort(1234, "0.0.0.0", clientSocket);
	// Accept a client socket
	clientSocket = accept(clientSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		cout << "failed at accept" << endl;
		sock->close();
		return 1;
	}

	int iResult = recv(clientSocket, recvbuf, 1024, 0);
	if (iResult > 0) {
		cout << recvbuf << endl;
	}
	sock->close();
	return 0;
}

