#pragma once
#include <mfapi.h>
#include <mfidl.h>
#include <iostream>
#include <Mfreadwrite.h>

class Media
{
public:
	Media();
	void	StartRecordingToFile();
	void	StopRecording();
	~Media();

private:
	IMFMediaSource*		m_pVIDSource;
	IMFMediaSource*		m_pAUDSource;
	IMFMediaSource*		m_pAggSource;
	IMFSourceReader*	m_pReader;
	IMFSinkWriter*		m_pSinkWriter;
	bool				m_stopRecording;

	HRESULT	EnumerateDevicesAndActivateSource(GUID deviceType);
	HRESULT	CreateSinkWriter(DWORD* pVideoOutStreamIndex, DWORD* pAudioOutStreamIndex);
	HRESULT WriteToFile(DWORD vidStreamIndex, DWORD audStreamIndex);
	HRESULT CreateVideoMediaTypeOut(IMFMediaType** pVidMediaTypeOut);
	HRESULT ReadWriteSample(int i, LONGLONG* baseTimeSamp, DWORD readStreamIndex, DWORD writeStreamIndex);
	HRESULT CreateAggregatedSourceReader();
	HRESULT SetSourceReaderAudioMediaType();
};

