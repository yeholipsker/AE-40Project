#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include "Rtp.h"

#define FREQUENCY						10.0
#define PORT							5004

Rtp::Rtp(int portBase)
{
#ifdef RTP_SOCKETTYPE_WINSOCK
	WSADATA dat;
	WSAStartup(MAKEWORD(2, 2), &dat);
#endif // RTP_SOCKETTYPE_WINSOCK

	// Set parameters for a new session.
	m_sessionParams.SetOwnTimestampUnit(1.0 / FREQUENCY);
	m_sessionParams.SetAcceptOwnPackets(true);
	m_transParams.SetPortbase(portBase);
	// Create the session.
	int status = m_session.Create(m_sessionParams, &m_transParams);
	checkerror(status);
}

int Rtp::SetAddress(std::string ipString)
{
	// Set the destination address.
	// "inet_addr" function returns a value in network byte order.
	uint32_t destip = inet_addr(ipString.c_str());
	// we need the IP address in host byte order, so we call to "ntohl".
	destip = ntohl(destip);
	RTPIPv4Address addr(destip, PORT);
	// Add the adress to the session.
	int status = m_session.AddDestination(addr);
	return status;
}

int Rtp::SendData(void* dataToSend, size_t length, uint8_t payloadType)
{
	// Set defaults.
	m_session.SetDefaultPayloadType(payloadType);
	m_session.SetDefaultMark(false);
	m_session.SetDefaultTimestampIncrement(160);

	// Send the data.
	std::cout << "Sent packet!" << std::endl;
	int status = m_session.SendPacket(dataToSend, length);
	return status;
}

HRESULT Rtp::ReceiveData(void** data)
{
	// Lock the access to the data.
	m_session.BeginDataAccess();
	if (m_session.GotoFirstSourceWithData())
	{
		// Get the packet.
		RTPPacket* packet = m_session.GetNextPacket();
		if (!packet)
		{
			return S_FALSE;
		}
		*data = packet;
		// Delete the packet.
		m_session.DeletePacket(packet);
	}
	// Unlock the access to the data.
	m_session.EndDataAccess();
	return S_OK;
}

void Rtp::checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

Rtp::~Rtp() { }