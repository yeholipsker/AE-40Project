// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Tcp.h"
#include <string>

#define SERVER_PORT 4444 

int main()
{
	char recvbuf[1024];
	int iResult;
	string str("this is a test");

	//Socket * sock = new Socket();
	Socket * sock;
	cout << "Socket * sock = new Socket() : " << (sock = new Socket()) << endl;

	//Tcp * tcp = new Tcp();
	Tcp * tcp;
	cout << "Tcp * tcp = new Tcp() : " << (tcp = new Tcp()) << endl;
	
	//SOCKET serverSocket = sock->initialize()
	SOCKET serverSocket;
	cout << "SOCKET serverSocket = sock->initialize() : " << (serverSocket = sock->initialize()) << endl;

	string ipAddress;
	cin >> ipAddress;

	//tcp->connectToHost(SERVER_PORT, ipAddress.c_str(), serverSocket);
	cout << "tcp->connectToHost(SERVER_PORT, ipAddress.c_str(), serverSocket) : " << (tcp->connectToHost(SERVER_PORT, ipAddress.c_str(), serverSocket)) << endl;

	iResult = send(serverSocket, str.c_str(), str.size(), 0);
	cout << "iResult = send(serverSocket, str.c_str(), str.size(), 0) : " << (send(serverSocket, str.c_str(), str.size(), 0)) << endl;

	if (iResult == SOCKET_ERROR) {
		cout << "failed";
		sock->close();
		return 1;
	}
	sock->close();
    return 0;
}

