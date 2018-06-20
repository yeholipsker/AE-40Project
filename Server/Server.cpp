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
	queue<Json::Value> * myQ;
} MyData;

FramedSource * audioSource;
//FramedSource * videoSource;
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
DWORD WINAPI Record(LPVOID lpParameter);
DWORD WINAPI Stream(LPVOID lpParameter);
void initialize();

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
	Json::Value ActionRoot;
	queue<Json::Value> * myQ = (queue<Json::Value>*)lpParameter;

	initialize();
	// Create the media manager.
	//Media* media = new Media();
	//media->InitializeSource();

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
				CreateThread(0, 0, Record, 0, 0, 0);
				CreateThread(0, 0, Stream, 0, 0, 0);
			}
			
			if (ActionRoot["Action"].asString() == STOP_COMMAND)
			{
				stopRecord = true;
			}
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

void initialize() {

	scheduler = BasicTaskScheduler::createNew();
	environment = BasicUsageEnvironment::createNew(*scheduler);

	in_addr destinationAddress = { 127, 0, 0, 1 };

	rtpGroupsockAud = new Groupsock(*environment, destinationAddress, BINDING_PORT_AUD, 255);
	rtpGroupsockAud->addDestination(destinationAddress, RTP_PORT_AUD, 0);
	//rtpGroupsockAud->multicastSendOnly();
	rtpGroupsockVid = new Groupsock(*environment, destinationAddress, BINDING_PORT_VID, 255);
	rtpGroupsockVid->addDestination(destinationAddress, RTP_PORT_VID, 0);
	//rtpGroupsockVid->multicastSendOnly();
	rtpSinkAud = MPEG1or2AudioRTPSink::createNew(*environment, rtpGroupsockAud);
	mpeg2LiveSource = MPEG2LiveSource::createNew(*environment);
	//audioSource = MPEG1or2AudioStreamFramer::createNew(*environment, mpeg2LiveSource);

	rtpSinkVid = H264VideoRTPSink::createNew(*environment, rtpGroupsockVid, 96);
	h264liveSource = H264LiveSource::createNew(*environment);
	//videoSource = H264VideoStreamFramer::createNew(*environment, h264liveSource,False);
}

DWORD WINAPI Record(LPVOID lpParameter) {
	IMFSample* pSampleVid = NULL;
	IMFSample* pSampleAud = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	DWORD pBuffLengthVid = NULL;
	DWORD pBuffLengthAud = NULL;
	LONGLONG timeStamp = NULL;
	HRESULT hr = S_OK;
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
	stopEventLoop = 1;
	stopRecord = false;
	return 0;
}
DWORD WINAPI Stream(LPVOID lpParameter) {
	rtpSinkVid->startPlaying(*h264liveSource, NULL, NULL);
	rtpSinkAud->startPlaying(*mpeg2LiveSource, NULL, NULL);
	environment->taskScheduler().doEventLoop(&stopEventLoop);
	stopEventLoop = 0;
	return 0;
}