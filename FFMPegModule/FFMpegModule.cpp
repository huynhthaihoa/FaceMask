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

DLLEXPORT int Process(const char* strInputFile, const char* strOutputFile, long bitrate, float fps, long duration)
{
	if (_avCapture == nullptr)
	{
		CAVCapture* AvCapture = new CAVCapture();
		_avCapture = AvCapture;
	}

	int ret = _avCapture->doReadWrite(strInputFile, strOutputFile, bitrate, fps, duration);
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

DLLEXPORT void SetUpdateStatusCallback(void (__stdcall* callback)(int64_t hour, int minute, int second))
{
	if (_avCapture)
		_avCapture->_callback = callback;
}
