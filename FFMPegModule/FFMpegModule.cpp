#include "stdafx.h"
#include "FFMpegModule.h"
#include "AVCapture.h"

CAVCapture* _avCapture;
std::atomic<bool> _isRun;

DLLEXPORT bool IsRun()
{
	if (_avCapture == nullptr)
		return false;
	return _isRun;
}

DLLEXPORT int Process(const char* strInputFile, const char* strOutputFile)
{
	if (_avCapture == nullptr)
	{
		CAVCapture* AvCapture = new CAVCapture();
		_avCapture = AvCapture;
	}

	int ret = _avCapture->doReadWrite(strInputFile, strOutputFile);
	_isRun = (ret == 0);

	return ret;
}

DLLEXPORT void Release()
{
	if (_avCapture)
	{
		delete _avCapture;
		_avCapture = nullptr;
	}
}
