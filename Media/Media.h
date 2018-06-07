#pragma once
#include "Encoder.h"

class Media
{
public:
	Media();
	void	StartRecordingToFile();
	void	StopRecording();
	~Media();

private:
	// Members
	IMFMediaSource*		m_pVIDSource;
	IMFMediaSource*		m_pAUDSource;
	IMFMediaSource*		m_pAggSource;

	// Methods
	HRESULT	EnumerateDevicesAndActivateSource(GUID deviceType);
	HRESULT	CreateSinkWriter(DWORD* pVideoOutStreamIndex, DWORD* pAudioOutStreamIndex);
	static DWORD WINAPI WriteToFile(LPVOID lpParameter);
	HRESULT CreateVideoMediaTypeOut(IMFMediaType** pVidMediaTypeOut);
	static HRESULT ReadWriteSample(int i, LONGLONG* baseTimeSamp, DWORD readStreamIndex, DWORD writeStreamIndex);
	HRESULT CreateAggregatedSourceReader();
	HRESULT SetSourceReaderAudioMediaType();
};

