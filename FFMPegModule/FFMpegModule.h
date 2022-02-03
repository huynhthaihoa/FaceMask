#pragma once
#define DLLEXPORT  extern "C" __declspec(dllexport)
#include <cstdint>
#include <iostream>
//#include "pch.h"

DLLEXPORT bool IsRun();
DLLEXPORT int ReadVideo(const char* strInputFile);
DLLEXPORT int WriteVideo(const char* strOutputFile);
//DLLEXPORT int Test();
DLLEXPORT int Process(const char* strInputFile, const char* strOutputFile);
DLLEXPORT void Release();