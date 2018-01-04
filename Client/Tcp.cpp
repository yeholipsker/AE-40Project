#include "stdafx.h"
#include "Tcp.h"


Tcp::Tcp()
{
}

bool Tcp::connectToHost(int portNo, const char* ipAddress, SOCKET mySocket)
{
	this->addr.sin_family = AF_INET; // address family Internet
	this->addr.sin_port = htons(portNo); //Port to connect on
	this->addr.sin_addr.s_addr = inet_addr(ipAddress); //Target IP
	if (connect(mySocket, (SOCKADDR *)&this->addr, sizeof(this->addr)) == SOCKET_ERROR){
		cout << "couldn't connect" << endl;
		return false;
	}
	else {
		return true;
	}
}

bool Tcp::listenOnPort(int portNo, const char * ipAddress, SOCKET mySocket)
{
	this->addr.sin_family = AF_INET; // address family Internet
	this->addr.sin_port = htons(portNo); //Port to connect on
	this->addr.sin_addr.s_addr = inet_addr(ipAddress); //0.0.0.0 for any
	if (bind(mySocket, (LPSOCKADDR)&this->addr, sizeof(this->addr)) == SOCKET_ERROR){
		cout << "failed at binding" << endl;
		return false;
	}
	if (listen(mySocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "failed at listening" << endl;
		return false;
	}
	return true;
}


Tcp::~Tcp()
{
}
