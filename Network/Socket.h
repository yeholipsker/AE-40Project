#pragma once
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <windows.h>
#include <iostream>
using namespace std;
class Socket
{
public:
	Socket();
	SOCKET initialize();
	void close();
	~Socket();
private:
	SOCKET mySocket;
	WSADATA wsadata;
};

