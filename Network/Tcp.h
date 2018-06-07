#pragma once
#include "MySocket.h"

class Tcp
{
public:
	Tcp(int portNo, const char* ipAddress);
	void connectToHost();
	void bindAndListen();
	SOCKET acceptClients();
	~Tcp();
private:
	SOCKADDR_IN addr;
	int portNo; 
	const char* ipAddress;
	MySocket * mySocket;
};

