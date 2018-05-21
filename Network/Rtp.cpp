#include "stdafx.h"
#include "Rtp.h"
#include "jthread\jthread.h"
#include "jrtplib3\rtpsession.h"
#include "jrtplib3\rtpsessionparams.h"
#include "jrtplib3\rtpudpv4transmitter.h"
#include <iostream>

using namespace jrtplib;

#define FREQUENCY		8000.0
#define FREQUENCY_INT	8000

Rtp::Rtp()
{
	RTPSession session;
	RTPSessionParams sessionParams;
	sessionParams.SetOwnTimestampUnit(1.0 / FREQUENCY);
	RTPUDPv4TransmissionParams transParams;  // TODO - CHANGE TO TCP
	transParams.SetPortbase(FREQUENCY_INT);

	int status = session.Create(sessionParams, &transParams);
	if (status < 0)
	{
		std::cerr << RTPGetErrorString(status) << std::endl;
		exit(-1);
	}
}


Rtp::~Rtp()
{
}