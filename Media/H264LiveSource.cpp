#include "stdafx.h"
#include "H264LiveSource.h"

// Static members
EventTriggerId H264LiveSource::m_eventTriggerId = 0;
unsigned H264LiveSource::m_referenceCount = 0;

H264LiveSource* H264LiveSource::createNew(UsageEnvironment &env)
{
	return new H264LiveSource(env);
}

H264LiveSource::H264LiveSource(UsageEnvironment& env) : FramedSource(env)
{
	if (m_referenceCount == 0)
	{
		m_media = new Media();
		m_media->InitializeSource();
		m_encoder = new Encoder();
		m_encoder->InitializeVideoEncoder(NULL); // TODO - SEND REAL MEDIA TYPE.
		//m_encoder->InitializeAudioEncoder(NULL);
		myQ = new std::queue<std::pair <BYTE*, DWORD>>();
	}
	++m_referenceCount;

	if (m_eventTriggerId == 0)
	{
		m_eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
	}
}

H264LiveSource::~H264LiveSource()
{
	--m_referenceCount;
	if (m_referenceCount == 0)
	{
		delete m_media;
		delete m_encoder;
		envir().taskScheduler().deleteEventTrigger(m_eventTriggerId);
		m_eventTriggerId = 0;
	}
}

void H264LiveSource::doGetNextFrame()
{
	IMFSample* pSample = NULL;
	IMFSample* ppSampleOut = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	DWORD pBuffLength = NULL;
	LONGLONG timeStamp = NULL;
	HRESULT hr = S_OK;

	BYTE* byteArray = NULL;
	while (byteArray == NULL)
	{
		pSample = NULL;
		ppSampleOut = NULL;
		stIndex = NULL;
		flags = NULL;
		timeStamp = NULL;
		hr = S_OK;
		m_media->getSourceReader()->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
											   0, &stIndex, &flags, &timeStamp, &pSample);
		if (pSample)
		{
			m_encoder->TransformVideoSample(pSample, &ppSampleOut, &byteArray, &pBuffLength);
			SafeRelease(pSample);
		}
	}

	std::pair <BYTE*, DWORD> myPair(byteArray, pBuffLength);
	myQ->push(myPair);
	gettimeofday(&m_currentTime, NULL);
	deliverFrame();
}

void H264LiveSource::deliverFrame0(void* clientData)
{
	((H264LiveSource*)clientData)->deliverFrame();
}

bool H264LiveSource::isH264VideoStreamFramer() const
{
	return true;
}

void H264LiveSource::deliverFrame()
{
	if (!isCurrentlyAwaitingData())
	{
		return;
	}

	std::pair <BYTE*, DWORD> myPair = myQ->front();
	myQ->pop();
	fPresentationTime = m_currentTime;
	fFrameSize = myPair.second;
	memmove(fTo, myPair.first, fFrameSize);
	FramedSource::afterGetting(this);
}
