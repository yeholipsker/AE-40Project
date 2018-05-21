#include "stdafx.h"
#include "Socket.h"
#include "Rtp.h"

// Constructor
Socket::Socket() { }

// initialize the socket.
void Socket::initialize()
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
}

SOCKET Socket::getSocket()
{
	return this->mySocket;
}

// Close the socket.
void Socket::close()
{
	closesocket(this->mySocket);
	WSACleanup();
}

// Destructor
Socket::~Socket() { }
