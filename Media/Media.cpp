#include "stdafx.h"
#include "Media.h"

//constructor
Media::Media()
{
	m_pVIDSource = NULL;
	m_pAUDSource = NULL;
	m_pAggSource = NULL;
	m_pReader = NULL;
	m_pSinkWriter = NULL;
	m_stopRecording = false;
}

//InitializeSource function - initializng aggregated source reader
void Media::InitializeSource()
{
	//variables declaration
	m_stopRecording = false;
	HRESULT hr = S_OK;
	DWORD vidStreamIndex = NULL;
	DWORD audStreamIndex = NULL;

	// Get the device lists and activate the source.
	CHECK_HR(hr = EnumerateDevicesAndActivateSource(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID),"EnumerateDevicesAndActivateSource audio");
	CHECK_HR(hr = EnumerateDevicesAndActivateSource(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID),"EnumerateDevicesAndActivateSource video");

	// Create aggregated audio & video sourceReader.
	CHECK_HR(hr = CreateAggregatedSourceReader(),"CreateAggregatedSource");

	// Set sourceReader compatible audio and video mediaType.
	CHECK_HR(hr = SetSourceReaderMediaTypes(),"SetSourceReaderMediaTypes");
}

//EnumerateDevicesAndActivateSource function - finding connected devices and use them as source
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

	//free allocated memory
	SafeRelease(pAttributes);
	for (size_t i = 0; i < count; i++)
	{
		SafeRelease(ppDevices[i]);
	}
	CoTaskMemFree(ppDevices);
	return hr;
}

//CreateAggregatedSourceReader function - create one source reader for audio and video
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

//SetSourceReaderMediaTypes function - setting desired media types on source reader
HRESULT Media::SetSourceReaderMediaTypes()
{
	//variable declarations
	HRESULT hr = S_OK;
	IMFMediaType *pTypeOutVid = NULL;
	IMFMediaType *pTypeOutAud;

	//set video media type parameters
	hr = MFCreateMediaType(&pTypeOutVid);
	hr = pTypeOutVid->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = pTypeOutVid->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
	CHECK_HR(MFSetAttributeSize(pTypeOutVid, MF_MT_FRAME_SIZE, 640, 480), "Failed to set frame size on H264 MFT out type.\n");
	CHECK_HR(MFSetAttributeRatio(pTypeOutVid, MF_MT_FRAME_RATE, 30, 1), "Failed to set frame rate on H264 MFT out type.\n");
	CHECK_HR(hr = m_pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pTypeOutVid), "SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM");
	
	//set audio media type parameters
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

//destructor
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