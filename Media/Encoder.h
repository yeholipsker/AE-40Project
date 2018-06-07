#pragma once

#include "Utilities.h"

class Encoder
{
public:
	Encoder();
	HRESULT InitializeVideoEncoder(IMFMediaType *pType);
	HRESULT TransformVideoSample(IMFSample *pSample, IMFSample **ppSampleOut);
	HRESULT InitializeAudioEncoder(IMFMediaType **pType);
	HRESULT TransformAudioSample(IMFSample *pSample, IMFSample **ppSampleOut);
	~Encoder();
private:
	IMFTransform * m_pVidEncoderTransform;
	IMFTransform * m_pAudEncoderTransform;
};