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
	std::queue<Json::Value> * messageQueue;
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
CRITICAL_SECTION CriticalSection;
CRITICAL_SECTION CriticalSectionStop;

//Functions
DWORD WINAPI Record(LPVOID lpParameter);
DWORD WINAPI Stream(LPVOID lpParameter);
void Initialize(std::string ip, int port);
void UnInitialize();

// The accept function - manage the queue and act according to the actions inside it.
DWORD WINAPI acceptFunction(LPVOID lpParameter) {
	// Initialization
	int iResult = 0;
	Json::Value Root;
	Json::CharReaderBuilder Reader;
	std::stringstream Stream;
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
				std::cout << "Couldn't parse json string" << std::endl;
				continue;
			}
			else {
				EnterCriticalSection(&CriticalSection);
				myData->messageQueue->push(Root);
				LeaveCriticalSection(&CriticalSection);
				if (Root["Action"].asString() == CHECK_COMMAND) {
					iResult = send(myData->clientSocket, sendbuf, strlen(sendbuf), 0);
					if (iResult == SOCKET_ERROR) {
						std::cout << "Something went wrong while sending connection check to client." << std::endl;
					}
				}
				if (Root["Action"].asString() == STOP_COMMAND) {
					break;
				}
			}
		}
		else {
			std::cout << "Connection with client ended - closing socket" << std::endl;
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
	EnterCriticalSection(&CriticalSection);
	std::queue<Json::Value> * messageQueue = (std::queue<Json::Value>*)lpParameter;
	LeaveCriticalSection(&CriticalSection);
	HANDLE handlerRecord = NULL;

	//Loop for managing actions queue
	while (true) {
		Sleep(1);
		// Pop the next action from te queue and do it.
		EnterCriticalSection(&CriticalSection);
		if (!messageQueue->empty()) {

			ActionRoot = messageQueue->front();
			messageQueue->pop();
			LeaveCriticalSection(&CriticalSection);

			// Recognize the action.
			if (ActionRoot["Action"].asString() == START_COMMAND)
			{
				//Initialize streaming variables
				Initialize(ActionRoot["IP"].asString(), ActionRoot["Port"].asInt());
				//Thread for recording from camera and microphone
				handlerRecord = CreateThread(0, 0, Record, 0, 0, 0);
				//Thread for streaming the recorded samples
				CreateThread(0, 0, Stream, 0, 0, 0);
			}

			if (ActionRoot["Action"].asString() == STOP_COMMAND)
			{
				//Stop Recording and the event loop
				EnterCriticalSection(&CriticalSectionStop);
				stopEventLoop = 1;
				LeaveCriticalSection(&CriticalSectionStop);
				break;
			}
		}
		else
		{
			LeaveCriticalSection(&CriticalSection);
		}
	}
	//wait for record thread
	WaitForSingleObject(handlerRecord,INFINITE);
	return 0;
}

// Main fuction - Starts the server, listening to incoming JSON messages from clients,
// and acts according to the messages
int main()
{
	std::cout << "Server Is On" << std::endl;
	InitializeCriticalSection(&CriticalSection);
	InitializeCriticalSection(&CriticalSectionStop);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	// Initialization of TCP and messages queue
	Tcp * tcp = new Tcp(SERVER_PORT, "0.0.0.0");
	std::queue<Json::Value> * messageQueue = new std::queue<Json::Value>();
	LPDWORD threadId = 0;

	// Bind and listen
	tcp->bindAndListen();

	// Creating thread that handles the actions queue.
	HANDLE handlerDequeue = CreateThread(0, 0, dequeFunction, (void*)messageQueue, 0, 0);

	// Accept Tester client 
	SOCKET clientSocket = tcp->acceptClients();
	if (clientSocket == INVALID_SOCKET) {
		std::cout << ACCEPT_FAILURE << WSAGetLastError() << std::endl;
	}
	else {
		std::cout << "Connection established with client - " << clientSocket << std::endl;
		// Create a thread to communicate with the new client.
		MyData* threadData = new MyData();
		threadData->clientSocket = clientSocket;
		EnterCriticalSection(&CriticalSection);
		threadData->messageQueue = messageQueue;
		LeaveCriticalSection(&CriticalSection);
		HANDLE handlerAccept = CreateThread(0, 0, acceptFunction, (void*)threadData, 0, threadId);

		//wait for threads
		WaitForSingleObject(handlerAccept, INFINITE);
		WaitForSingleObject(handlerDequeue, INFINITE);

		//free allocated memory
		delete messageQueue;
		delete threadData;
		delete tcp;
		CoUninitialize();
	}
	std::cout << "Server is down" << std::endl;
	return 0;
}

// Initialize function - Initializing streaming variables
void Initialize(std::string ip, int port) {

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

// UnInitialize function - close and release streaming variables
void UnInitialize()
{
	Medium::close(rtpSinkVid);
	Medium::close(rtpSinkAud);
	Medium::close(h264liveSource);
	Medium::close(mpeg2LiveSource);
	delete rtpGroupsockVid;
	delete rtpGroupsockAud;
	delete scheduler;
	environment->reclaim();

	// Return stop variables to their initial state
	EnterCriticalSection(&CriticalSectionStop);
	stopRecord = false;
	stopEventLoop = 0;
	LeaveCriticalSection(&CriticalSectionStop);
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

	// Initializing media foundation, source reader and encoders
	MFStartup(MF_VERSION);
	Encoder * enc = new Encoder();
	Media * media = new Media();
	if (!media->InitializeSource())
	{
		stopRecord = true;
		// Free allocated memory
		delete enc;
		delete media;
		MFShutdown();
		stopEventLoop = 1;
		return 0;
	}
	else
	{
		enc->InitializeVideoEncoder(NULL);
		enc->InitializeAudioEncoder(media->getOutputMediaTypeAudio());
	}
	//record and encode samples in loop
	while (!stopRecord)
	{
		BYTE* byteArrayVid = NULL;
		//read video samples and create new encoded sample
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
		//copy the sample as buffer, and send to video live source
		BYTE* rawBufferCopyVid = new BYTE[pBuffLengthVid];
		memcpy(rawBufferCopyVid, byteArrayVid, (pBuffLengthVid));
		std::pair <BYTE*, DWORD> myPairVid(rawBufferCopyVid, pBuffLengthVid);
		h264liveSource->PushToQueue(myPairVid);
		delete byteArrayVid;

		BYTE* byteArrayAud = NULL;
		//read audio samples and create new encoded sample
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
		//copy the sample as buffer, and send to video live source
		BYTE* rawBufferCopyAud = new BYTE[pBuffLengthAud];
		memcpy(rawBufferCopyAud, byteArrayAud, (pBuffLengthAud));
		std::pair <BYTE*, DWORD> myPairAud(rawBufferCopyAud, pBuffLengthAud);
		mpeg2LiveSource->PushToQueue(myPairAud);
		delete byteArrayAud;
		//inform live sources that there are new samples
		environment->taskScheduler().triggerEvent(h264liveSource->m_eventTriggerId, h264liveSource);
		environment->taskScheduler().triggerEvent(mpeg2LiveSource->m_eventTriggerId, mpeg2LiveSource);
	}

	// Free allocated memory
	delete enc;
	delete media;
	MFShutdown();
	UnInitialize();
	return 0;
}

//Stream function - responsible for start and stop streaming
DWORD WINAPI Stream(LPVOID lpParameter) {
	rtpSinkVid->startPlaying(*h264liveSource, NULL, NULL);
	rtpSinkAud->startPlaying(*mpeg2LiveSource, NULL, NULL);
	environment->taskScheduler().doEventLoop(&stopEventLoop);
	rtpSinkVid->stopPlaying();
	rtpSinkAud->stopPlaying();
	stopRecord = true;
	return 0;
}