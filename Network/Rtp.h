#pragma once
#include <string>
#include "jthread\jthread.h"
#include "jrtplib3\rtpsession.h"
#include "jrtplib3\rtpsessionparams.h"
#include "jrtplib3\rtpudpv4transmitter.h"
#include "Ws2tcpip.h"
#include "jrtplib3\rtppacket.h"

using namespace jrtplib;

class Rtp
{
public:
	// Constructor. Get the port for recieve data.
	Rtp(int portBase);
	// Set the destination. 
	int SetAddress(std::string ipString);
	// Send the data using RTP protocol.
	int SendData(void* dataToSend, size_t length, uint8_t payloadType);
	// Receive the data using RTP protocol.
	HRESULT ReceiveData(void** data);
	// Destructor.
	virtual ~Rtp();

private:
	// This function checks if there was a RTP error.If so, it displays an error message and exists.
	void checkerror(int rtperr);
	RTPSession m_session;
	RTPSessionParams m_sessionParams;
	RTPUDPv4TransmissionParams m_transParams;
};

