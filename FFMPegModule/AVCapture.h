#pragma once
#include <iostream>
#include <condition_variable>
//#include <pplcancellation_token.h>
#include <ppltasks.h>
#include <ppl.h>
#include <string>
#include "ai_dnn.h"

extern "C" {
#include <libavutil/opt.h>        
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

//struct sData {
//	void* data;
//	int	size;
//	//int type;
//	//std::chrono::time_point<std::chrono::steady_clock> rec_time;
//
//	sData(void* p, int s, int t, std::chrono::time_point<std::chrono::steady_clock> tp) : data(p), size(s) {}
//};

//class AutoCS
//{
//public:
//	AutoCS(CRITICAL_SECTION* p) :m_p(p)
//	{
//		EnterCriticalSection(p);
//	}
//	~AutoCS() { LeaveCriticalSection(m_p); }
//	CRITICAL_SECTION* m_p;
//};

class CAVCapture
{
public:
	int readVideo(const char* strInputFile);
	int writeVideo(const char* strOutputFile);
	CAVCapture();
	~CAVCapture();
	void AIThread();
	void pushFrame(int rows, int cols, uint8_t* buffer);
	void waitForFinish();
	int doReadWrite(const char* strInputFile, const char* strOutputFile);
	int openReading(const char* strInputFile, int& videoindex);
	int openWriting(const char* strOutputFile);
	void closeReading();
	void closeWriting();
private:
	bool _bLoop;
	std::thread _thr_ai;
	CAIDnn* _pDnn;
	//std::deque<sData> _recQue;
	//unsigned int _volSize;
	//uint8_t* _pVolData;
	mutex _mtx;
	condition_variable _cond;
	//CRITICAL_SECTION _criticalSection;
	queue<Mat> _iFrames; //input frames
	queue<Mat> _oFrames; //output frames

	int _frameWidth;
	int _frameHeight;
	int _nFrames;
	AVRational _fps;
	//Size _frameSize;
	//AVPacket* packet;

	//AVCodecContext* pCodecCtx;
	//AVFormatContext* pFormatCtx;
	
	//AVFrame* pFrame;
	//AVFrame* dst;
	//AVStream* pStream;
	////SwsContext* pSwsCtx;
	//uint8_t* pBuffer;
	int _ySize;
	AVPixelFormat _format;
	int _nBytes;
	AVCodec* pCodec;

	AVFormatContext* pRFormatCtx;
	AVCodecContext* pRCodecCtx;
	AVCodec* pRCodec;
	AVFrame* pRFrame;
	AVFrame* pRDst;
	AVPacket* pRPacket;
	uint8_t* pRBuffer;

	AVFormatContext* pWFormatCtx;
	AVCodecContext* pWCodecCtx;
	AVStream* pStream;
	AVFrame* pWFrame;
	uint8_t* pWBuffer;
	AVPacket* pWPacket;

	VideoWriter* _pWriter;
};

