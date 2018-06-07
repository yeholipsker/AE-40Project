#pragma once
#include <FramedSource.hh>
#include <DeviceSource.hh>

class H264LiveSource : public FramedSource
{
public:
	static DeviceSource* createNew(UsageEnvironment& env,
		DeviceParameters params);

public:
	static EventTriggerId eventTriggerId;
	
protected:
	H264LiveSource();
	virtual ~H264LiveSource();

private:
	// redefined virtual functions:
	virtual void doGetNextFrame();
	//virtual void doStopGettingFrames(); // optional

	static void deliverFrame0(void* clientData);
	void deliverFrame();

	static unsigned referenceCount; // used to count how many instances of this class currently exist
	DeviceParameters fParams;
};

