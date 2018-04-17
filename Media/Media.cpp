#include "stdafx.h"
#include "Media.h"

Media::Media()
{
	m_pVIDSource = NULL;
	m_pAUDSource = NULL;
	m_pReader = NULL;
}

void Media::createMediaFile()
{
	CoInitialize(NULL);
	MFStartup(MF_VERSION);

	// Get the device lists.
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

	std::cout << "EnumerateDevices for both audio & video is done." << std::endl;

	// Create a collection of audio & video sources.
	IMFCollection* pCollection = NULL;
	MFCreateCollection(&pCollection);
	pCollection->AddElement(m_pAUDSource);
	pCollection->AddElement(m_pVIDSource);

	// Aggregate the audio & video sources to one source.
	IMFMediaSource* pAggSource = NULL;
	MFCreateAggregateSource(pCollection, &pAggSource);

	// Create source reader for the media source.
	CreateSourceReader(pAggSource);

	std::cout << "Create aggregate source for both audio & video is done." << std::endl;
}

void Media::EnumerateDevices(GUID deviceType)
{
	IMFAttributes* m_pAttributes = NULL;
	IMFActivate** m_ppDevices = NULL;
	IMFMediaSource* m_pSource = NULL;

	UINT32 count = 0;

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
	
	if (deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
	{
		hr = m_ppDevices[0]->ActivateObject(IID_PPV_ARGS(&m_pAUDSource));
	}
	else if(deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
	{
		hr = m_ppDevices[0]->ActivateObject(IID_PPV_ARGS(&m_pVIDSource));
	}

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

void Media::CreateSourceReader(IMFMediaSource * m_pSource)
{
	HRESULT hr = MFCreateSourceReaderFromMediaSource(m_pSource, NULL, &m_pReader);

	if (SUCCEEDED(hr))
	{
		//CreateMediaFileForReader(m_pReader); TODO - IMPLEMENT.
		m_pReader->Release();
	}
}

Media::~Media()
{
}
