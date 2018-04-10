#pragma once
#include <mfapi.h>
#include <mfidl.h>
#include <iostream>
class Media
{
public:
	Media();
	void createMediaFile();//TODO change name
	void EnumerateDevices(GUID deviceType);
	~Media();
};

