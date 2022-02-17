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
	/**
    * Class constructor
	* @param strClassFile [string]: class file path (.names file)
	* @param strCfgFile [string]: config file path (.cfg file)
	* @param strWeightsFile [string]: weight file path (.weights file)
	* @param confThres [float]: confident threshold (default is 0.5f)
	* @param nmsThres [float]: non-maximum suppression (NMS) threshold (default is 0.4f)
    */
	CAIDnn(std::string strClassFile, string strCfgFile, std::string strWeightsFile, float confThres = 0.5f, float nmsThres = 0.4f);
	/**
    Class destructor
    */
	~CAIDnn();
	/**
    * Analyse the current frame
    * @param frame [Mat&]: frame to be analyzed
	* @return analyzed frame
    */
	Mat analysis(Mat& frame);
private:
	/**
    * Postprocess the frame after running inference model on it
    * @param frame [Mat&]: frame to be analyzed
	* @param outs [vector<Mat>&]: output of the inference model
    */
	void postprocess(Mat& frame, const std::vector<Mat>& outs);
	//vector<string> _classes;
	mutex _mtx;
	condition_variable _cond;
	string _appPath;

	float _confThres;// = 0.5f;
	float _nmsThres;

	dnn::Net _net;
	vector<string> _names;
};

