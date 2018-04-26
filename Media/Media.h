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
	HRESULT EnumerateTypesForStream(IMFSourceReader* pReader, DWORD dwStreamIndex);
	void CreateSinkWriter(DWORD* pVideoOutStreamIndex);
	void WriteToFile(DWORD* pStreamIndex);
	void CreateVideoMediaTypeOut(IMFMediaType** pMediaTypeOut);
	~Media();
private:
	IMFMediaSource* m_pVIDSource;
	IMFMediaSource* m_pAUDSource;
	IMFMediaSource* m_pAggSource;
	IMFSourceReader* m_pReader;
	IMFSinkWriter* m_pSinkWriter;
};

