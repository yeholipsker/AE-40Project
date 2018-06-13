#pragma once

#include "Utilities.h"

class Encoder
{
public:
	Encoder();
	HRESULT InitializeVideoEncoder(IMFMediaType *pType);
	HRESULT TransformVideoSample(IMFSample * pSample, IMFSample ** ppSampleOut, BYTE ** ppRawBuffer, DWORD * pBuffLength);
	HRESULT InitializeAudioEncoder(IMFMediaType **pType);
	HRESULT TransformAudioSample(IMFSample * pSample, IMFSample ** ppSampleOut, BYTE ** ppRawBuffer, DWORD * pBuffLength);
	~Encoder();
private:
	IMFTransform * m_pVidEncoderTransform;
	IMFTransform * m_pAudEncoderTransform;
	HRESULT FindOutputMediaType();
	HRESULT FindInputMediaType();
};