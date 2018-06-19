#include "stdafx.h"
#include "MPEG2LiveSource.h"


// Static members
EventTriggerId MPEG2LiveSource::m_eventTriggerId = 0;
unsigned MPEG2LiveSource::m_referenceCount = 0;
bool isInitialized1 = false;

MPEG2LiveSource* MPEG2LiveSource::createNew(UsageEnvironment &env)
{
	return new MPEG2LiveSource(env);
}

MPEG2LiveSource::MPEG2LiveSource(UsageEnvironment& env) : FramedSource(env)
{
	if (m_referenceCount == 0)
	{
	}
	++m_referenceCount;

	if (m_eventTriggerId == 0)
	{
		m_eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
	}
}

MPEG2LiveSource::~MPEG2LiveSource()
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

void MPEG2LiveSource::doGetNextFrame()
{
	//std::cout << "MPEG2LiveSource::doGetNextFrame()" << std::endl;
	IMFSample* pSample = NULL;
	IMFSample* ppSampleOut = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	DWORD pBuffLength = NULL;
	LONGLONG timeStamp = NULL;
	HRESULT hr = S_OK;
	BYTE* byteArray = NULL;

	if (!isInitialized1)
	{
		isInitialized1 = true;
		initialize();
	}

	//while (byteArray == NULL)
	//{
		pSample = NULL;
		ppSampleOut = NULL;
		stIndex = NULL;
		flags = NULL;
		timeStamp = NULL;
		hr = S_OK;
		m_media->getSourceReader()->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0, &stIndex, &flags, &timeStamp, &pSample);
		if (pSample)
		{
			m_encoder->TransformAudioSample(pSample, &ppSampleOut, &byteArray, &pBuffLength);
			SafeRelease(pSample);
		}
	//}
		if (byteArray)
		{
			std::pair <BYTE*, DWORD> myPair(byteArray, pBuffLength);
			myQ->push(myPair);
			SafeRelease(&ppSampleOut);
		}
		gettimeofday(&m_currentTime, NULL);
		deliverFrame();
}

bool MPEG2LiveSource::initialize()
{
	m_media = new Media();
	m_media->InitializeSource();
	m_encoder = new Encoder();
	m_encoder->InitializeAudioEncoder(NULL); // TODO - SEND REAL MEDIA TYPE.
											 //m_encoder->InitializeAudioEncoder(NULL);
	myQ = new std::queue<std::pair <BYTE*, DWORD>>();
	return true;
}

void MPEG2LiveSource::deliverFrame0(void* clientData)
{
	((MPEG2LiveSource*)clientData)->deliverFrame();
}

void MPEG2LiveSource::deliverFrame()
{
	if (!isCurrentlyAwaitingData())
	{
		return;
	}
	if (!myQ->empty())
	{
		std::pair <BYTE*, DWORD> myPair = myQ->front();
		myQ->pop();
		fPresentationTime = m_currentTime;
		fFrameSize = myPair.second;
		if (fFrameSize > fMaxSize) fFrameSize = fMaxSize;
		memmove(fTo, myPair.first, fFrameSize);
	}
	FramedSource::afterGetting(this);
}