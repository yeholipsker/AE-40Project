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
	void EnumerateDevices(
		GUID deviceType,
		IMFMediaSource* m_pSource,
		IMFAttributes* m_pAttributes,
		IMFActivate** m_ppDevices);
	void CreateSourceReader(IMFMediaSource* m_pAUDSource);
	~Media();
private:
	IMFMediaSource* m_pVIDSource;
	IMFAttributes* m_pVIDAttributes;
	IMFActivate** m_ppVIDDevices;
	IMFMediaSource* m_pAUDSource;
	IMFAttributes* m_pAUDAttributes;
	IMFActivate** m_ppAUDDevices;
	IMFSourceReader* m_pReader;
};

