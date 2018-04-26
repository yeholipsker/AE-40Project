#pragma once
#include <mfapi.h>
#include <mfidl.h>
#include <iostream>
#include <Mfreadwrite.h>

class Media
{
public:
	Media();
	void createMediaFile();//TODO change name
	void EnumerateDevices(GUID deviceType);
	void CreateSinkWriter(DWORD* pVideoOutStreamIndex, DWORD* pAudioOutStreamIndex);
	void WriteToFile(DWORD vidStreamIndex, DWORD audStreamIndex);
	HRESULT CreateVideoMediaTypeOut(IMFMediaType** pVidMediaTypeOut);
	HRESULT CreateAudioMediaTypeOut(IMFMediaType** pAudMediaTypeOut);
	HRESULT ReadWriteSample(int i, LONGLONG* baseTimeSamp, DWORD readStreamIndex, DWORD writeStreamIndex);
	~Media();
private:
	IMFMediaSource* m_pVIDSource;
	IMFMediaSource* m_pAUDSource;
	IMFMediaSource* m_pAggSource;
	IMFSourceReader* m_pReader;
	IMFSinkWriter* m_pSinkWriter;
};

