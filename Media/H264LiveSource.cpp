#include "stdafx.h"
#include "H264LiveSource.h"


DeviceSource * H264LiveSource::createNew(UsageEnvironment & env, DeviceParameters params)
{
	return nullptr;
}

H264LiveSource::H264LiveSource(UsageEnvironment& env) : FramedSource(env)
{
}


H264LiveSource::~H264LiveSource()
{
}

void H264LiveSource::doGetNextFrame()
{
}

void H264LiveSource::deliverFrame0(void * clientData)
{
}

void H264LiveSource::deliverFrame()
{
}
