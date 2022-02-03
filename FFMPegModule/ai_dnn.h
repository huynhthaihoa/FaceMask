#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <fcntl.h>
#include <errno.h>

#include <opencv2/opencv.hpp>

#pragma once

using namespace std;

using namespace cv;
using namespace dnn;

class CAIDnn
{
public:
	CAIDnn(std::string strClassFile, string strCfgFile, std::string strWeightsFile, float confThres = 0.5f, float nmsThres = 0.4f);
	~CAIDnn();
	pair<Mat, int64_t> analysis(Mat& frame, int64_t pts);
private:
	void postprocess(Mat& frame, const std::vector<Mat>& outs);
	vector<string> _classes;
	mutex _mtx;
	condition_variable _cond;
	string _appPath;

	float _confThres;// = 0.5f;
	float _nmsThres;

	dnn::Net _net;
	vector<string> _names;
};

