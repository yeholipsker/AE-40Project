#pragma once
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

