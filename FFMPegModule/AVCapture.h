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

class CAVCapture
{
public:
	CAVCapture();
	~CAVCapture();
	void AIThread();
	void pushFrame(int rows, int cols, uint8_t* buffer, int64_t pts);
	void waitForFinish();
	int doReadWrite(const char* strInputFile, const char* strOutputFile, int64_t bitRates = 6000000, float fps = 30.0f, int64_t duration = 0);
	int writeFrame(Mat frame, int64_t pts);
	int openReading(const char* strInputFile);
	int openWriting(const char* strOutputFile);
	void closeReading();
	void closeWriting();
	bool flushPackets();
private:
	bool _bLoop;
	std::thread _thr_ai;
	CAIDnn* _pDnn;
	mutex _mtx;
	condition_variable _cond;

	queue<pair<Mat, int64_t>> _frames;

	int64_t _nFrames;

	int _ySize;
	AVPixelFormat _format;
	int _nBytes;	

	AVFormatContext* pRFormatCtx;
	AVCodecContext* pRCodecCtx;
	AVCodec* pRCodec;
	AVFrame* pRFrame;
	AVFrame* pRDst;
	AVPacket* pRPacket;
	uint8_t* pRBuffer;
	int _videoIndex;

	AVCodec* pCodec;
	AVFormatContext* pWFormatCtx;
	AVCodecContext* pWCodecCtx;
	AVStream* pStream;
	AVFrame* pWFrame;
	AVOutputFormat* pWOutputFmt;
	SwsContext* pWSwsContext;

	int64_t _bitRates;
	float _fps;
	int64_t _frameDuration;
	int64_t _videoDuration;

	string _strOutputName;
	string _strOutputExt;

	int _idx;
};

