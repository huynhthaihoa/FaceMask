#include "stdafx.h"
#include "ai_dnn.h"

CAIDnn::CAIDnn(std::string strClassFile, string strCfgFile, std::string strWeightsFile, float threshold)
{
    if (threshold >= 0.0f && threshold <= 1.0f)
        _threshold = threshold;

    HMODULE hModule;
    hModule = ::GetModuleHandle(nullptr); // handle of current module
    //ASSERT(hModule != 0);

    CString strExeFileName;
    //VERIFY(::GetModuleFileName(hModule, strExeFileName.GetBuffer(_MAX_PATH), _MAX_PATH));
    ::GetModuleFileName(hModule, strExeFileName.GetBuffer(_MAX_PATH), _MAX_PATH);
    strExeFileName.ReleaseBuffer();

    char Drive[_MAX_DRIVE];
    char Path[_MAX_PATH];
    char Filename[_MAX_FNAME];
    char Ext[_MAX_EXT];
    CT2A ascii(strExeFileName, CP_UTF8);
    _splitpath_s(ascii.m_psz, Drive, Path, Filename, Ext);

    appPath = string(Drive) + string(Path);
    //appPath.Format(_T("%s%s"), Drive, Path);
    //strcat(Drive, Path);

    //std::string classFile = std::format("{}{}{}", Drive, Path, strClassFile.c_str());
    //std::string classFile = Drive + Path;
    //appPath.Format(_T("%s%s"), Drive, Path);

    //std::string classFile = string(Drive) + string(Path) + strClassFile;
    
    ifstream ifs(appPath + strClassFile);
    string line;
    while (getline(ifs, line))
        _classes.push_back(line);

    _net = readNet(appPath + strWeightsFile, appPath + strCfgFile);

    _net.setPreferableBackend(DNN_BACKEND_CUDA);
    _net.setPreferableTarget(DNN_TARGET_CUDA_FP16);
//#ifdef _GPU
//    net.setPreferableBackend(DNN_BACKEND_CUDA);
//    net.setPreferableTarget(DNN_TARGET_CUDA_FP16);
//#else
//    net.setPreferableBackend(DNN_BACKEND_OPENCV);
//    net.setPreferableTarget(DNN_TARGET_CPU);
//#endif

    vector<int> _outLayers = _net.getUnconnectedOutLayers();
    vector<string> _layersNames = _net.getLayerNames();

    _names.resize(_outLayers.size());
    for (int i = 0; i < _outLayers.size(); i++)
        _names[i] = _layersNames[_outLayers[i] - 1];

    //_bLoop = true;
    //_thr_ai = thread{ &CAIDnn::AIThread, this, net };

}

CAIDnn::~CAIDnn()
{
    //if (_bLoop) {
    //	_bLoop = false;
    //	_thr_ai.join();
    //}
}

Mat CAIDnn::analysis(Mat& frame)
{
    Mat blob;
    vector<Mat> outs;

    if (!_net.empty()) {
        blobFromImage(frame, blob, 1 / 255.0, cv::Size(608, 608), Scalar(0, 0, 0), true, false);
        
        _net.setInput(blob);
        _net.forward(outs, _names);
    }



    Mat frame2 = frame.clone();
    postprocess(frame2, outs);
    return frame2;// .clone();
}


void CAIDnn::postprocess(Mat& frame, const std::vector<Mat>& outs)
{
    float confThreshold = _threshold, nmsThreshold = 0.4f;

    float prob = 0.0;
    int cx = 0;
    int cy = 0;

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<Rect> boxes;
    for (size_t i = 0; i < outs.size(); ++i) {
        // Network produces output blob with a shape NxC where N is a number of
        // detected objects and C is a number of classes + 4 where the first 4
        // numbers are [center_x, center_y, width, height]
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold) {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(Rect(left, top, width, height));
            }
        }
    }

    std::vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];

        int classId = classIds[idx];

        if (classId == 1)
        {
            Rect region(box.x, box.y, box.width, box.height);
            //Put Gaussian blur filter on detected region
            GaussianBlur(frame(region), frame(region), Size(0, 0), 4);
            //Draw red rectangle around detected border
            cv::rectangle(frame, region, CV_RGB(255, 0, 0));
        }
    }

}
