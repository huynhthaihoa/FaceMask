#pragma once
#define DLLEXPORT  extern "C" __declspec(dllexport)
#include <cstdint>
#include <iostream>

/**
* Check whether reading-writing process is running or not
* @return true if reading-writing process is running, false if otherwise
*/
DLLEXPORT bool IsRun();
/**
* Do reading-writing process (wrapper for CAvCapture::doReadWrite)
* @param strInputFile [const char*]: input video file path (reading)
* @param strOutputFile [const char*]: output video file path (writing)
* @param bitRates [int64_t]: writing video's bit rate (default is 6000000)
* @param fps [float]: writing video's frames per second (default is 30.0f)
* @param duration [int64_t]: split duration (default is 0 - writing video will not be split)
* @return zero on success, negative on reading error, positive on writing error
*/
DLLEXPORT int Process(const char* strInputFile, const char* strOutputFile, long bitrate, float fps, long duration);
/**
Release all using resources (use when finishing the program)
*/
DLLEXPORT void Release();
/**
Set up callback function for CAvCapture::updateStatus
*/
//DLLEXPORT void SetUpdateStatusCallback(void (__stdcall * callback)(int64_t hour, int64_t minute, int64_t second));