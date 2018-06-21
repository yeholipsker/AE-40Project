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
#include "Media\MPEG2LiveSource.h"

#define SERVER_PORT					4444
#define CHECK_COMMAND				"Check"
#define START_COMMAND				"Start"
#define STOP_COMMAND				"Stop"
#define ACCEPT_FAILURE				"Failed at accept - "
#define BINDING_PORT				8000
#define RTP_PORT					5004
#define BINDING_PORT_AUD			6665
#define RTP_PORT_AUD				6666
#define BINDING_PORT_VID			8887
#define RTP_PORT_VID				8888

// Struct contains data for the threads.
typedef struct MyData
{
	SOCKET clientSocket;
	queue<Json::Value> * messageQueue;
} MyData;

//Global variables for streaming
RTPSink * rtpSinkAud;
RTPSink * rtpSinkVid;
UsageEnvironment* environment;
TaskScheduler * scheduler;
MPEG2LiveSource * mpeg2LiveSource;
H264LiveSource* h264liveSource;
Groupsock * rtpGroupsockAud;
Groupsock * rtpGroupsockVid;
bool stopRecord = false;
char stopEventLoop = 0;

//Functions
DWORD WINAPI Record(LPVOID lpParameter);
DWORD WINAPI Stream(LPVOID lpParameter);
void Initialize(string ip, int port);
void UnInitialize();

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
				myData->messageQueue->push(Root);
				//end of mutex - TODO
				if (Root["Action"].asString() == CHECK_COMMAND) {
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
	//Initialization
	Json::Value ActionRoot;
	queue<Json::Value> * messageQueue = (queue<Json::Value>*)lpParameter;

	//Loop for managing actions queue
	while (true) {
		Sleep(1);
		//need mutex - TOOD
		// Pop the next action from te queue and do it.
		if (!messageQueue->empty()) {

			ActionRoot = messageQueue->front();
			messageQueue->pop();
			//need mutex - TODO

			// Recognize the action.
			if (ActionRoot["Action"].asString() == START_COMMAND)
			{
				//Initialize streaming variables
				Initialize(ActionRoot["IP"].asString(), ActionRoot["Port"].asInt());
				//Thread for recording from camera and microphone
				CreateThread(0, 0, Record, 0, 0, 0);
				//Thread for streaming the recorded samples
				CreateThread(0, 0, Stream, 0, 0, 0);
			}

			if (ActionRoot["Action"].asString() == STOP_COMMAND)
			{
				//Stop Recording and the event loop
				stopRecord = true;
				stopEventLoop = 1;
				//Sleep(1000);
				//UnInitialize();
			}
		}
	}
	return 0;
}

// Main fuction - Starts the server, listening to incoming JSON messages from clients,
// and acts according to the messages
int main()
{
	cout << "Server Is On" << endl;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	// Initialization of TCP and messages queue
	Tcp * tcp = new Tcp(SERVER_PORT, "0.0.0.0");
	queue<Json::Value> * messageQueue = new queue<Json::Value>();

	// Bind and listen
	tcp->bindAndListen();

	// Creating thread that handles the actions queue.
	CreateThread(0, 0, dequeFunction, (void*)messageQueue, 0, 0);

	// Accept clients
	while (true) {
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
			threadData->messageQueue = messageQueue;
			CreateThread(0, 0, acceptFunction, (void*)threadData, 0, 0);
		}
	}
	return 0;
}

// Initialize function - Initializing streaming variables
void Initialize(string ip, int port) {

	//Initialize scheduler, environment
	scheduler = BasicTaskScheduler::createNew();
	environment = BasicUsageEnvironment::createNew(*scheduler);

	//Initialize sockets for audio, video
	struct in_addr destinationAddress;
	destinationAddress.s_addr = our_inet_addr(ip.c_str());
	rtpGroupsockAud = new Groupsock(*environment, destinationAddress, (port - 1), 255);
	rtpGroupsockAud->addDestination(destinationAddress, port, 0);
	rtpGroupsockVid = new Groupsock(*environment, destinationAddress, (port + 1), 255);
	rtpGroupsockVid->addDestination(destinationAddress, (port + 2), 0);

	//Initialize sinks and live source for audio, video
	rtpSinkAud = MPEG1or2AudioRTPSink::createNew(*environment, rtpGroupsockAud);
	mpeg2LiveSource = MPEG2LiveSource::createNew(*environment);
	rtpSinkVid = H264VideoRTPSink::createNew(*environment, rtpGroupsockVid, 96);
	h264liveSource = H264LiveSource::createNew(*environment);
}

void UnInitialize()
{
	rtpSinkVid->stopPlaying();
	rtpSinkAud->stopPlaying();
	Medium::close(rtpSinkVid);
	Medium::close(h264liveSource);
	Medium::close(rtpSinkAud);
	Medium::close(mpeg2LiveSource);
	delete rtpGroupsockVid;
	delete rtpGroupsockAud;
	delete scheduler;
	environment->reclaim();
}

//Record function - recording audio and video samples
DWORD WINAPI Record(LPVOID lpParameter) {

	//variables declaration
	IMFSample* pSampleVid = NULL;
	IMFSample* pSampleAud = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	DWORD pBuffLengthVid = NULL;
	DWORD pBuffLengthAud = NULL;
	LONGLONG timeStamp = NULL;
	HRESULT hr = S_OK;

	//
	MFStartup(MF_VERSION);
	Media * media = new Media();
	media->InitializeSource();
	Encoder * enc = new Encoder();
	enc->InitializeVideoEncoder(NULL);
	enc->InitializeAudioEncoder(NULL);
	while (!stopRecord)
	{
		BYTE* byteArrayVid = NULL;
		while (!byteArrayVid)
		{
			media->getSourceReader()->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
				0, &stIndex, &flags, &timeStamp, &pSampleVid);
			if (pSampleVid)
			{
				enc->TransformVideoSample(pSampleVid, &byteArrayVid, &pBuffLengthVid);
				SafeRelease(pSampleVid);
			}
		}
		BYTE* rawBufferCopyVid = new BYTE[pBuffLengthVid];
		memcpy(rawBufferCopyVid, byteArrayVid, (pBuffLengthVid));
		std::pair <BYTE*, DWORD> myPairVid(rawBufferCopyVid, pBuffLengthVid);
		h264liveSource->PushToQueue(myPairVid);
		delete byteArrayVid;
		environment->taskScheduler().triggerEvent(h264liveSource->m_eventTriggerId, h264liveSource);
		BYTE* byteArrayAud = NULL;
		while (!byteArrayAud)
		{
			media->getSourceReader()->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
				0, &stIndex, &flags, &timeStamp, &pSampleAud);
			if (pSampleAud)
			{
				enc->TransformAudioSample(pSampleAud, &byteArrayAud, &pBuffLengthAud);
				SafeRelease(pSampleAud);
			}
		}
		BYTE* rawBufferCopyAud = new BYTE[pBuffLengthAud];
		memcpy(rawBufferCopyAud, byteArrayAud, (pBuffLengthAud));
		std::pair <BYTE*, DWORD> myPairAud(rawBufferCopyAud, pBuffLengthAud);
		mpeg2LiveSource->PushToQueue(myPairAud);
		delete byteArrayAud;
		environment->taskScheduler().triggerEvent(mpeg2LiveSource->m_eventTriggerId, mpeg2LiveSource);
	}
	stopRecord = false;
	delete enc;
	delete media;
	MFShutdown();
	return 0;
}

DWORD WINAPI Stream(LPVOID lpParameter) {
	rtpSinkVid->startPlaying(*h264liveSource, NULL, NULL);
	rtpSinkAud->startPlaying(*mpeg2LiveSource, NULL, NULL);
	environment->taskScheduler().doEventLoop(&stopEventLoop);
	stopEventLoop = 0;
	return 0;
}