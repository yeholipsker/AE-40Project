// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Network\Tcp.h"
#include "json\json.h"
#define SERVER_PORT 4444
#define CHECK_COMMAND "Check"
#define START_COMMAND "Start"
#define STOP_COMMAND "Stop"

int main()
{
	//initialization
	int iResult = 0;
	Json::Value Root;
	Json::CharReaderBuilder Reader;
	stringstream Stream;
	//initialize one connection (maybe need multiple)
	char recvbuf[1024] = { 0 };
	char sendbuf[] = "recieved";
	Socket * sock = new Socket();
	SOCKET clientSocket = sock->initialize();
	cout << "socket initialized with value - " << clientSocket << endl;
	Tcp * tcp = new Tcp();
	bool bool1 = tcp->listenOnPort(SERVER_PORT, "0.0.0.0", clientSocket);
	cout << "listen on port returned with : " << bool1 << endl;
	// Accept a client socket
	clientSocket = accept(clientSocket, NULL, NULL);
	cout << "accept returned with : " << clientSocket << endl;
	if (clientSocket == INVALID_SOCKET) {
		cout << "failed at accept" << endl;
		sock->close();
		return 1;
	}
	while (true) {
		iResult = recv(clientSocket, recvbuf, 1024, 0);
		Stream << recvbuf;
		cout << "iResult = " << iResult << endl;
		if (iResult > 0) {
			if (!Json::parseFromStream(Reader, Stream, &Root, NULL)) {
				cout << "Couldn't parse json string" << endl;
				continue;
			}
			else {
				cout << "got Action: " << Root["Action"].asString() << endl;
				cout << "from Ip: " << Root["IP"].asString() << endl;
				cout << "and Port: " << Root["Port"].asInt() << endl;
				cout << recvbuf << endl;
				if (Root["Action"].asString() == CHECK_COMMAND) {
					iResult = send(clientSocket, sendbuf, strlen(sendbuf), 0);
					if (iResult == SOCKET_ERROR) {
						cout << "Something went wrong while sending connection check to client." << endl;
					}
				}
			}
		}
	}
	sock->close();
	return 0;
}

