#pragma once
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <windows.h>
#include <iostream>
using namespace std;
class MySocket
{
public:
	MySocket();
	void initialize();
	SOCKET getSocket();
	void close();
	~MySocket();
private:
	SOCKET mySocket;
	WSADATA wsadata;
};

