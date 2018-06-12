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
#define RTP_PORT					8002

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

				Groupsock rtpGroupsockVid(*environment, destinationAddress, 6666, 255);
				rtpGroupsockVid.addDestination(destinationAddress, 6664, 0);

				RTPSink * rtpSink = MPEG1or2AudioRTPSink::createNew(*environment, &rtpGroupsock);
				MPEG2LiveSource * mpeg2LiveSource = MPEG2LiveSource::createNew(*environment);
				MediaSource* audioSource = MPEG1or2AudioStreamFramer::createNew(*environment, mpeg2LiveSource);
				rtpSink->startPlaying(*audioSource, NULL, NULL);

				//RTPSink * rtpSinkVid = H264VideoRTPSink::createNew(*environment, &rtpGroupsockVid, 96);
				//H264LiveSource* h264liveSource = H264LiveSource::createNew(*environment);
				//rtpSinkVid->startPlaying(*h264liveSource, NULL, NULL);
				environment->taskScheduler().doEventLoop();
			}
			if (ActionRoot["Action"].asString() == STOP_COMMAND)
			{
				media->StopRecording();
			}
			
			// Do the action.

			// Print the action to the screen - temporary!
			cout << "got Action: " << ActionRoot["Action"].asString() << endl;
			cout << "from Ip: " << ActionRoot["IP"].asString() << endl;
			cout << "and Port: " << ActionRoot["Port"].asInt() << endl;

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


/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2018, Live Networks, Inc.  All rights reserved
// A test program that reads a MPEG-1 or 2 Program Stream file,
// splits it into Audio and Video Elementary Streams,
// and streams both using RTP
// main program
/*
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

UsageEnvironment* env;
char const* inputFileName = "test.mpg";
MPEG1or2Demux* mpegDemux;
FramedSource* audioSource;
FramedSource* videoSource;
RTPSink* audioSink;
RTPSink* videoSink;

void play(); // forward

			 // To stream using "source-specific multicast" (SSM), uncomment the following:
			 //#define USE_SSM 1
#ifdef USE_SSM
Boolean const isSSM = True;
#else
Boolean const isSSM = False;
#endif

// To set up an internal RTSP server, uncomment the following:
//#define IMPLEMENT_RTSP_SERVER 1
// (Note that this RTSP server works for multicast only)

// To stream *only* MPEG "I" frames (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

int main(int argc, char** argv) {
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);

	// Create 'groupsocks' for RTP and RTCP:
	char const* destinationAddressStr
#ifdef USE_SSM
		= "232.255.42.42";
#else
		= "239.255.42.42";
	// Note: This is a multicast address.  If you wish to stream using
	// unicast instead, then replace this string with the unicast address
	// of the (single) destination.  (You may also need to make a similar
	// change to the receiver program.)
#endif
	const unsigned short rtpPortNumAudio = 6666;
	const unsigned short rtcpPortNumAudio = rtpPortNumAudio + 1;
	const unsigned short rtpPortNumVideo = 8888;
	const unsigned short rtcpPortNumVideo = rtpPortNumVideo + 1;
	const unsigned char ttl = 7; // low, in case routers don't admin scope

	struct in_addr destinationAddress;
	destinationAddress.s_addr = our_inet_addr(destinationAddressStr);
	const Port rtpPortAudio(rtpPortNumAudio);
	const Port rtcpPortAudio(rtcpPortNumAudio);
	const Port rtpPortVideo(rtpPortNumVideo);
	const Port rtcpPortVideo(rtcpPortNumVideo);

	Groupsock rtpGroupsockAudio(*env, destinationAddress, rtpPortAudio, ttl);
	Groupsock rtcpGroupsockAudio(*env, destinationAddress, rtcpPortAudio, ttl);
	Groupsock rtpGroupsockVideo(*env, destinationAddress, rtpPortVideo, ttl);
	Groupsock rtcpGroupsockVideo(*env, destinationAddress, rtcpPortVideo, ttl);
#ifdef USE_SSM
	rtpGroupsockAudio.multicastSendOnly();
	rtcpGroupsockAudio.multicastSendOnly();
	rtpGroupsockVideo.multicastSendOnly();
	rtcpGroupsockVideo.multicastSendOnly();
#endif

	// Create a 'MPEG Audio RTP' sink from the RTP 'groupsock':
	audioSink = MPEG1or2AudioRTPSink::createNew(*env, &rtpGroupsockAudio);

	// Create (and start) a 'RTCP instance' for this RTP sink:
	const unsigned estimatedSessionBandwidthAudio = 160; // in kbps; for RTCP b/w share
	const unsigned maxCNAMElen = 100;
	unsigned char CNAME[maxCNAMElen + 1];
	gethostname((char*)CNAME, maxCNAMElen);
	CNAME[maxCNAMElen] = '\0'; // just in case
#ifdef IMPLEMENT_RTSP_SERVER
	RTCPInstance* audioRTCP =
#endif
		RTCPInstance::createNew(*env, &rtcpGroupsockAudio,
			estimatedSessionBandwidthAudio, CNAME,
			audioSink, NULL , isSSM);
	// Note: This starts RTCP running automatically

	// Create a 'MPEG Video RTP' sink from the RTP 'groupsock':
	videoSink = MPEG1or2VideoRTPSink::createNew(*env, &rtpGroupsockVideo);

	// Create (and start) a 'RTCP instance' for this RTP sink:
	const unsigned estimatedSessionBandwidthVideo = 4500; // in kbps; for RTCP b/w share
#ifdef IMPLEMENT_RTSP_SERVER
	RTCPInstance* videoRTCP =
#endif
		RTCPInstance::createNew(*env, &rtcpGroupsockVideo,
			estimatedSessionBandwidthVideo, CNAME,
			videoSink, NULL , isSSM);
	// Note: This starts RTCP running automatically

#ifdef IMPLEMENT_RTSP_SERVER
	RTSPServer* rtspServer = RTSPServer::createNew(*env);
	// Note that this (attempts to) start a server on the default RTSP server
	// port: 554.  To use a different port number, add it as an extra
	// (optional) parameter to the "RTSPServer::createNew()" call above.
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}
	ServerMediaSession* sms
		= ServerMediaSession::createNew(*env, "testStream", inputFileName,
			"Session streamed by \"testMPEG1or2AudioVideoStreamer\"",
			isSSM);
	sms->addSubsession(PassiveServerMediaSubsession::createNew(*audioSink, audioRTCP));
	sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, videoRTCP));
	rtspServer->addServerMediaSession(sms);

	char* url = rtspServer->rtspURL(sms);
	*env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
#endif

	// Finally, start the streaming:
	*env << "Beginning streaming...\n";
	play();

	env->taskScheduler().doEventLoop(); // does not return

	return 0; // only to prevent compiler warning
}

void afterPlaying(void* clientData) {
	// One of the sinks has ended playing.
	// Check whether any of the sources have a pending read.  If so,
	// wait until its sink ends playing also:
	if (audioSource->isCurrentlyAwaitingData()
		|| videoSource->isCurrentlyAwaitingData()) return;

	// Now that both sinks have ended, close both input sources,
	// and start playing again:
	*env << "...done reading from file\n";

	audioSink->stopPlaying();
	videoSink->stopPlaying();
	// ensures that both are shut down
	Medium::close(audioSource);
	Medium::close(videoSource);
	Medium::close(mpegDemux);
	// Note: This also closes the input file that this source read from.

	// Start playing once again:
	play();
}

void play() {
	// Open the input file as a 'byte-stream file source':
	ByteStreamFileSource* fileSource
		= ByteStreamFileSource::createNew(*env, inputFileName);
	if (fileSource == NULL) {
		*env << "Unable to open file \"" << inputFileName
			<< "\" as a byte-stream file source\n";
		exit(1);
	}

	// We must demultiplex Audio and Video Elementary Streams
	// from the input source:
	mpegDemux = MPEG1or2Demux::createNew(*env, fileSource);
	FramedSource* audioES = mpegDemux->newAudioStream();
	FramedSource* videoES = mpegDemux->newVideoStream();

	// Create a framer for each Elementary Stream:
	audioSource
		= MPEG1or2AudioStreamFramer::createNew(*env, audioES);
	videoSource
		= MPEG1or2VideoStreamFramer::createNew(*env, videoES, iFramesOnly);

	// Finally, start playing each sink.
	*env << "Beginning to read from file...\n";
	//videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
	audioSink->startPlaying(*audioSource, afterPlaying, audioSink);
}

*/