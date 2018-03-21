#include "stdafx.h"
#include "Socket.h"

Socket::Socket()
{
}

SOCKET Socket::initialize()
{
	if (WSAStartup(MAKEWORD(2, 2), &this->wsadata)) {
		cout << "something went wrong with winsock startup" << endl;
	}
	if (this->wsadata.wVersion != 0x0202) {
		WSACleanup();
		cout << "this is not winsock version 2!" << endl;
	}
	this->mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->mySocket == INVALID_SOCKET){
		WSACleanup();
		cout << "Failed at socket" << endl;
	}
	return this->mySocket;
}

void Socket::close()
{
	closesocket(this->mySocket);
	WSACleanup();
}


Socket::~Socket()
{
}
