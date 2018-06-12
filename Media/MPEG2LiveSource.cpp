#include "stdafx.h"
#include "MPEG2LiveSource.h"


// Static members
EventTriggerId MPEG2LiveSource::m_eventTriggerId = 0;
unsigned MPEG2LiveSource::m_referenceCount = 0;
bool isInitialized1 = false;

MPEG2LiveSource* MPEG2LiveSource::createNew(UsageEnvironment &env)
{
	std::cout << "createNew" << std::endl;
	return new MPEG2LiveSource(env);
}

MPEG2LiveSource::MPEG2LiveSource(UsageEnvironment& env) : FramedSource(env)
{
	std::cout << "constructor, " << m_referenceCount << std::endl;
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
		std::cout << "if (!isInitialized)" << std::endl;
		isInitialized1 = true;
		initialize();
	}

	while (byteArray == NULL)
	{
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
			std::cout << "if (psample)" << std::endl;
			m_encoder->TransformAudioSample(pSample, &ppSampleOut, &byteArray, &pBuffLength);
			SafeRelease(pSample);
		}
	}

	std::pair <BYTE*, DWORD> myPair(byteArray, pBuffLength);
	myQ->push(myPair);
	gettimeofday(&m_currentTime, NULL);
	deliverFrame();
	SafeRelease(&ppSampleOut);
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

bool MPEG2LiveSource::isH264VideoStreamFramer() const
{
	return false;
}

void MPEG2LiveSource::deliverFrame()
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
	std::cout << "memmove(fTo, myPair.first, fFrameSize);" << std::endl;
	FramedSource::afterGetting(this);
}