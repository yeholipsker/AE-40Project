#pragma once
#include "Encoder.h"

class Media
{
public:
	Media();
	void	InitializeSource();
	void	StopRecording();
	// Getter
	IMFSourceReader* getSourceReader() { return m_pReader; }
	~Media();

private:
	// Members
	IMFMediaSource*		m_pVIDSource;
	IMFMediaSource*		m_pAUDSource;
	IMFMediaSource*		m_pAggSource;
	IMFSourceReader*	m_pReader;
	IMFSinkWriter*		m_pSinkWriter;
	bool				m_stopRecording;

	// Methods
	HRESULT	EnumerateDevicesAndActivateSource(GUID deviceType);
	HRESULT CreateAggregatedSourceReader();
	HRESULT SetSourceReaderMediaTypes();
};

