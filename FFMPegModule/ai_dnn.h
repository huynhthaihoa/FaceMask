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
	CAIDnn(std::string strClassFile, string strCfgFile, std::string strWeightsFile, float threshold);
	~CAIDnn();
	Mat analysis(Mat& frame);
	//void pushFrame(Mat& frame);
	//void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);
private:
	void postprocess(Mat& frame, const std::vector<Mat>& outs);
	//bool _bLoop = false;
	//queue<Mat> _Frames;
	vector<string> _classes;
	//	int _objCX;
	//	int _objCY;
	//	float _evtProb;
	//	mutex _mtxProb;
	mutex _mtx;
	condition_variable _cond;
	string appPath;
	//thread _thr_ai;

	//	int _img_w;
	//	int _img_h;

	float _threshold = 0.5f;

	dnn::Net _net;
	vector<string> _names;
	//vector<int> _outLayers;
	//vector<string> _layersNames;
	//vector<string> _classes;
};

