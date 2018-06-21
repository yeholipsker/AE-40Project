#include "stdafx.h"
#include "MPEG2LiveSource.h"
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// Static members
EventTriggerId MPEG2LiveSource::m_eventTriggerId = 0;
unsigned MPEG2LiveSource::m_referenceCount = 0;

MPEG2LiveSource* MPEG2LiveSource::createNew(UsageEnvironment &env)
{
	return new MPEG2LiveSource(env);
}

void MPEG2LiveSource::PushToQueue(std::pair<BYTE*, DWORD> myPair)
{
	EnterCriticalSection(&CriticalSection);
	myQueue->push(myPair);
	LeaveCriticalSection(&CriticalSection);
}

MPEG2LiveSource::MPEG2LiveSource(UsageEnvironment& env) : FramedSource(env)
{
	InitializeCriticalSection(&CriticalSection);
	if (m_referenceCount == 0)
	{
		EnterCriticalSection(&CriticalSection);
		myQueue = new std::queue<std::pair <BYTE*, DWORD>>();
		LeaveCriticalSection(&CriticalSection);
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
		envir().taskScheduler().deleteEventTrigger(m_eventTriggerId);
		m_eventTriggerId = 0;
	}
	EnterCriticalSection(&CriticalSection);
	while (!myQueue->empty())
	{
		std::pair<BYTE*, DWORD> myPair = myQueue->front();
		myQueue->pop();
		delete myPair.first;
	}
	delete myQueue;
	LeaveCriticalSection(&CriticalSection);
}

void MPEG2LiveSource::doGetNextFrame()
{
		deliverFrame();
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
	EnterCriticalSection(&CriticalSection);
	if (!myQueue->empty())
	{
		std::pair <BYTE*, DWORD> myPair = myQueue->front();
		myQueue->pop();
		LeaveCriticalSection(&CriticalSection);
		if (myPair.first)
		{
			gettimeofday(&fPresentationTime, NULL);
			fFrameSize = myPair.second;
			if (fFrameSize > fMaxSize) fFrameSize = fMaxSize;
			memmove(fTo, myPair.first, fFrameSize);
			delete myPair.first;
			FramedSource::afterGetting(this);
		}
	}
	else {
		LeaveCriticalSection(&CriticalSection);
	}
}