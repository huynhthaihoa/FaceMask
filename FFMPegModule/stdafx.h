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


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#pragma warning(disable:4819)