#include "stdafx.h"
#include "FFMpegModule.h"
#include "AVCapture.h"


CAVCapture* _avCapture;
std::atomic<bool> _isRun;
std::atomic<int> dataReady(0);

DLLEXPORT bool IsRun()
{
	if (_avCapture == nullptr)
		return false;
	return _isRun;
}

DLLEXPORT int ReadVideo(const char* strInputFile)
{
#if 0
	_isRun = 0;
	return 1;
#else
	if (_isRun == true)
		return -300;

	try
	{
		if (_avCapture != nullptr)
		{
			delete _avCapture;
			_avCapture = nullptr;
		}
	}
	catch (...)
	{
		int a = 1;
	}

	CAVCapture* AvCapture = new CAVCapture();
	_avCapture = AvCapture;

	int ret = _avCapture->readVideo(strInputFile);
	//if (ret != 0)
	//	_isRun = 0;

	_isRun = (ret == 0);

	return ret;
#endif
}

DLLEXPORT int WriteVideo(const char* strOutputFile)
{
	if (_isRun == true)
		return 300;
	if (_avCapture == nullptr)
		return 400;

	int ret = _avCapture->writeVideo(strOutputFile);

	_isRun = (ret == 0);

	return ret;
}

//DLLEXPORT int Test()
//{
//	return 1;
//}

DLLEXPORT int Process(const char* strInputFile, const char* strOutputFile)
{
	if (_avCapture == nullptr)
	{
		CAVCapture* AvCapture = new CAVCapture();
		_avCapture = AvCapture;
	}
#if 1
	int ret = _avCapture->doReadWrite(strInputFile, strOutputFile);
	_isRun = (ret == 0);
#else
	int ret = _avCapture->readVideo(strInputFile);
	if (ret == 0)
		ret = _avCapture->writeVideo(strOutputFile);
#endif
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
