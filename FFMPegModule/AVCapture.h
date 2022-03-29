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

#ifndef USE_THREAD
#define USE_THREAD
#endif

#ifndef USE_OPENCV_CAPTURE
#define USE_OPENCV_CAPTURE
#endif

class CAVCapture
{
public:
	/**
	Class constructor
    */
	CAVCapture();
	/**
    Class destructor
    */
	~CAVCapture();
	/**
    * Main function to do reading - writing process
	* @param strInputFile [const char*]: input video file path (reading)
	* @param strOutputFile [const char*]: output video file path (writing)
	* @param bitRates [int64_t]: writing video's bit rate (default is 6000000)
	* @param fps [float]: writing video's frames per second (default is 30.0f)
	* @param duration [int64_t]: split duration (default is 0 - writing video will not be split)
	* @return zero on success, negative on reading error, positive on writing error
    */
	int doReadWrite(const char* strInputFile, const char* strOutputFile, int64_t bitRates = 6000000, float fps = 30.0f, int64_t duration = 0);
#ifdef USE_THREAD
	/**
    AI thread to process each frame in the queue
    */
	void AIThread();

#ifndef USE_OPENCV_CAPTURE
	//*@param rows[int]: number of rows of the frame
	//* @param cols[int]: number of columns of the frame
	/**
	* Convert current buffer to frame & push it into queue for further processing
	* @param buffer [uint8_t*]: current buffer 
	*/
	void pushFrame(uint8_t* buffer); //int rows, int cols, 
#else
    /**
	* Push frame into queue for further processing
	* @param frame [cv::Mat]: current frame
	*/
	void pushFrame(const cv::Mat &frame);
#endif
	/**
	To keep the reading - writing process waiting until AI thread is finished
    */
	void waitForFinish();
	/**
    * Write current frame to output video
    * @param frame [Mat]: frame to be written
	* @return zero on success, positive value on error
    */
	int writeFrame(const Mat& frame);
#else
	//* @param rows[int]: number of rows of the frame
	//* @param cols[int]: number of columns of the frame
	/**
    * Process and write current buffer to the output video
	* @param buffer [uint8_t*]: current buffer
	* @return zero on success, positive value on error
    */
	int writeFrame(uint8_t* buffer); //int rows, int cols, 
#endif
	/**
    * Open input video file for reading
    * @param strInputFile [const char*]: input video file path
	* @return zero on success, negative value on error 
    */
	int openReading(const char* strInputFile);
	/**
    * Create & open output video file for writing
    * @param strInputFile [const char*]: output video file path
    * @return zero on success, positive value on error
    */
	int openWriting(const char* strOutputFile);
	/**
    Finish reading video file by releasing all related resources
    */
	void closeReading();
	/**
    Finish writing video file by releasing all related resources
    */
	void closeWriting();
	/**
	* Flush current packets into the encoder
	* @return true if success, false if otherwise
	*/
	bool flushPackets();
	//*@param  hour[int64_t]: amount of hours that has been written
	//* @param  minute[int]: amount of minutes that has been written
	//* @param  second[int]: amount of seconds that has been written
	/**
    * Call callback function when finished writing one video (for duration-split mode) 
    */
	void updateStatus();// int64_t hour, int minute, int second);
	/**
	Callback function
	*/
	function<void(int64_t, int, int)> _callback;

private:
	CAIDnn* _pDnn;

#ifdef USE_THREAD
	bool _bLoop;
	std::thread _thr_ai;
	mutex _mtx;
	condition_variable _cond;
	queue<Mat> _frames;
#endif

	//for reading
#ifndef USE_OPENCV_CAPTURE
	AVFormatContext* pRFormatCtx;
	AVCodecContext* pRCodecCtx;
	AVCodec* pRCodec;
	AVFrame* pRFrame;
	AVFrame* pRDst;
	AVPacket* pRPacket;
	uint8_t* pRBuffer;
	int _videoIndex;
	SwsContext* pRSwsCtx;
#else
	cv::VideoCapture _videoCapture;
#endif

	//for writing
	AVCodec* pWCodec;
	AVFormatContext* pWFormatCtx;
	AVCodecContext* pWCodecCtx;
	AVStream* pWStream;
	AVFrame* pWFrame;
	AVOutputFormat* pWOutputFmt;
	SwsContext* pWSwsCtx;
	int64_t _nFrames;
	int64_t _bitRates;
	int64_t _frameDuration;
	int64_t _videoDuration;
	float _fps;
	//int _idx;
	string _strOutputName;
	string _strOutputExt;

	int _sec;
	int _min;
	int64_t _hr;

};

