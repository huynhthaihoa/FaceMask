#pragma once
#include <iostream>
#include <condition_variable>
#include <ppltasks.h>
#include <ppl.h>
#include <string>
#include "ai_dnn.h"

extern "C" {
#include <libavutil/opt.h>   
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
}

#include <thread>

#pragma warning(disable: 4819)
#pragma warning(disable:4996)

#ifndef _THREAD
#define _THREAD
#endif

class CAVCapture
{
public:
	CAVCapture();
	~CAVCapture();
	int doReadWrite(const char* strInputFile, const char* strOutputFile, int64_t bitRates = 6000000, float fps = 30.0f, int64_t duration = 0);
#ifdef _THREAD
	void AIThread();
	void pushFrame(int rows, int cols, uint8_t* buffer);
	void waitForFinish();
	int writeFrame(Mat frame);
#else
	int writeFrame(int rows, int cols, uint8_t* buffer);
#endif
	int openReading(const char* strInputFile);
	int openWriting(const char* strOutputFile);
	void closeReading();
	void closeWriting();
	bool flushPackets();
private:
	CAIDnn* _pDnn;

#ifdef _THREAD
	bool _bLoop;
	std::thread _thr_ai;
	mutex _mtx;
	condition_variable _cond;
	queue<Mat> _frames;
#endif

	int64_t _nFrames;

	//int _ySize;
	//AVPixelFormat _format;
	//int _nBytes;	

	AVFormatContext* pRFormatCtx;
	AVCodecContext* pRCodecCtx;
	AVCodec* pRCodec;
	AVFrame* pRFrame;
	AVFrame* pRDst;
	AVPacket* pRPacket;
	uint8_t* pRBuffer;
	int _videoIndex;
	SwsContext* pRSwsCtx;

	AVCodec* pWCodec;
	AVFormatContext* pWFormatCtx;
	AVCodecContext* pWCodecCtx;
	AVStream* pWStream;
	AVFrame* pWFrame;
	AVOutputFormat* pWOutputFmt;
	SwsContext* pWSwsCtx;

	int64_t _bitRates;
	float _fps;
	int64_t _frameDuration;
	int64_t _videoDuration;

	string _strOutputName;
	string _strOutputExt;

	int _idx;
};

