#include "stdafx.h"
#include "Media.h"
#include <Mferror.h>

#define CHECK_HR(hr, msg) if (hr != S_OK) std::cout << "FAILED!!\t" << msg << std::endl; else std::cout << "SUCCEEDED\t" << msg << std::endl;

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
	HRESULT hr = S_OK;
	CoInitialize(NULL);
	MFStartup(MF_VERSION);

	// Get the device lists.
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
	EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

	std::cout << "EnumerateDevices for both audio & video is done." << std::endl;

	// Create a collection of audio & video sources.
	IMFCollection* pCollection = NULL;
	CHECK_HR(MFCreateCollection(&pCollection), "Create Collection");
	CHECK_HR(pCollection->AddElement(m_pAUDSource),"add audio element");
	CHECK_HR(pCollection->AddElement(m_pVIDSource), "add video element");
	// Aggregate the audio & video sources to one source.
	CHECK_HR(MFCreateAggregateSource(pCollection, &m_pAggSource), "MFCreateAggregateSource");
	// Create source reader.
	CHECK_HR(MFCreateSourceReaderFromMediaSource(m_pAggSource, NULL, &m_pReader), "MFCreateSourceReaderFromMediaSource");
	//HRESULT hr = MFCreateSourceReaderFromMediaSource(m_pVIDSource, NULL, &m_pReader);

	if (SUCCEEDED(hr))
	{
		// Create sink writer.
		DWORD vidStreamIndex = NULL;
		DWORD audStreamIndex = NULL;
		CreateSinkWriter(&vidStreamIndex,&audStreamIndex);

		// Write media to a file.
		WriteToFile(vidStreamIndex, audStreamIndex);

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

void Media::CreateSinkWriter(DWORD *pVideoOutStreamIndex, DWORD *pAudioOutStreamIndex)
{	
	IMFMediaType* pVidMediaTypeIn = NULL;
	IMFMediaType* pVidMediaTypeOut = NULL;
	IMFMediaType* pAudMediaTypeIn = NULL;
	IMFMediaType* pAudMediaTypeOut = NULL;
	DWORD videoOutStreamIndex = NULL;
	DWORD audioOutStreamIndex = NULL;
	DWORD videoInStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
	DWORD audioInStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;

	//create sink writer
	HRESULT hr = MFCreateSinkWriterFromURL(L"output.wmv", NULL, NULL, &m_pSinkWriter);
	
	//create output video media type and add it to stream
	CHECK_HR(CreateVideoMediaTypeOut(&pVidMediaTypeOut),"CreateVideoMediaTypeOut");
	CHECK_HR(m_pSinkWriter->AddStream(pVidMediaTypeOut, &videoOutStreamIndex),"Add Video Stream");
	//get deafult input video mediatype and set it to the stream
	CHECK_HR(m_pReader->GetCurrentMediaType(videoInStreamIndex, &pVidMediaTypeIn),"GetCurrentMediaType Video");
	CHECK_HR(m_pSinkWriter->SetInputMediaType(videoOutStreamIndex, pVidMediaTypeIn, NULL),"SetInputMediaType Video");

	//create output audio media type and add it to stream
	CHECK_HR(CreateAudioMediaTypeOut(&pAudMediaTypeOut), "CreateAudioMediaTypeOut");
	CHECK_HR(m_pSinkWriter->AddStream(pAudMediaTypeOut, &audioOutStreamIndex), "Add Audio Stream");
	//get deafult input audio mediatype and set it to the stream
	CHECK_HR(m_pReader->GetCurrentMediaType(audioInStreamIndex, &pAudMediaTypeIn),"GetCurrentMediaType Audio");
	hr = m_pSinkWriter->SetInputMediaType(audioOutStreamIndex, pAudMediaTypeIn, NULL);
	//CHECK_HR(m_pSinkWriter->SetInputMediaType(audioOutStreamIndex, pAudMediaTypeIn, NULL), "SetInputMediaType Audio");

	if (SUCCEEDED(hr))
	{
		hr = m_pSinkWriter->BeginWriting();
	}

	if (SUCCEEDED(hr))
	{
		*pVideoOutStreamIndex = videoOutStreamIndex;
	}
}

void Media::WriteToFile(DWORD vidStreamIndex, DWORD audStreamIndex)
{
	LONGLONG baseTimeSamp = NULL;

	for (int i = 0; i < 100; i++)
	{
		ReadWriteSample(i, &baseTimeSamp,MF_SOURCE_READER_FIRST_AUDIO_STREAM, audStreamIndex);
		ReadWriteSample(i, &baseTimeSamp, MF_SOURCE_READER_FIRST_VIDEO_STREAM, vidStreamIndex);
	}

	CHECK_HR(m_pSinkWriter->Finalize(), "Finalize");
}

HRESULT Media::CreateVideoMediaTypeOut(IMFMediaType ** pMediaTypeOut)
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

	return hr;
}

HRESULT Media::CreateAudioMediaTypeOut(IMFMediaType ** pAudMediaTypeOut)
{
	const UINT32 AUDIO_PROFILE_LEVEL = 0;
	const UINT32 AUDIO_BIT_RATE = 128000;
	const UINT32 AUDIO_BIT_DEPTH = 16;
	const UINT32 CHANNEL_NUM = 2;
	const UINT32 SAMPLE_RATE = 44100;

	CHECK_HR(MFCreateMediaType(pAudMediaTypeOut), "MFCreateMediaType Audio");

	CHECK_HR((*pAudMediaTypeOut)->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio), "Set major type Audio");

	CHECK_HR((*pAudMediaTypeOut)->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float), "Set sub type Audio");
	
	//CHECK_HR((*pAudMediaTypeOut)->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, AUDIO_PROFILE_LEVEL), "set profile level Audio" );

	CHECK_HR((*pAudMediaTypeOut)->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, AUDIO_BIT_RATE), "set bit rate Audio");

	//CHECK_HR((*pAudMediaTypeOut)->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, AUDIO_BIT_DEPTH), "set bit depth Audio");
	
	//CHECK_HR((*pAudMediaTypeOut)->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, SPEAKER_ALL), "set channel mask Audio");

	CHECK_HR((*pAudMediaTypeOut)->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, CHANNEL_NUM), "set channel num Audio");

	CHECK_HR((*pAudMediaTypeOut)->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, SAMPLE_RATE), "set sample per second Audio");
	
	return S_OK;
}

HRESULT Media::ReadWriteSample(int i, LONGLONG* baseTimeSamp, DWORD readStreamIndex, DWORD writeStreamIndex)
{
	IMFSample* pSample = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	LONGLONG timeStamp = NULL;

	HRESULT hr = m_pReader->ReadSample(readStreamIndex, 0, &stIndex, &flags, &timeStamp, &pSample);
	if (i == 0)
	{
		*baseTimeSamp = timeStamp;
	}
	if (SUCCEEDED(hr) && pSample)
	{
		hr = pSample->SetSampleTime(timeStamp - *baseTimeSamp);
	}
	if (SUCCEEDED(hr) && pSample)
	{
		hr = m_pSinkWriter->WriteSample(writeStreamIndex, pSample);
		std::cout << "Wrote sample!! i = " << i << std::endl;
	}
	if (pSample)
	{
		pSample->Release();
		pSample = NULL;
	}
	return S_OK;
}

Media::~Media() { }