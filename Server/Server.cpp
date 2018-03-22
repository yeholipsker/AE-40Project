// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Network\Tcp.h"
#include "json\json.h"
#include <queue>

#define SERVER_PORT					4444
#define CHECK_COMMAND				"Check"
#define START_COMMAND				"Start"
#define STOP_COMMAND				"Stop"
#define ACCEPT_FAILURE				"Failed at accept - "

// Struct contains data for the threads.
typedef struct MyData
{
	SOCKET clientSocket;
	queue<Json::Value> * myQ;
} MyData, *PmyData;

// The accept function - manage the queue and act according to the actions inside it.
DWORD WINAPI acceptFunction(LPVOID lpParameter) {
	// Initialization
	int iResult = 0;
	Json::Value Root;
	Json::CharReaderBuilder Reader;
	stringstream Stream;
	char recvbuf[1024] = { 0 };
	char sendbuf[] = "recieved";
	bool connection = true;
	PmyData myData = (PmyData)lpParameter;

	// Working loop 
	while (connection) {
		iResult = recv(myData->clientSocket, recvbuf, 1024, 0);
		Stream << recvbuf;
		if (iResult > 0) {
			if (!Json::parseFromStream(Reader, Stream, &Root, NULL)) {
				cout << "Couldn't parse json string" << endl;
				continue;
			}
			else {
				//need mutex - TODO
				myData->myQ->push(Root);
				//end of mutex - TODO
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

// The queue function - manage the queue and act according to the actions inside it.
DWORD WINAPI dequeFunction(LPVOID lpParameter) {
	Json::Value Root;
	queue<Json::Value> * myQ = (queue<Json::Value>*)lpParameter;
	while (true){
		//need mutex - TOOD
		if (!myQ->empty()){
			Root = myQ->front();
			myQ->pop();
			//need mutex - TODO

			// Print the action to the screen - temporary!
			cout << "got Action: " << Root["Action"].asString() << endl;
			cout << "from Ip: " << Root["IP"].asString() << endl;
			cout << "and Port: " << Root["Port"].asInt() << endl;
		}
	}
	return 0;
}

int main()
{
	// Initialization
	SOCKET listenSocket;
	bool binding;
	Socket * sock = new Socket();
	Tcp * tcp = new Tcp();
	// The actions queue.
	queue<Json::Value> * myQ = new queue<Json::Value>();

	cout << "Server Is On" << endl;

	// Creating thread that handles the actions queue.
	CreateThread(0, 0, dequeFunction, myQ, 0, 0);

	// Initialize socket.
	listenSocket = sock->initialize();

	// Bind and listen. - TODO - Change the function name to "bindAndListen".
	binding = tcp->listenOnPort(SERVER_PORT, "0.0.0.0", listenSocket);

	// Loop - Accept a client socket
	while (true){
		SOCKET clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			cout << ACCEPT_FAILURE << WSAGetLastError() << endl;
		}
		else {
			cout << "Connection established with client - " << clientSocket << endl;
			// Create a thread to communicate with the new client.
			PmyData threadData = new MyData();
			threadData->clientSocket = clientSocket;
			threadData->myQ = myQ;
			CreateThread(0, 0, acceptFunction, threadData, 0, 0);
		}
	}

	return 0;
}

