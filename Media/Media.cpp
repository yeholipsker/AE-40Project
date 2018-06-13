#include "stdafx.h"
#include "Media.h"

// Struct contains data for the threads.
typedef struct MyData
{
	DWORD vidStreamIndex;
	DWORD audStreamIndex;
} MyData;

Media::Media()
{
	m_pVIDSource = NULL;
	m_pAUDSource = NULL;
	m_pAggSource = NULL;
	m_pReader = NULL;
	m_pSinkWriter = NULL;
	m_stopRecording = false;
}

void Media::InitializeSource()
{
	Utilities* u = new Utilities();
	m_stopRecording = false;
	HRESULT hr = S_OK;
	DWORD vidStreamIndex = NULL;
	DWORD audStreamIndex = NULL;

	//initialize COM & MF
	//CHECK_HR(hr = CoInitialize(NULL),"CoInitialize");
	//CHECK_HR(hr = MFStartup(MF_VERSION),"MFStartup");

	// Get the device lists and activate the source.
	CHECK_HR(hr = EnumerateDevicesAndActivateSource(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID),"EnumerateDevicesAndActivateSource audio");
	CHECK_HR(hr = EnumerateDevicesAndActivateSource(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID),"EnumerateDevicesAndActivateSource video");

	// Create aggregated audio & video sourceReader.
	CHECK_HR(hr = CreateAggregatedSourceReader(),"CreateAggregatedSource");

	// Set sourceReader compatible audio mediaType.
	CHECK_HR(hr = SetSourceReaderAudioMediaType(),"SetSourceReaderAudioMediaType");

	IMFMediaType *pTypeOutVid = NULL;
	MFCreateMediaType(&pTypeOutVid);
	hr = pTypeOutVid->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = pTypeOutVid->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
	CHECK_HR(hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pTypeOutVid), "SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM");
	IMFMediaType *pType;
	MFCreateMediaType(&pType);
	hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,NULL,pType);
	IMFMediaType *pTypeOutAud = NULL;
	m_pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pTypeOutAud);
	u->LogMediaType(pTypeOutAud);
	// For creating local file. WE DONT NEED IT ANYMORE!!! TODO - REMOVE!
	/*
	// Create sink writer.
	CHECK_HR(hr = CreateSinkWriter(&vidStreamIndex,&audStreamIndex),"CreateSinkWriter");

	MyData* dataForThreads = new MyData();
	dataForThreads->vidStreamIndex = vidStreamIndex;
	dataForThreads->audStreamIndex = audStreamIndex;

	// Creating thread that write to the file.
	CreateThread(0, 0, WriteToFile, dataForThreads, 0, 0);
	*/
}

HRESULT Media::EnumerateDevicesAndActivateSource(GUID deviceType)
{
	IMFAttributes* pAttributes = NULL;
	IMFActivate** ppDevices = NULL;
	IMFMediaSource* pMediaSource = NULL;
	UINT32 count = 0;
	HRESULT hr = S_OK;
	
	// Set the device type.
	CHECK_HR(hr = MFCreateAttributes(&pAttributes, 1),"CreateAttributes");
	CHECK_HR(hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, deviceType),"set device type");
	// Get the appropriate devices for this type.
	CHECK_HR(hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count),"EnumDeviceSources");
	for (DWORD i = 0; i < count; i++)
	{
		hr = S_OK;
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
	// Activate the first suitable device.
	int n = 0;
	if (deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
	{
		n = 1;
	}
	CHECK_HR(hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pMediaSource)), "ActivateObject");

	// Save the media source.
	if (deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
	{
		m_pAUDSource = pMediaSource;
	}
	else if(deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
	{
		m_pVIDSource = pMediaSource;
	}
	
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
	// Create source reader for the aggregated source.
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

	// Create sink writer.
	CHECK_HR(hr = MFCreateSinkWriterFromURL(L"output.wmv", NULL, NULL, &m_pSinkWriter),"CreateSinkWriterFromURL");
	
	// Create output video media type and add it to the stream.
	CHECK_HR(hr = CreateVideoMediaTypeOut(&pVidMediaTypeOut),"CreateVideoMediaTypeOut");
	CHECK_HR(hr = m_pSinkWriter->AddStream(pVidMediaTypeOut, &videoOutStreamIndex),"Add Video Stream");

	// Get input video mediatype and set it to the stream.
	CHECK_HR(hr = m_pReader->GetCurrentMediaType(videoInStreamIndex, &pVidMediaTypeIn),"GetCurrentMediaType Video");
	CHECK_HR(hr = m_pSinkWriter->SetInputMediaType(videoOutStreamIndex, pVidMediaTypeIn, NULL),"SetInputMediaType Video");

	// Create output audio media type and add it to stream.
	CHECK_HR(hr = m_pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &pAudMediaTypeOut), "GetNativeMediaType Audio");
	CHECK_HR(hr = m_pSinkWriter->AddStream(pAudMediaTypeOut, &audioOutStreamIndex), "Add Audio Stream");

	// Get input audio mediatype and set it to the stream.
	CHECK_HR(hr = m_pReader->GetCurrentMediaType(audioInStreamIndex, &pAudMediaTypeIn), "GetCurrentMediaType");
	CHECK_HR(hr = m_pSinkWriter->SetInputMediaType(audioOutStreamIndex, pAudMediaTypeIn, NULL), "SetInputMediaType Audio");
	
	// Begin writing
	CHECK_HR(hr = m_pSinkWriter->BeginWriting(), "BeginWriting");

	// Return stream indices.
	if (SUCCEEDED(hr))
	{
		*pVideoOutStreamIndex = videoOutStreamIndex;
		*pAudioOutStreamIndex = audioOutStreamIndex;
	}

	return hr;
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
/*
DWORD WINAPI Media::WriteToFile(LPVOID lpParameter)
{
	HRESULT hr = S_OK;
	LONGLONG baseTimeSamp = NULL;
	MyData* streamIndices = (MyData*)lpParameter;

	for (int i = 0; !m_stopRecording; i++)
	{
		// Write audio & video sample to the file.
		CHECK_HR(hr = ReadWriteSample(i, &baseTimeSamp, MF_SOURCE_READER_FIRST_AUDIO_STREAM, streamIndices->audStreamIndex), "ReadWriteSample audio");
		CHECK_HR(hr = ReadWriteSample(i, &baseTimeSamp, MF_SOURCE_READER_FIRST_VIDEO_STREAM, streamIndices->vidStreamIndex), "ReadWriteSample video");
	}

	// Completes writing operation on the sink writer.
	CHECK_HR(hr = m_pSinkWriter->Finalize(), "Finalize");

	// Release the reader & writer.
	m_pReader->Release();
	m_pSinkWriter->Release();

	// Shutdown MF & COM
	CHECK_HR(hr = MFShutdown(), "MFShutdown");
	CoUninitialize();

	return hr;
}

HRESULT Media::ReadWriteSample(int i, LONGLONG* baseTimeSamp, DWORD readStreamIndex, DWORD writeStreamIndex)
{
	IMFSample* pSample = NULL;
	DWORD stIndex = NULL;
	DWORD flags = NULL;
	LONGLONG timeStamp = NULL;
	HRESULT hr = S_OK;

	// Read one sample.
	CHECK_HR(hr = m_pReader->ReadSample(readStreamIndex, 0, &stIndex, &flags, &timeStamp, &pSample), "ReadSample");
	// Set the base time stamp.
	if (i == 0)
	{
		*baseTimeSamp = timeStamp;
	}
	// Set the sample time stamp.
	if (SUCCEEDED(hr) && pSample)
	{
		CHECK_HR(hr = pSample->SetSampleTime(timeStamp - *baseTimeSamp), "SetSampleTime");
	}
	// erite the sample.
	if (SUCCEEDED(hr) && pSample)
	{
		CHECK_HR(hr = m_pSinkWriter->WriteSample(writeStreamIndex, pSample), "WriteSample");
	}
	
	SafeRelease(&pSample);

	return hr;
}
*/
void Media::StopRecording() { m_stopRecording = true; }

Media::~Media() { }