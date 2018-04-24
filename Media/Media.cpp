#include "stdafx.h"
#include "Media.h"
#include <Mferror.h>

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
	//MFCreateAggregateSource(pCollection, &pAggSource);

	std::cout << "Create aggregate source for both audio & video is done." << std::endl;

	// Create source reader for the media source. 
	//CreateSourceReader(pAggSource); // TODO - Put inside an 'if SUCCEEDED' statement.
	CreateSourceReader(m_pAUDSource);

	std::cout << "Create source reader done." << std::endl;
}

HRESULT Media::EnumerateTypesForStream(IMFSourceReader *pReader, DWORD dwStreamIndex)
{
	HRESULT hr = S_OK;
	DWORD dwMediaTypeIndex = 0;

	while (SUCCEEDED(hr))
	{
		IMFMediaType *pType = NULL;
		hr = m_pReader->GetNativeMediaType(dwStreamIndex, dwMediaTypeIndex, &pType);
		if (hr == MF_E_NO_MORE_TYPES)
		{
			hr = S_OK;
			break;
		}
		else if (SUCCEEDED(hr))
		{
			// Examine the media type. (Not shown.)

			pType->Release();
		}
		++dwMediaTypeIndex;
	}
	return hr;
}

void Media::EnumerateDevices(GUID deviceType)
{
	IMFAttributes* pAttributes = NULL;
	IMFActivate** ppDevices = NULL;
	IMFMediaSource* pMediaSource = NULL;
	UINT32 count = 0;
	HRESULT hr = MFCreateAttributes(&pAttributes, 1);

	if (FAILED(hr))
	{
		std::cout << "fail at MFCreateAttributes. hresult = " << hr << std::endl;
	}

	hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, deviceType);
	if (FAILED(hr))
	{
		std::cout << "fail at SetGUID. hresult = " << hr << std::endl;
	}

	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
	if (FAILED(hr))
	{
		std::cout << "fail at MFEnumDeviceSources. hresult = " << hr << std::endl;
	}
	
	hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pMediaSource));
	if (FAILED(hr))
	{
		std::cout << "fail at ActivateObject. hresult = " << hr << std::endl;
	}

	if (deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
	{
		m_pAUDSource = pMediaSource;
	}
	else if(deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
	{
		m_pVIDSource = pMediaSource;
	}

	for (DWORD i = 0; i < count; i++)
	{
		HRESULT hr = S_OK;
		WCHAR *szFriendlyName = NULL;

		// Try to get the display name.
		UINT32 cchName;
		hr = ppDevices[i]->GetAllocatedString(
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

void Media::CreateSourceReader(IMFMediaSource * pSource)
{
	HRESULT hr = MFCreateSourceReaderFromMediaSource(pSource, NULL, &m_pReader);

	if (SUCCEEDED(hr))
	{
		//CreateMediaFileForReader(m_pReader); TODO - IMPLEMENT.

		IMFMediaType *pType = NULL;
		DWORD dwStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;

		m_pReader->GetCurrentMediaType(dwStreamIndex, &pType);

		HRESULT hr = EnumerateTypesForStream(m_pReader, dwStreamIndex);

		//m_pReader->Release();
	}
	else
	{

	}
}

Media::~Media()
{
}
