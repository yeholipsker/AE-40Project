#include "stdafx.h"
#include "MySocket.h"

// Constructor
MySocket::MySocket() { }

// initialize the socket.
void MySocket::initialize()
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

SOCKET MySocket::getSocket()
{
	return this->mySocket;
}

// Close the socket.
void MySocket::close()
{
	closesocket(this->mySocket);
	WSACleanup();
}

// Destructor
MySocket::~MySocket() { }
