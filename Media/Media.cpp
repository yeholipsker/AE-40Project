#include "stdafx.h"
#include "Media.h"


Media::Media()
{
}

void Media::createMediaFile()
{
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
}

void Media::EnumerateDevices(GUID deviceType)
{
	IMFMediaSource *pSource = NULL;
	IMFAttributes *pAttributes = NULL;
	IMFActivate **ppDevices = NULL;
	UINT32 count = 0;

	CoInitialize(NULL);

	HRESULT hr = MFCreateAttributes(&pAttributes, 1);
	if (FAILED(hr))
	{
		std::cout << "fail at MFCreateAttributes" << std::endl;
	}

	hr = pAttributes->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		deviceType
	);
	if (FAILED(hr))
	{
		std::cout << "fail at SetGUID" << std::endl;
	}

	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
	if (FAILED(hr))
	{
		std::cout << "fail at MFEnumDeviceSources	`" << std::endl;
	}

	hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pSource));
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


Media::~Media()
{
}
