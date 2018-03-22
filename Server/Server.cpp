// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Network\Tcp.h"
#include "json\json.h"
#include <queue>
#define SERVER_PORT 4444
#define CHECK_COMMAND "Check"
#define START_COMMAND "Start"
#define STOP_COMMAND "Stop"
#define ACCEPT_FAILURE "failed at accept - "

typedef struct MyData
{
	SOCKET clientSocket;
	queue<Json::Value> * myQ;
}MyData, *PmyData;

DWORD WINAPI acceptFunction(LPVOID lpParameter) {

	//initialization
	int iResult = 0;
	Json::Value Root;
	Json::CharReaderBuilder Reader;
	stringstream Stream;
	char recvbuf[1024] = { 0 };
	char sendbuf[] = "recieved";
	bool connection = true;
	//SOCKET clientSocket = *(SOCKET *)lpParameter;
	PmyData myData = (PmyData)lpParameter;

	//working loop 
	while (connection) {
		iResult = recv(myData->clientSocket, recvbuf, 1024, 0);
		Stream << recvbuf;
		if (iResult > 0) {
			if (!Json::parseFromStream(Reader, Stream, &Root, NULL)) {
				cout << "Couldn't parse json string" << endl;
				continue;
			}
			else {
				//need mutex
				myData->myQ->push(Root);
				//end of mutex
				cout << recvbuf << endl;
				if (Root["Action"].asString() == CHECK_COMMAND) {
					//Sleep(6000); -For testing purposes
					iResult = send(myData->clientSocket, sendbuf, strlen(sendbuf), 0);
					if (iResult == SOCKET_ERROR) {
						cout << "Something went wrong while sending connection check to client." << endl;
					}
				}
			}
		}
		else {
			cout << "Connection with client ended - closing socket" << endl;
			closesocket(myData->clientSocket);
			WSACleanup();
			connection = false;
		}
	}

	return 0;
}

DWORD WINAPI dequeFunction(LPVOID lpParameter) {
	Json::Value Root;
	queue<Json::Value> * myQ = (queue<Json::Value>*)lpParameter;
	while (true){
		//need mutex
		if (!myQ->empty()){
			Root = myQ->front();
			myQ->pop();
			//need mutex
			cout << "got Action: " << Root["Action"].asString() << endl;
			cout << "from Ip: " << Root["IP"].asString() << endl;
			cout << "and Port: " << Root["Port"].asInt() << endl;
		}
	}
	return 0;
}

int main()
{
	//initialization
	SOCKET bindSocket;
	bool binding;
	Socket * sock = new Socket();
	Tcp * tcp = new Tcp();
	queue<Json::Value> * myQ = new queue<Json::Value>();

	cout << "Server Is On" << endl;

	//creating thread that handles actions queue
	CreateThread(0, 0, dequeFunction, myQ, 0, 0);

	//initialize socket
	bindSocket = sock->initialize();
	if (bindSocket == INVALID_SOCKET){
		cout << "Failed at initializing socket" << endl;
	}
	
	//bind and listen on port
	binding = tcp->listenOnPort(SERVER_PORT, "0.0.0.0", bindSocket);
	if (!binding){
		cout << "Failed at binding/listening" << endl;
	}

	//Loop - Accept a client socket
	while (true){
		SOCKET clientSocket = accept(bindSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			cout << ACCEPT_FAILURE << WSAGetLastError() << endl;
		}
		else {
			cout << "Connection established with client - " << clientSocket << endl;
			PmyData threadData = new MyData();
			threadData->clientSocket = clientSocket;
			threadData->myQ = myQ;
			CreateThread(0, 0, acceptFunction, threadData, 0, 0);
		}
	}

	return 0;
}

