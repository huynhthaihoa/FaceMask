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
	//* @param strClassFile [string]: class file path (.names file)
	/**
    * Class constructor
	* @param strCfgFile [string]: config file path (.cfg file)
	* @param strWeightsFile [string]: weight file path (.weights file)
	* @param confThres [float]: confident threshold (default is 0.5f)
	* @param nmsThres [float]: non-maximum suppression (NMS) threshold (default is 0.4f)
    */
	CAIDnn(string strCfgFile, std::string strWeightsFile, float confThres = 0.5f, float nmsThres = 0.4f); //std::string strClassFile, 
	/**
    Class destructor
    */
	~CAIDnn();
	/**
    * Analyse the current frame
    * @param frame [Mat&]: frame to be analysed
	* @return analysed frame
    */
	Mat analysis(const Mat& frame);
private:
	/**
    * Postprocess the frame after running inference model on it
    * @param frame [Mat&]: frame to be postprocessed
	* @param outs [vector<Mat>&]: output of the inference model
    */
	void postprocess(Mat& frame, const std::vector<Mat>& outs);
	//vector<string> _classes;
	//mutex _mtx;
	//condition_variable _cond;
	//string _appPath;

	/**
	Confidence threshold
	*/
	float _confThres;

	/**
    NMS threshold
    */
	float _nmsThres;

	/**
	DNN instance
	*/
	dnn::Net _net;

	/**
    Layers' names
    */
	vector<string> _names;
};

