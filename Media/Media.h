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
	void CreateSourceReader(IMFMediaSource* m_pAUDSource);
	HRESULT EnumerateTypesForStream(IMFSourceReader* pReader, DWORD dwStreamIndex);
	void CreateSinkWriter(DWORD* pStreamIndex);
	void WriteToFile(DWORD* pStreamIndex);
	~Media();
private:
	IMFMediaSource* m_pVIDSource;
	IMFMediaSource* m_pAUDSource;
	IMFSourceReader* m_pReader;
	IMFSinkWriter* m_pSinkWriter;
};

