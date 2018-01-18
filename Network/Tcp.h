#pragma once
#include "Socket.h"

class Tcp
{
public:
	Tcp();
	bool connectToHost(int portNo, const char* ipAddress, SOCKET mySocket);
	bool listenOnPort(int portNo, const char* ipAddress, SOCKET mySocket);
	~Tcp();
private:
	SOCKADDR_IN addr;
};

