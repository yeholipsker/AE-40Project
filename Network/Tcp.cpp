#include "stdafx.h"
#include "Tcp.h"

// Constructor
Tcp::Tcp(int portNo, const char * ipAddress)
{
	this->portNo = portNo;
	this->ipAddress = ipAddress;
	this->mySocket = new MySocket();
	this->mySocket->initialize();
}

// The connect method.
void Tcp::connectToHost()
{
	this->addr.sin_family = AF_INET; // Address family Internet
	this->addr.sin_port = htons(this->portNo); // Port to connect on
	inet_pton(AF_INET, this->ipAddress, &this->addr.sin_addr.s_addr);//inet_addr(ipAddress); //Target IP - TODO
	if (connect(this->mySocket->getSocket(), (SOCKADDR *)&this->addr, sizeof(this->addr)) == SOCKET_ERROR){
		cout << "couldn't connect. Error: " << WSAGetLastError() << endl;
		return;
	}
	else {
		return;
	}
}

// The bind & listen method.
void Tcp::bindAndListen()
{
	this->addr.sin_family = AF_INET; // address family Internet
	this->addr.sin_port = htons(this->portNo); //Port to connect on
	inet_pton(AF_INET, this->ipAddress, &this->addr.sin_addr.s_addr);//0.0.0.0 for any
	if (bind(this->mySocket->getSocket(), (LPSOCKADDR)&this->addr, sizeof(this->addr)) == SOCKET_ERROR){
		cout << "failed at binding" << endl;
		closesocket(this->mySocket->getSocket());
		WSACleanup();
		return;
	}
	if (listen(this->mySocket->getSocket(), SOMAXCONN) == SOCKET_ERROR) {
		cout << "failed at listening" << endl;
		closesocket(this->mySocket->getSocket());
		WSACleanup();
		return;
	}
}

SOCKET Tcp::acceptClients()
{
	return accept(this->mySocket->getSocket(), NULL, NULL);
}

// Destructor
Tcp::~Tcp() {
	this->mySocket->close();
}
