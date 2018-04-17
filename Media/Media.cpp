#include "stdafx.h"
#include "Media.h"

Media::Media()
{
	m_pVIDSource = NULL;
	m_pVIDAttributes = NULL;
	m_ppVIDDevices = NULL;
	m_pAUDSource = NULL;
	m_pAUDAttributes = NULL;
	m_ppAUDDevices = NULL;

	m_pReader = NULL;
}

void Media::createMediaFile()
{
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID, m_pAUDSource, m_pAUDAttributes, m_ppAUDDevices);
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID, m_pVIDSource, m_pVIDAttributes, m_ppVIDDevices);

	std::cout << "EnumerateDevices for both audio & video is done." << std::endl;

	CreateSourceReader(m_pAUDSource);
	CreateSourceReader(m_pVIDSource);

	std::cout << "MFCreateSourceReaderFromMediaSource for both audio & video is done." << std::endl;
}

void Media::EnumerateDevices(
	GUID deviceType,
	IMFMediaSource* m_pSource,
	IMFAttributes* m_pAttributes,
	IMFActivate** m_ppDevices)
{
	UINT32 count = 0;

	CoInitialize(NULL);

	HRESULT hr = MFCreateAttributes(&m_pAttributes, 1);
	if (FAILED(hr))
	{
		std::cout << "fail at MFCreateAttributes" << std::endl;
	}

	hr = m_pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, deviceType);
	if (FAILED(hr))
	{
		std::cout << "fail at SetGUID" << std::endl;
	}

	hr = MFEnumDeviceSources(m_pAttributes, &m_ppDevices, &count);
	if (FAILED(hr))
	{
		std::cout << "fail at MFEnumDeviceSources	`" << std::endl;
	}

	hr = m_ppDevices[0]->ActivateObject(IID_PPV_ARGS(&m_pSource));
	if (FAILED(hr))
	{
		std::cout << "fail at ActivateObject" << std::endl;
	}

	for (DWORD i = 0; i < count; i++)
	{
		HRESULT hr = S_OK;
		WCHAR *szFriendlyName = NULL;

		// Try to get the display name.
		UINT32 cchName;
		hr = m_ppDevices[i]->GetAllocatedString(
			MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
			&szFriendlyName, &cchName);

		if (SUCCEEDED(hr))
		{
			OutputDebugString(szFriendlyName);
			OutputDebugString(L"\n");
		}
		CoTaskMemFree(szFriendlyName);
	}
}

void Media::CreateSourceReader(IMFMediaSource * m_pAUDSource)
{
	HRESULT hr = MFCreateSourceReaderFromMediaSource(m_pAUDSource, NULL, &m_pReader);

	if (SUCCEEDED(hr))
	{
		//CreateMediaFileForReader(m_pReader); TODO - IMPLEMENT.
		m_pReader->Release();
	}
}

Media::~Media()
{
}
