// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Network\Tcp.h"
#include "json\json.h"
#include <queue>
#include "Media\Media.h"
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include <GroupsockHelper.hh>
#include "Media\H264LiveSource.h"

#define SERVER_PORT					4444
#define CHECK_COMMAND				"Check"
#define START_COMMAND				"Start"
#define STOP_COMMAND				"Stop"
#define ACCEPT_FAILURE				"Failed at accept - "
#define BINDING_PORT				8888
#define RTP_PORT					8886

// Struct contains data for the threads.
typedef struct MyData
{
	SOCKET clientSocket;
	queue<Json::Value> * myQ;
} MyData;

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
	MyData* myData = (MyData*)lpParameter;

	// Working loop 
	while (connection) {
		Sleep(1);
		memset(recvbuf, 0, 1024);
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
			connection = false;
		}
	}
	return 0;
}

// The queue function - manage the queue and act according to the actions inside it.
DWORD WINAPI dequeFunction(LPVOID lpParameter) {
	Json::Value ActionRoot;
	queue<Json::Value> * myQ = (queue<Json::Value>*)lpParameter;

	// Create the media manager.
	Media* media = new Media();

	while (true){
		Sleep(1);
		//need mutex - TOOD
		// Pop the next action from te queue and do it.
		if (!myQ->empty()) {

			ActionRoot = myQ->front();
			myQ->pop();
			//need mutex - TODO

			// Recognize the action.
			if (ActionRoot["Action"].asString() == START_COMMAND)
			{
				//media->StartRecordingToFile();
				TaskScheduler* scheduler = BasicTaskScheduler::createNew();
				UsageEnvironment* environment = BasicUsageEnvironment::createNew(*scheduler);

				// Set address to Multicast.
				in_addr destinationAddress = { 127, 0, 0, 1 };
				Groupsock rtpGroupsock(*environment, destinationAddress, BINDING_PORT, 255);
				rtpGroupsock.addDestination(destinationAddress, RTP_PORT, 0);
				RTPSink * rtpSink = H264VideoRTPSink::createNew(*environment, &rtpGroupsock, 96);
				H264LiveSource* h264liveSource = H264LiveSource::createNew(*environment);
				rtpSink->startPlaying(*h264liveSource, NULL, NULL);
				environment->taskScheduler().doEventLoop();
			}
			if (ActionRoot["Action"].asString() == STOP_COMMAND)
			{
				media->StopRecording();
			}
			
			// Do the action.

			/************************************************************************/

			// Print the action to the screen - temporary!
			cout << "got Action: " << ActionRoot["Action"].asString() << endl;
			cout << "from Ip: " << ActionRoot["IP"].asString() << endl;
			cout << "and Port: " << ActionRoot["Port"].asInt() << endl;

			/************************************************************************/
		}
	}
	return 0;
}

int main()
{
	cout << "Server Is On" << endl;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	MFStartup(MF_VERSION);
	// Initialization
	Tcp * tcp = new Tcp(SERVER_PORT, "0.0.0.0");
	queue<Json::Value> * myQ = new queue<Json::Value>();

	// Bind and listen
	tcp->bindAndListen();

	// Creating thread that handles the actions queue.
	CreateThread(0, 0, dequeFunction, myQ, 0, 0);

	// Accept clients
	while (true){
		Sleep(1);
		SOCKET clientSocket = tcp->acceptClients();
		if (clientSocket == INVALID_SOCKET) {
			cout << ACCEPT_FAILURE << WSAGetLastError() << endl;
		}
		else {
			cout << "Connection established with client - " << clientSocket << endl;
			// Create a thread to communicate with the new client.
			MyData* threadData = new MyData();
			threadData->clientSocket = clientSocket;
			threadData->myQ = myQ;
			CreateThread(0, 0, acceptFunction, threadData, 0, 0);
		}
	}
	return 0;
}

