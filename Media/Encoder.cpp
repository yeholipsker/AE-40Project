#include "stdafx.h"
#include "Encoder.h"
#include <cstdlib>


Encoder::Encoder()
{
	m_pAudEncoderTransform = NULL;
	m_pVidEncoderTransform = NULL;
}

HRESULT Encoder::InitializeVideoEncoder(IMFMediaType * pType)
{
	IMFMediaType *pMFTInputMediaType = NULL, *pMFTOutputMediaType = NULL, *pTypeOut = NULL;
	IUnknown *spTransformUnk = NULL;
	DWORD mftStatus = 0;
	HRESULT hr = S_OK;
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	//MFStartup(MF_VERSION);

	// Create H.264 encoder.
	CHECK_HR(CoCreateInstance(CLSID_CMSH264EncoderMFT, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&spTransformUnk), "Failed to create H264 encoder MFT.\n");

	CHECK_HR(spTransformUnk->QueryInterface(IID_PPV_ARGS(&m_pVidEncoderTransform)), "Failed to get IMFTransform interface from H264 encoder MFT object.\n");

	// Transform output type
	MFCreateMediaType(&pMFTOutputMediaType);
	pMFTOutputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	pMFTOutputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
	pMFTOutputMediaType->SetUINT32(MF_MT_AVG_BITRATE, 240 * 1000);
	CHECK_HR(MFSetAttributeSize(pMFTOutputMediaType, MF_MT_FRAME_SIZE, 640, 480), "Failed to set frame size on H264 MFT out type.\n");
	CHECK_HR(MFSetAttributeRatio(pMFTOutputMediaType, MF_MT_FRAME_RATE, 30, 1), "Failed to set frame rate on H264 MFT out type.\n");
	CHECK_HR(MFSetAttributeRatio(pMFTOutputMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), "Failed to set aspect ratio on H264 MFT out type.\n");
	pMFTOutputMediaType->SetUINT32(MF_MT_INTERLACE_MODE, 2);
	pMFTOutputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
	CHECK_HR(m_pVidEncoderTransform->SetOutputType(0, pMFTOutputMediaType, 0), "Failed to set output media type on H.264 encoder MFT.\n");

	m_pVidEncoderTransform->GetOutputCurrentType(0, &pTypeOut);
	Utilities * util = new Utilities();
	util->LogMediaType(pTypeOut);
	// Transform input type
	MFCreateMediaType(&pMFTInputMediaType);
	pMFTInputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	pMFTInputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
	CHECK_HR(MFSetAttributeSize(pMFTInputMediaType, MF_MT_FRAME_SIZE, 640, 480), "Failed to set frame size on H264 MFT out type.\n");
	CHECK_HR(MFSetAttributeRatio(pMFTInputMediaType, MF_MT_FRAME_RATE, 30, 1), "Failed to set frame rate on H264 MFT out type.\n");
	CHECK_HR(MFSetAttributeRatio(pMFTInputMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), "Failed to set aspect ratio on H264 MFT out type.\n");

	CHECK_HR(hr = m_pVidEncoderTransform->SetInputType(0, pMFTInputMediaType, 0), "Failed to set input media type on H.264 encoder MFT.\n");


	CHECK_HR(m_pVidEncoderTransform->GetInputStatus(0, &mftStatus), "Failed to get input status from H.264 MFT.\n");
	if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus)
	{
		printf("E: pEncoderTransform->GetInputStatus() not accept data.\n");

		SafeRelease(&pMFTInputMediaType);
		SafeRelease(&pMFTOutputMediaType);

		return S_FALSE;
	}

	CHECK_HR(m_pVidEncoderTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL), "Failed to process FLUSH command on H.264 MFT.\n");
	CHECK_HR(m_pVidEncoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL), "Failed to process BEGIN_STREAMING command on H.264 MFT.\n");
	CHECK_HR(m_pVidEncoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL), "Failed to process START_OF_STREAM command on H.264 MFT.\n");

	return S_OK;
}

HRESULT Encoder::TransformVideoSample(IMFSample * pSample, BYTE ** ppRawBuffer, DWORD * pBuffLength)
{
	IMFSample *pOutSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;
	*ppRawBuffer = NULL;
	DWORD mftOutFlags;
	HRESULT hr = m_pVidEncoderTransform->ProcessInput(0, pSample, 0);
	CHECK_HR(hr = m_pVidEncoderTransform->GetOutputStatus(&mftOutFlags), "H264 MFT GetOutputStatus failed.\n");

	// Note: Decoder does not return MFT flag MFT_OUTPUT_STATUS_SAMPLE_READY, so we just need to rely on S_OK return
	if (mftOutFlags == S_OK)
	{
		return S_OK;
	}
	else if (mftOutFlags == MFT_OUTPUT_STATUS_SAMPLE_READY)
	{
		DWORD processOutputStatus = 0;
		MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
		MFT_OUTPUT_STREAM_INFO StreamInfo;
		m_pVidEncoderTransform->GetOutputStreamInfo(0, &StreamInfo);

		CHECK_HR(MFCreateSample(&pOutSample), "Failed to create MF sample.\n");
		CHECK_HR(MFCreateMemoryBuffer(StreamInfo.cbSize, &pBuffer), "Failed to create memory buffer.\n");
		//CHECK_HR(pBuffer->SetCurrentLength(StreamInfo.cbSize), "Failed SetCurrentLength.\n");
		CHECK_HR(pOutSample->AddBuffer(pBuffer), "Failed to add sample to buffer.\n");
		outputDataBuffer.dwStreamID = 0;
		outputDataBuffer.dwStatus = 0;
		outputDataBuffer.pEvents = NULL;
		outputDataBuffer.pSample = pOutSample;

		HRESULT hr = m_pVidEncoderTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);
		if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
		{
			SafeRelease(&pBuffer);
			SafeRelease(&pOutSample);
			return hr;
		}

		LONGLONG llVideoTimeStamp, llSampleDuration;
		pSample->GetSampleTime(&llVideoTimeStamp);
		pSample->GetSampleDuration(&llSampleDuration);
		CHECK_HR(outputDataBuffer.pSample->SetSampleTime(llVideoTimeStamp), "Error setting MFT sample time.\n");
		CHECK_HR(outputDataBuffer.pSample->SetSampleDuration(llSampleDuration), "Error setting MFT sample duration.\n");

		IMFMediaBuffer *pMediaBuffer = NULL;
		CHECK_HR(pOutSample->ConvertToContiguousBuffer(&pMediaBuffer), "ConvertToContiguousBuffer failed.\n");

		BYTE* rawBuffer = NULL;
		pMediaBuffer->Lock(&rawBuffer, NULL, pBuffLength);
		BYTE* rawBufferCopy = new BYTE[*pBuffLength];
		memcpy(rawBufferCopy, rawBuffer, (*pBuffLength));
		//rawBufferCopy[*pBuffLength] = '\0';
		pMediaBuffer->Unlock();
		SafeRelease(&pMediaBuffer);

		// Decoded sample out
		//*ppSampleOut = outputDataBuffer.pSample;
		*ppRawBuffer = rawBufferCopy;

		//SafeRelease(&pMediaBuffer);
		SafeRelease(&pBuffer);

		return S_OK;
	}
}

HRESULT Encoder::InitializeAudioEncoder(IMFMediaType ** pType)
{
	IMFMediaType *pMFTInputMediaType = NULL, *pMFTOutputMediaType = NULL;
	IUnknown *spTransformUnk = NULL;
	DWORD mftStatus = 0;
	HRESULT hr = S_OK;
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	//MFStartup(MF_VERSION);

	// Create MPEG-2 encoder.
	CHECK_HR(hr = CoCreateInstance(CLSID_MP3ACMCodecWrapper, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&spTransformUnk), "Failed to create H264 encoder MFT.\n");

	CHECK_HR(hr = spTransformUnk->QueryInterface(IID_PPV_ARGS(&m_pAudEncoderTransform)), "Failed to get IMFTransform interface from mp3 encoder MFT object.\n");

	CHECK_HR(hr = FindOutputMediaType(),"FindOutputMediaType()");
	CHECK_HR(hr = FindInputMediaType(), "FindInputMediaType()");

	CHECK_HR(hr = m_pAudEncoderTransform->GetInputStatus(0, &mftStatus), "Failed to get input status from H.264 MFT.\n");
	if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus)
	{
		printf("E: pEncoderTransform->GetInputStatus() not accept data.\n");

		SafeRelease(&pMFTInputMediaType);
		SafeRelease(&pMFTOutputMediaType);

		return S_FALSE;
	}

	CHECK_HR(hr = m_pAudEncoderTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL), "Failed to process FLUSH command on H.264 MFT.\n");
	CHECK_HR(hr = m_pAudEncoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL), "Failed to process BEGIN_STREAMING command on H.264 MFT.\n");
	CHECK_HR(hr = m_pAudEncoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL), "Failed to process START_OF_STREAM command on H.264 MFT.\n");

	return S_OK;
}

HRESULT Encoder::TransformAudioSample(IMFSample * pSample, IMFSample ** ppSampleOut, BYTE ** ppRawBuffer, DWORD * pBuffLength)
{
	IMFSample *pOutSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;
	DWORD mftOutFlags;
	HRESULT hr = m_pAudEncoderTransform->ProcessInput(0, pSample, 0);
	CHECK_HR(hr = m_pAudEncoderTransform->GetOutputStatus(&mftOutFlags), "H264 MFT GetOutputStatus failed.\n");

	// Note: Decoder does not return MFT flag MFT_OUTPUT_STATUS_SAMPLE_READY, so we just need to rely on S_OK return
	if (mftOutFlags == S_OK)
	{
		return S_OK;
	}
	else if (mftOutFlags == MFT_OUTPUT_STATUS_SAMPLE_READY)
	{
		DWORD processOutputStatus = 0;
		MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
		MFT_OUTPUT_STREAM_INFO StreamInfo;
		m_pAudEncoderTransform->GetOutputStreamInfo(0, &StreamInfo);

		CHECK_HR(MFCreateSample(&pOutSample), "Failed to create MF sample.\n");
		CHECK_HR(MFCreateMemoryBuffer(StreamInfo.cbSize, &pBuffer), "Failed to create memory buffer.\n");
		//CHECK_HR(pBuffer->SetCurrentLength(StreamInfo.cbSize), "Failed SetCurrentLength.\n");
		CHECK_HR(pOutSample->AddBuffer(pBuffer), "Failed to add sample to buffer.\n");
		outputDataBuffer.dwStreamID = 0;
		outputDataBuffer.dwStatus = 0;
		outputDataBuffer.pEvents = NULL;
		outputDataBuffer.pSample = pOutSample;

		HRESULT hr = m_pAudEncoderTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);
		if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
		{
			SafeRelease(&pBuffer);
			SafeRelease(&pOutSample);
			return hr;
		}

		LONGLONG llVideoTimeStamp, llSampleDuration;
		pSample->GetSampleTime(&llVideoTimeStamp);
		pSample->GetSampleDuration(&llSampleDuration);
		CHECK_HR(outputDataBuffer.pSample->SetSampleTime(llVideoTimeStamp), "Error setting MFT sample time.\n");
		CHECK_HR(outputDataBuffer.pSample->SetSampleDuration(llSampleDuration), "Error setting MFT sample duration.\n");

		IMFMediaBuffer *pMediaBuffer = NULL;
		CHECK_HR(pOutSample->ConvertToContiguousBuffer(&pMediaBuffer), "ConvertToContiguousBuffer failed.\n");

		BYTE* rawBuffer = NULL;
		pMediaBuffer->Lock(&rawBuffer, NULL, pBuffLength);
		BYTE* rawBufferCopy = new BYTE[*pBuffLength];
		memcpy(rawBufferCopy, rawBuffer, (*pBuffLength));
		//rawBufferCopy[*pBuffLength] = '\0';
		pMediaBuffer->Unlock();
		SafeRelease(&pMediaBuffer);

		// Decoded sample out
		*ppSampleOut = outputDataBuffer.pSample;
		*ppRawBuffer = rawBufferCopy;

		//SafeRelease(&pMediaBuffer);
		SafeRelease(&pBuffer);
		return S_OK;
	}
}


Encoder::~Encoder()
{
}

HRESULT Encoder::FindOutputMediaType()
{
	DWORD m_dwInputID = 0, m_dwOutputID = 0;
	HRESULT hr = S_OK;
	IMFMediaType * pOutputType = NULL;
	Utilities * u = new Utilities();

	for (DWORD iType = 0; hr == S_OK ; iType++)
	{
		UINT32 avgBytesPerSecond = NULL, samplesPerSecond = NULL;
		CHECK_HR(hr = m_pAudEncoderTransform->GetOutputAvailableType(m_dwOutputID, iType, &pOutputType),"m_pAudEncoderTransform->GetOutputAvailableType");
		pOutputType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &avgBytesPerSecond);
		pOutputType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
		//u->LogMediaType(pOutputType);
		//OutputDebugString(L"\n");
		if (avgBytesPerSecond == 40000 && samplesPerSecond == 44100)
		{
			CHECK_HR(hr = m_pAudEncoderTransform->SetOutputType(m_dwOutputID, pOutputType, 0), "m_pAudEncoderTransform->SetOutputType");
			SafeRelease(&pOutputType);
			break;
		}
		SafeRelease(&pOutputType);
	}
	delete u;
	return hr;
}

HRESULT Encoder::FindInputMediaType()
{
	DWORD m_dwInputID = 0, m_dwOutputID = 0;
	HRESULT hr = S_OK;
	IMFMediaType * pInputType = NULL;
	Utilities * u = new Utilities();

	for (DWORD iType = 0; hr == S_OK; iType++)
	{
		UINT32 avgBytesPerSecond = NULL, samplesPerSecond = NULL;
		CHECK_HR(hr = m_pAudEncoderTransform->GetInputAvailableType(m_dwInputID, iType, &pInputType), "m_pAudEncoderTransform->GetOutputAvailableType");
		pInputType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &avgBytesPerSecond);
		pInputType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
		//u->LogMediaType(pInputType);
		//OutputDebugString(L"\n");
		if (avgBytesPerSecond == 176400 && samplesPerSecond == 44100)
		{
			CHECK_HR(hr = m_pAudEncoderTransform->SetInputType(m_dwInputID, pInputType, 0), "m_pAudEncoderTransform->SetOutputType");
			SafeRelease(&pInputType);
			break;
		}
		SafeRelease(&pInputType);
	}
	delete u;
	return hr;
}
