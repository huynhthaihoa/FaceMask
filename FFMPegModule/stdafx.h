// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"







// Windows ��� ����:
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
// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#pragma warning(disable:4819)