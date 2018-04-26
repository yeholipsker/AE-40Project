#include "stdafx.h"
#include "Media.h"
#include <Mferror.h>

Media::Media()
{
	m_pVIDSource = NULL;
	m_pAUDSource = NULL;
	m_pAggSource = NULL;
	m_pReader = NULL;
	m_pSinkWriter = NULL;
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
	// TODO - DO THIS FUNCTION.
	//MFCreateAggregateSource(pCollection, &m_pAggSource);

	std::cout << "Create aggregate source for both audio & video is done." << std::endl;

	// Create source reader.
	// TODO - SWITCH TO AGG.
	//HRESULT hr = MFCreateSourceReaderFromMediaSource(m_pAggSource, NULL, &m_pReader);
	HRESULT hr = MFCreateSourceReaderFromMediaSource(m_pVIDSource, NULL, &m_pReader);

	if (SUCCEEDED(hr))
	{
		std::cout << "Create source reader is done." << std::endl;

		// Create sink writer.
		DWORD vidStreamIndex = NULL;
		DWORD audStreamIndex = NULL;
		CreateSinkWriter(&vidStreamIndex);

		// Write media to a file.
		WriteToFile(&vidStreamIndex);

		// Release the reader.
		m_pReader->Release();
	}
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

// TODO - REMOVE??
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

void Media::CreateSinkWriter(DWORD *pVideoOutStreamIndex)
{	
	IMFMediaType* pMediaTypeIn = NULL;
	IMFMediaType* pMediaTypeOut = NULL;
	DWORD videoOutStreamIndex = NULL;
	DWORD videoInStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;

	//create sink writer
	HRESULT hr = MFCreateSinkWriterFromURL(L"output.wmv", NULL, NULL, &m_pSinkWriter);
	
	//create output video media type and add it to stream
	if (SUCCEEDED(hr))
	{
		CreateVideoMediaTypeOut(&pMediaTypeOut);
	}
	
	if (SUCCEEDED(hr))
	{
		hr = m_pSinkWriter->AddStream(pMediaTypeOut, &videoOutStreamIndex);
	}

	//get deafult input video mediatype and set it to the stream
	if (SUCCEEDED(hr))
	{
		hr = m_pReader->GetCurrentMediaType(videoInStreamIndex, &pMediaTypeIn);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pSinkWriter->SetInputMediaType(videoOutStreamIndex, pMediaTypeIn, NULL);
	}

	//create output audio media type and add it to stream
	//get deafult input audio mediatype and set it to the stream

	if (SUCCEEDED(hr))
	{
		hr = m_pSinkWriter->BeginWriting();
	}

	if (SUCCEEDED(hr))
	{
		*pVideoOutStreamIndex = videoOutStreamIndex;
	}
}

void Media::WriteToFile(DWORD* pStreamIndex)
{
	IMFSample* pSample = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	LONGLONG llTstamp = NULL;
	LONGLONG llBaseTimeSamp = NULL;
	for (int i = 0; i < 100; i++)
	{
		HRESULT hr = m_pReader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, &stIndex, &flags, &llTstamp, &pSample);
		if (i == 0)
		{
			llBaseTimeSamp = llTstamp;
		}
		if (SUCCEEDED(hr) && pSample)
		{
			hr = pSample->SetSampleTime(llTstamp - llBaseTimeSamp);
		}
		if (SUCCEEDED(hr) && pSample)
		{
			hr = m_pSinkWriter->WriteSample(*pStreamIndex, pSample);
			std::cout << "Wrote sample!! i = " << i << std::endl;
		}
		if (pSample)
		{
			pSample->Release();
			pSample = NULL;
		}
	}
	
	HRESULT hr = m_pSinkWriter->Finalize();
	if (SUCCEEDED(hr))
	{
		std::cout << "finalized!" << std::endl;
	}
}

void Media::CreateVideoMediaTypeOut(IMFMediaType ** pMediaTypeOut)
{
	// Format constants
	const UINT32 VIDEO_WIDTH = 640;
	const UINT32 VIDEO_HEIGHT = 480;
	const UINT32 VIDEO_FPS = 30;
	const UINT64 VIDEO_FRAME_DURATION = 10 * 1000 * 1000 / VIDEO_FPS;
	const UINT32 VIDEO_BIT_RATE = 800000;
	const GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_WMV3;
	const GUID   VIDEO_INPUT_FORMAT = MFVideoFormat_RGB32;
	const UINT32 VIDEO_PELS = VIDEO_WIDTH * VIDEO_HEIGHT;
	const UINT32 VIDEO_FRAME_COUNT = 20 * VIDEO_FPS;
	
	// Set the output media type.
	HRESULT hr = MFCreateMediaType(pMediaTypeOut);

	if (SUCCEEDED(hr))
	{
		hr = (*pMediaTypeOut)->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	if (SUCCEEDED(hr))
	{
		hr = (*pMediaTypeOut)->SetGUID(MF_MT_SUBTYPE, VIDEO_ENCODING_FORMAT);
	}

	if (SUCCEEDED(hr))
	{
		hr = (*pMediaTypeOut)->SetUINT32(MF_MT_AVG_BITRATE, VIDEO_BIT_RATE);
	}
	if (SUCCEEDED(hr))
	{
		hr = (*pMediaTypeOut)->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeSize(*pMediaTypeOut, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(*pMediaTypeOut, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(*pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
}

Media::~Media() { }