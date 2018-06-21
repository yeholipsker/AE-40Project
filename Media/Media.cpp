#include "stdafx.h"
#include "Media.h"
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

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
	m_stopRecording = false;
	HRESULT hr = S_OK;
	DWORD vidStreamIndex = NULL;
	DWORD audStreamIndex = NULL;

	// Get the device lists and activate the source.
	CHECK_HR(hr = EnumerateDevicesAndActivateSource(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID),"EnumerateDevicesAndActivateSource audio");
	CHECK_HR(hr = EnumerateDevicesAndActivateSource(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID),"EnumerateDevicesAndActivateSource video");

	// Create aggregated audio & video sourceReader.
	CHECK_HR(hr = CreateAggregatedSourceReader(),"CreateAggregatedSource");

	// Set sourceReader compatible audio mediaType.
	CHECK_HR(hr = SetSourceReaderMediaTypes(),"SetSourceReaderMediaTypes");
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
	// Activate the first suitable device.
	int n = 0;
	if (deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
	{
		n = 0;
	}

	CHECK_HR(hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pMediaSource)), "ActivateObject");  // TODO - need to be compatabke with the 'no headphones' mode.

	// Save the media source.
	if (deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
	{
		m_pAUDSource = pMediaSource;
	}
	else if(deviceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
	{
		m_pVIDSource = pMediaSource;
	}
	SafeRelease(pAttributes);
	for (size_t i = 0; i < count; i++)
	{
		SafeRelease(ppDevices[i]);
	}
	CoTaskMemFree(ppDevices);
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
	SafeRelease(pCollection);
	return hr;
}

HRESULT Media::SetSourceReaderMediaTypes()
{
	HRESULT hr = S_OK;
	IMFMediaType *pTypeOutVid = NULL;
	hr = MFCreateMediaType(&pTypeOutVid);
	hr = pTypeOutVid->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = pTypeOutVid->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
	CHECK_HR(hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pTypeOutVid), "SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM");
	IMFMediaType *pTypeOutAud;
	hr = MFCreateMediaType(&pTypeOutAud);
	hr = pTypeOutAud->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	hr = pTypeOutAud->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	hr = pTypeOutAud->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 40000);
	hr = pTypeOutAud->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
	hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pTypeOutAud);
	Utilities * u = new Utilities();
	u->LogMediaType(pTypeOutAud);
	SafeRelease(pTypeOutVid);
	SafeRelease(pTypeOutAud);
	return hr;
}

IMFMediaType * Media::getOutputMediaTypeAudio() 
{
	IMFMediaType * outAud = NULL;
	m_pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outAud);
	return outAud;
}

IMFMediaType * Media::getOutputMediaTypeVideo()
{
	IMFMediaType * outVid = NULL;
	m_pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &outVid);
	return outVid;
}

void Media::StopRecording() { m_stopRecording = true; }

Media::~Media() 
{
	m_pAUDSource->Shutdown();
	SafeRelease(m_pAUDSource);
	m_pVIDSource->Shutdown();
	SafeRelease(m_pVIDSource);
	m_pAggSource->Shutdown();
	SafeRelease(m_pAggSource);
	SafeRelease(m_pReader);
}