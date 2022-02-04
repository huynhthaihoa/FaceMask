// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"







// Windows 헤더 파일:
#include <windows.h>
//#include <afx.h>
#include <atlstr.h>
//#undef _WINDOWS_
//#include <afxwin.h>



#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include <fstream>

#include <objbase.h>
#include <iostream>

#include <format>

#ifndef _NDEBUG
#define ENABLE_ASSERTS 1
#endif



using namespace cv;
using namespace cv::dnn;

using namespace std;
using namespace dnn;

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0' 
    if (size <= 0) {
        throw std::runtime_error("Error during formatting.");
    }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside }
}
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#pragma warning(disable:4819)