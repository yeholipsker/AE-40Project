// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Tcp.h"
#include <string>

int main()
{
	char recvbuf[1024];
	int iResult;
	string str("this is a test");
	Socket * sock = new Socket();
	Tcp * tcp = new Tcp();
	SOCKET serverSocket = sock->initialize();
	string ipAddress;
	cin >> ipAddress;
	tcp->connectToHost(1234, ipAddress.c_str(), serverSocket);
	iResult = send(serverSocket, str.c_str(), str.size(), 0);
	if (iResult == SOCKET_ERROR) {
		cout << "failed";
		sock->close();
		return 1;
	}
	sock->close();
    return 0;
}

