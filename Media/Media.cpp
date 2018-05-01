#include "stdafx.h"
#include "Media.h"
#include <Mferror.h>

#define CHECK_HR(hr, msg) if (hr != S_OK) std::cout << "FAILED!!\t" << msg << std::endl;// else std::cout << "SUCCEEDED\t" << msg << std::endl;
template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

template <class T> inline void SafeRelease(T*& pT)
{
	if (pT != NULL)
	{
		pT->Release();
		pT = NULL;
	}
}

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
	DWORD vidStreamIndex = NULL;
	DWORD audStreamIndex = NULL;

	//initialize COM & MF
	CHECK_HR(hr = CoInitialize(NULL),"CoInitialize");
	CHECK_HR(hr = MFStartup(MF_VERSION),"MFStartup");

	// Get the device lists and activate.
	CHECK_HR(hr = EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID),"EnumerateDevices audio");
	CHECK_HR(hr = EnumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID),"EnumerateDevices video");

	// Create aggregated audio & video sourceReader.
	CHECK_HR(hr = CreateAggregatedSourceReader(),"CreateAggregatedSource");

	//Set sourceReader compatible mediaType
	CHECK_HR(hr = SetSourceReaderAudioMediaType(),"SetSourceReaderAudioMediaType");

	// Create sink writer.
	CHECK_HR(hr = CreateSinkWriter(&vidStreamIndex,&audStreamIndex),"CreateSinkWriter");

	// Write media to a file.
	CHECK_HR(hr = WriteToFile(vidStreamIndex, audStreamIndex),"WriteToFile");

	// Release the reader & writer.
	m_pReader->Release();
	m_pSinkWriter->Release();

	//shutdown MF & COM
	CHECK_HR(hr = MFShutdown(),"MFShutdown");
	CoUninitialize();
}

HRESULT Media::EnumerateDevices(GUID deviceType)
{
	IMFAttributes* pAttributes = NULL;
	IMFActivate** ppDevices = NULL;
	IMFMediaSource* pMediaSource = NULL;
	UINT32 count = 0;
	HRESULT hr = S_OK;
	
	CHECK_HR(hr = MFCreateAttributes(&pAttributes, 1),"CreateAttributes");

	CHECK_HR(hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, deviceType),"set device type");
	
	CHECK_HR(hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count),"EnumDeviceSources");
	
	CHECK_HR(hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pMediaSource)),"ActivateObject");

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
		WCHAR *szFriendlyName = NULL;

		// Try to get the display name.
		UINT32 cchName;
		CHECK_HR(hr = ppDevices[i]->GetAllocatedString(
			MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
			&szFriendlyName, &cchName),"GetAllocatedString");

		if (SUCCEEDED(hr))
		{
			OutputDebugString(szFriendlyName);
			OutputDebugString(L"\n");
		}
		CoTaskMemFree(szFriendlyName);
	}
	return hr;
}

HRESULT Media::CreateSinkWriter(DWORD *pVideoOutStreamIndex, DWORD *pAudioOutStreamIndex)
{	
	IMFMediaType* pVidMediaTypeIn = NULL;
	IMFMediaType* pVidMediaTypeOut = NULL;
	IMFMediaType* pAudMediaTypeIn = NULL;
	IMFMediaType* pAudMediaTypeOut = NULL;
	DWORD videoOutStreamIndex = NULL;
	DWORD audioOutStreamIndex = NULL;
	DWORD videoInStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
	DWORD audioInStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
	HRESULT hr = S_OK;

	//create sink writer
	CHECK_HR(hr = MFCreateSinkWriterFromURL(L"output.wmv", NULL, NULL, &m_pSinkWriter),"CreateSinkWriterFromURL");
	
	//create output video media type and add it to stream
	CHECK_HR(hr = CreateVideoMediaTypeOut(&pVidMediaTypeOut),"CreateVideoMediaTypeOut");
	CHECK_HR(hr = m_pSinkWriter->AddStream(pVidMediaTypeOut, &videoOutStreamIndex),"Add Video Stream");

	//get input video mediatype and set it to the stream
	CHECK_HR(hr = m_pReader->GetCurrentMediaType(videoInStreamIndex, &pVidMediaTypeIn),"GetCurrentMediaType Video");
	CHECK_HR(hr = m_pSinkWriter->SetInputMediaType(videoOutStreamIndex, pVidMediaTypeIn, NULL),"SetInputMediaType Video");

	//create output audio media type and add it to stream
	CHECK_HR(hr = CreateAudioMediaTypeOut(&pAudMediaTypeOut), "CreateAudioMediaTypeOut");
	CHECK_HR(hr = m_pSinkWriter->AddStream(pAudMediaTypeOut, &audioOutStreamIndex), "Add Audio Stream");

	//get input audio mediatype and set it to the stream
	CHECK_HR(hr = m_pReader->GetCurrentMediaType(audioInStreamIndex, &pAudMediaTypeIn), "GetCurrentMediaType");
	CHECK_HR(hr = m_pSinkWriter->SetInputMediaType(audioOutStreamIndex, pAudMediaTypeIn, NULL), "SetInputMediaType Audio");
	
	//begin writing
	CHECK_HR(hr = m_pSinkWriter->BeginWriting(), "BeginWriting");

	//return stream indexes
	if (SUCCEEDED(hr))
	{
		*pVideoOutStreamIndex = videoOutStreamIndex;
		*pAudioOutStreamIndex = audioOutStreamIndex;
	}

	return hr;
}

HRESULT Media::WriteToFile(DWORD vidStreamIndex, DWORD audStreamIndex)
{
	HRESULT hr = S_OK;
	LONGLONG baseTimeSamp = NULL;

	for (int i = 0; i < 100; i++)
	{
		ReadWriteSample(i, &baseTimeSamp,MF_SOURCE_READER_FIRST_AUDIO_STREAM, audStreamIndex);
		ReadWriteSample(i, &baseTimeSamp, MF_SOURCE_READER_FIRST_VIDEO_STREAM, vidStreamIndex);
	}

	CHECK_HR(hr = m_pSinkWriter->Finalize(), "Finalize");
	return hr;
}

HRESULT Media::CreateVideoMediaTypeOut(IMFMediaType ** pMediaTypeOut)
{
	//another option - taking one of the native media types
	/*HRESULT hr;
	CHECK_HR(hr = m_pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, pMediaTypeOut), "GetNativeMediaType Audio");
	return hr;*/

	// Format constants
	const UINT32 VIDEO_WIDTH = 640;
	const UINT32 VIDEO_HEIGHT = 480;
	const UINT32 VIDEO_FPS = 30;
	const UINT64 VIDEO_FRAME_DURATION = 10 * 1000 * 1000 / VIDEO_FPS;
	const UINT32 VIDEO_BIT_RATE = 800000;
	const GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_WMV3;
	const UINT32 VIDEO_PELS = VIDEO_WIDTH * VIDEO_HEIGHT;
	const UINT32 VIDEO_FRAME_COUNT = 20 * VIDEO_FPS;
	
	// Set the output media type.
	HRESULT hr = S_OK;
	CHECK_HR(hr = MFCreateMediaType(pMediaTypeOut),"CreateMediaType CreateVideoMediaTypeOut");
	CHECK_HR(hr = (*pMediaTypeOut)->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video),"set major type");
	CHECK_HR(hr = (*pMediaTypeOut)->SetGUID(MF_MT_SUBTYPE, VIDEO_ENCODING_FORMAT),"set subtype");
	CHECK_HR(hr = (*pMediaTypeOut)->SetUINT32(MF_MT_AVG_BITRATE, VIDEO_BIT_RATE),"set bitrate");
	CHECK_HR(hr = (*pMediaTypeOut)->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive),"set interlace mode");
	CHECK_HR(hr = MFSetAttributeSize(*pMediaTypeOut, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT),"set size");
	CHECK_HR(hr = MFSetAttributeRatio(*pMediaTypeOut, MF_MT_FRAME_RATE, VIDEO_FPS, 1),"set frame rate");
	CHECK_HR(hr = MFSetAttributeRatio(*pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1),"set pixel aspect ratio");

	return hr;
}

HRESULT Media::CreateAudioMediaTypeOut(IMFMediaType ** pAudMediaTypeOut)
{
	HRESULT hr;
	CHECK_HR(hr = m_pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, pAudMediaTypeOut),"GetNativeMediaType Audio");
	return hr;
}

HRESULT Media::ReadWriteSample(int i, LONGLONG* baseTimeSamp, DWORD readStreamIndex, DWORD writeStreamIndex)
{
	IMFSample* pSample = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	LONGLONG timeStamp = NULL;
	HRESULT hr = S_OK;

	CHECK_HR(hr = m_pReader->ReadSample(readStreamIndex, 0, &stIndex, &flags, &timeStamp, &pSample),"ReadSample");
	if (i == 0)
	{
		*baseTimeSamp = timeStamp;
	}
	if (SUCCEEDED(hr) && pSample)
	{
		CHECK_HR(hr = pSample->SetSampleTime(timeStamp - *baseTimeSamp),"SetSampleTime");
	}
	if (SUCCEEDED(hr) && pSample)
	{
		CHECK_HR(hr = m_pSinkWriter->WriteSample(writeStreamIndex, pSample),"WriteSample");
	}
	SafeRelease(&pSample);
	return hr;
}

HRESULT Media::CreateAggregatedSourceReader()
{
	HRESULT hr = S_OK;
	// Create a collection of audio & video sources.
	IMFCollection* pCollection = NULL;
	CHECK_HR(hr = MFCreateCollection(&pCollection), "Create Collection");
	CHECK_HR(hr = pCollection->AddElement(m_pAUDSource), "add audio element");
	CHECK_HR(hr = pCollection->AddElement(m_pVIDSource), "add video element");
	// Aggregate the audio & video sources to one source.
	CHECK_HR(hr = MFCreateAggregateSource(pCollection, &m_pAggSource), "MFCreateAggregateSource");
	// Create source reader.
	CHECK_HR(hr = MFCreateSourceReaderFromMediaSource(m_pAggSource, NULL, &m_pReader), "MFCreateSourceReaderFromMediaSource");
	return hr;
}

HRESULT Media::SetSourceReaderAudioMediaType()
{
	HRESULT hr = S_OK;
	IMFMediaType *pPartialMediaType = NULL;
	CHECK_HR(hr = MFCreateMediaType(&pPartialMediaType), "MFCreateMediaType source reader");
	CHECK_HR(hr = pPartialMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio), "set major type source reader");
	CHECK_HR(hr = pPartialMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float), "set subtype source reader");
	CHECK_HR(hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pPartialMediaType), "SetCurrentMediaType source reader");
	return hr;
}

Media::~Media() { }