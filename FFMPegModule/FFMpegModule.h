#pragma once
#define DLLEXPORT  extern "C" __declspec(dllexport)
#include <cstdint>
#include <iostream>

DLLEXPORT bool IsRun();
DLLEXPORT int Process(const char* strInputFile, const char* strOutputFile, long bitrate, float fps, long duration);
DLLEXPORT void Release();
DLLEXPORT void UpdateStatus(void (__stdcall * callback)(int64_t hour, int64_t minute, int64_t second));