#pragma once
#include <FramedSource.hh>
#include <DeviceSource.hh>
#include "Media.h"
#include <queue>
#include <GroupsockHelper.hh>

class H264LiveSource : public FramedSource
{
public:
	static H264LiveSource* createNew(UsageEnvironment& env);

public:
	static EventTriggerId m_eventTriggerId;
	
protected:
	H264LiveSource(UsageEnvironment& env);
	virtual ~H264LiveSource();

private:

	// redefined virtual functions:
	virtual void doGetNextFrame();
	bool initialize();
	void deliverFrame();
	//virtual void doStopGettingFrames(); // optional
	static void deliverFrame0(void* clientData);
	bool isH264VideoStreamFramer() const;

	// Members
	Media* m_media;
	Encoder* m_encoder;
	std::queue<std::pair<BYTE*, DWORD>> * myQ;
	static unsigned m_referenceCount; // used to count how many instances of this class currently exist
	DeviceParameters m_fParams;
	timeval m_currentTime;
	bool isInitialized;
};

