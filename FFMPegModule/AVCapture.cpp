#include "stdafx.h"
#include "AVCapture.h"

extern std::atomic<bool> _isRun;
extern std::atomic<int> dataReady;

int CAVCapture::readVideo(const char* strInputFile)
{
	av_register_all();
    avdevice_register_all();
    avcodec_register_all();
	pRFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pRFormatCtx, strInputFile, NULL, NULL) != 0)
        return -12;
    
    // Get video file information
    if (avformat_find_stream_info(pRFormatCtx, NULL) < 0)
    {
        avformat_close_input(&pRFormatCtx);
        pRFormatCtx = nullptr;
        return -13;

    }

    int videoindex = -1;
    for (int i = 0; i < pRFormatCtx->nb_streams; i++) {
        if (pRFormatCtx->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    }
    if (videoindex < 0)
    {
        avformat_close_input(&pRFormatCtx);
        pRFormatCtx = nullptr;
        return -1;
    }

    pRCodecCtx = pRFormatCtx->streams[videoindex]->codec;
    pRCodec = avcodec_find_decoder(pRCodecCtx->codec_id);
    if (pRCodec == nullptr)
        return -15;
    if (avcodec_open2(pRCodecCtx, pRCodec, nullptr) < 0)
        return -16;


    if (!_pDnn)
    {
        avcodec_close(pRCodecCtx);
        avformat_close_input(&pRFormatCtx);
        return -1;
    }
    
    pRFrame = av_frame_alloc();

    _format = AVPixelFormat::AV_PIX_FMT_RGB24;
    _nBytes = avpicture_get_size(_format, pRCodecCtx->width, pRCodecCtx->height);
    pRBuffer = static_cast<uint8_t*>(av_malloc(_nBytes * sizeof(uint8_t)));
    avpicture_fill(reinterpret_cast<AVPicture*>(pRFrame), pRBuffer, AVPixelFormat::AV_PIX_FMT_BGR24, pRCodecCtx->width, pRCodecCtx->height);
    
    pRDst = av_frame_alloc();
    avpicture_fill(reinterpret_cast<AVPicture*>(pRDst), pRBuffer, _format, pRCodecCtx->width, pRCodecCtx->height);

    _ySize = pRCodecCtx->width * pRCodecCtx->height;
    pRPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    av_new_packet(pRPacket, _ySize);

    _frameWidth = pRCodecCtx->width;
    _frameHeight = pRCodecCtx->height;
    _fps = pRCodecCtx->framerate;  //pFormatCtx->streams[videoindex]->r_frame_rate;
    _nFrames = 0;
    //_nFrames = pFormatCtx->streams[videoindex]->nb_frames;

    // debug output file information
 // cout << "--------------- File Information ----------------" << endl;;
    av_dump_format(pRFormatCtx, 0, strInputFile, 0);
    //  cout << "-------------------------------------------------" << endl;

    //pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);


    concurrency::task<void> t = concurrency::create_task([videoindex, this]()//, &frame
    {
            _bLoop = true;
            _thr_ai = thread(&CAVCapture::AIThread, this);

            while (av_read_frame(pRFormatCtx, pRPacket) >= 0)
            {
                if (pRPacket->stream_index == videoindex)// read a compressed data
                {
                    int success = 0;
                    int ret = avcodec_decode_video2(pRCodecCtx, pRFrame, &success, pRPacket);// decode a compressed data
                    if (ret < 0)
                        break;
                    if (success)
                    {
                        // to cut just above the width, in order to better display
                        SwsContext* pSwsCtx = sws_getContext(pRCodecCtx->width, pRCodecCtx->height, pRCodecCtx->pix_fmt, pRCodecCtx->width, pRCodecCtx->height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
                        sws_scale(pSwsCtx, (const uint8_t* const*)reinterpret_cast<AVPicture*>(pRFrame)->data, reinterpret_cast<AVPicture*>(pRFrame)->linesize, 0, pRFrame->height, reinterpret_cast<AVPicture*>(pRDst)->data, reinterpret_cast<AVPicture*>(pRDst)->linesize);
                        pushFrame(pRCodecCtx->height, pRCodecCtx->width, pRBuffer);
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        av_free_packet(pRPacket);
                        sws_freeContext(pSwsCtx);

                    }
                    else
                        break;
                }
                else
                {
                    av_free_packet(pRPacket);
                    av_init_packet(pRPacket);
                    break;
                }
            }
            waitForFinish();

            if (_bLoop) {
                _bLoop = false;
                if (_thr_ai.joinable())
                    _thr_ai.join();
            }

            //if (_pDnn) {
            //    delete _pDnn;
            //    _pDnn = nullptr;
            //}

            
            // close the file and release memory
            if (pRFormatCtx != nullptr)
            {
                //sws_freeContext(pSwsCtx);
                av_frame_free(&pRFrame);
                av_frame_free(&pRDst);
                av_free(pRBuffer);
                avcodec_close(pRCodecCtx);
                avformat_close_input(&pRFormatCtx);
            }

            //_isRun = false;
            dataReady = 1;
            Concurrency::wait(10);
            _cond.notify_all();
    });



    return 0;
}

int CAVCapture::writeVideo(const char* strOutputFile)
{
    //_isRun = true;


    av_register_all();
    avdevice_register_all();
    avcodec_register_all();
    int ret;

    // open output format context
    pWFormatCtx = nullptr;
    ret = avformat_alloc_output_context2(&pWFormatCtx, nullptr, nullptr, strOutputFile);
    if (ret < 0) 
        return 1;

    // open output IO context
    ret = avio_open2(&pWFormatCtx->pb, strOutputFile, AVIO_FLAG_WRITE, nullptr, nullptr);
    if (ret < 0)
        return 2;

    // create new video stream
    pCodec = avcodec_find_encoder(pWFormatCtx->oformat->video_codec);
    pStream = avformat_new_stream(pWFormatCtx, pCodec);
    if (!pStream)
        return 3;

    avcodec_get_context_defaults3(pStream->codec, pCodec);
    
    // open video encoder
    pWCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pWCodecCtx)
        return 4;

    pStream->codec->width = _frameWidth;
    pStream->codec->height = _frameHeight;
    pStream->codec->pix_fmt = pCodec->pix_fmts[0];
    pStream->codec->time_base = pStream->time_base = av_inv_q(_fps);
    pStream->r_frame_rate = pStream->avg_frame_rate = _fps;
    if (pWFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
        pStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // open video encoder
    ret = avcodec_open2(pStream->codec, pCodec, nullptr);
    if (ret < 0)
        return 5;

    avcodec_parameters_from_context(pStream->codecpar, pWCodecCtx);

    //if (!pSwsCtx)
    //    return -5;

    // allocate frame buffer for encoding
    pWFrame = av_frame_alloc();
    pWBuffer = static_cast<uint8_t*>(av_malloc(_nBytes * sizeof(uint8_t)));
    avpicture_fill(reinterpret_cast<AVPicture*>(pWFrame), pWBuffer, pStream->codec->pix_fmt, pWCodecCtx->width, pWCodecCtx->height);

    pWFrame->width = _frameWidth;
    pWFrame->height = _frameHeight;
    pWFrame->format = static_cast<int>(pStream->codec->pix_fmt);

    pWPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    av_new_packet(pWPacket, _ySize);

    
    concurrency::task<void> t = concurrency::create_task([this]() 
    {    
        avformat_write_header(pWFormatCtx, nullptr);
        int64_t frame_pts = 0;
        unsigned nb_frames = 0;
        bool end_of_stream = false;
        int got_pkt = 0;
        int ret;

        while (dataReady == 0) {             // (3)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        //encoding loop
        for(;;)
        {
            if (_oFrames.size() == 0)
            {
                avcodec_send_frame(pWCodecCtx, nullptr);
                end_of_stream = true;
            }

            if (!end_of_stream)
            {
                Mat image = _oFrames.front();

                // convert cv::Mat(OpenCV) to AVFrame(FFmpeg)
                const int stride[4] = { static_cast<int>(image.step[0]) };
                // initialize sample scaler
                SwsContext* pSwsCtx = sws_getContext(_frameWidth, _frameHeight, AVPixelFormat::AV_PIX_FMT_BGR24, _frameWidth, _frameHeight, pStream->codec->pix_fmt, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                if (!pSwsCtx)
                    break;
                //pSwsCtx = sws_getCachedContext(nullptr, _frameWidth, _frameHeight, AVPixelFormat::AV_PIX_FMT_BGR24, _frameWidth, _frameHeight, pStream->codec->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
                sws_scale(pSwsCtx, &image.data, stride, 0, image.rows, pWFrame->data, pWFrame->linesize);
                
                pWFrame->pts = frame_pts++;
                ret = avcodec_send_frame(pWCodecCtx, pWFrame);
                if (ret < 0) {
                    std::cerr << "fail to avcodec_send_frame: ret=" << ret << "\n";
                    break;
                }
                sws_freeContext(pSwsCtx);
                _oFrames.pop();
            }

            while ((ret = avcodec_receive_packet(pWCodecCtx, pWPacket)) >= 0) {
                // rescale packet timestamp
                pWPacket->duration = 1;
                av_packet_rescale_ts(pWPacket, pStream->codec->time_base, pStream->time_base);
                // write encoded packet
                av_write_frame(pWFormatCtx, pWPacket);
                av_packet_unref(pWPacket);
                //std::cout << nb_frames << '\r' << std::flush;  // dump progress
                ++nb_frames;
            }

            if (ret == AVERROR_EOF)
                break;
        }

        if (pWFormatCtx != nullptr)
        {
            av_write_trailer(pWFormatCtx);
            avio_close(pWFormatCtx->pb);
            
            av_packet_free(&pWPacket);
            av_frame_free(&pWFrame);

            //sws_freeContext(pSwsCtx);
            avcodec_close(pStream->codec);
            avcodec_free_context(&pWCodecCtx);
            avformat_free_context(pWFormatCtx);
        }

        dataReady = 0;
        Concurrency::wait(10);
    });


    return 0;
}

CAVCapture::CAVCapture()
{
    _pDnn = new CAIDnn("obj.names", "obj.cfg", "obj.weights", 0.5f);
    _bLoop = false;

    pRFormatCtx = nullptr;

    //_volSize = 0;
    //_pVolData = nullptr;
}

CAVCapture::~CAVCapture()
{
    if (_pDnn) {
        delete _pDnn;
        _pDnn = nullptr;
    }

    if (_pWriter)
    {
        delete _pWriter;
        _pWriter = nullptr;
    }
}

//int sCount = 0;
void CAVCapture::AIThread()
{
    Mat blob;
    vector<Mat> outs;
    while (_bLoop) {
        unique_lock<mutex> lock(_mtx);

        if (_iFrames.size() <= 0)
            _cond.wait(lock);

        if (_iFrames.size() <= 0)
            break;

        //Mat frame = _iFrames.front();
        Mat frame = _iFrames.front();
        //Mat frame_2 = frame.clone();
        _iFrames.pop();

        if(_pDnn)
            _pWriter->write(_pDnn->analysis(frame));
        //if (_pDnn)
        //{
        //    Mat frame_2 = frame.clone();
        //    //string inpFileName = format("C:\\Users\\sbnetHo\\Desktop\\Documents\\output\\%d_1.png", sCount);
        //    //string outFileName = format("C:\\Users\\sbnetHo\\Desktop\\Documents\\output\\%d_2.png", sCount++);
        //    //cv::imwrite(inpFileName, frame_2);
        //   //cv::imwrite(outFileName, _pDnn->analysis(frame_2));
        //    _pWriter->write(_pDnn->analysis(frame_2));
        //    //string fileName = "C:\\Users\\sbnetHo\\Desktop\\Documents\\output" + 
        //    //cv::imwrite()
        //    //_pWriter->write(_pDnn->analysis(frame));
        //    //Mat oFrame = frame.clone();
        //    //Mat pFrame = _pDnn->analysis(oFrame);
        //    //_pWriter->write(pFrame);
        //    //Mat oFrames = _pDnn->analysis(frame);
        //    //_pWriter->write(oFrames);
        //    //_pWriter->write(frame);
        //    //_pWriter->write(_pDnn->analysis(frame));
        //    //_oFrames.push(_pDnn->analysis(frame));
        //}

        lock.unlock();

        //if (_pDnn)
        //{
        //    Mat frame_2 = frame.clone();
        //    _pWriter->write(_pDnn->analysis(frame_2));
        //}
        //if (_pDnn)
        //    _pWriter->write(_pDnn->analysis(frame));
    }
}

void CAVCapture::pushFrame(int rows, int cols, uint8_t* buffer)
{
    //AutoCS cs(&_criticalSection);
    //cv::Size sz(width, height);
    cv::Mat frame(rows, cols, CV_8UC3, buffer);
    //Mat res = _pDnn->analysis(frame);
    //_iFrames.push(res);
    bool bNotify = false;
    unique_lock<mutex> lock(_mtx);

    int count = _iFrames.size();
    if (count <= 0)
        bNotify = true;
    _iFrames.push(frame);
    ++_nFrames;
    if (bNotify)
        _cond.notify_one();
}

void CAVCapture::waitForFinish()
{
    while (1) {
        unique_lock<mutex> lock(_mtx);
        if (_iFrames.size() <= 0) {
            break;
        }
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    _cond.notify_one();
}

int CAVCapture::doReadWrite(const char* strInputFile, const char* strOutputFile)
{
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();
    //pRFormatCtx = NULL;
    pRFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pRFormatCtx, strInputFile, NULL, NULL) != 0)
        return -12;

    // Get video file information
    if (avformat_find_stream_info(pRFormatCtx, NULL) < 0)
    {
        avformat_close_input(&pRFormatCtx);
        pRFormatCtx = nullptr;
        return -13;

    }

    int videoindex = -1;
    for (int i = 0; i < pRFormatCtx->nb_streams; i++) {
        if (pRFormatCtx->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    }
    if (videoindex < 0)
    {
        avformat_close_input(&pRFormatCtx);
        pRFormatCtx = nullptr;
        return -1;
    }

    pRCodecCtx = pRFormatCtx->streams[videoindex]->codec;
    pRCodec = avcodec_find_decoder(pRCodecCtx->codec_id);
    if (pRCodec == nullptr)
        return -15;
    if (avcodec_open2(pRCodecCtx, pRCodec, nullptr) < 0)
        return -16;


    if (!_pDnn)
    {
        avcodec_close(pRCodecCtx);
        avformat_close_input(&pRFormatCtx);
        return -1;
    }

    pRFrame = av_frame_alloc();

    _format = AVPixelFormat::AV_PIX_FMT_RGB24;
    _nBytes = avpicture_get_size(_format, pRCodecCtx->width, pRCodecCtx->height);
    pRBuffer = static_cast<uint8_t*>(av_malloc(_nBytes * sizeof(uint8_t)));
    avpicture_fill(reinterpret_cast<AVPicture*>(pRFrame), pRBuffer, AVPixelFormat::AV_PIX_FMT_BGR24, pRCodecCtx->width, pRCodecCtx->height);

    pRDst = av_frame_alloc();
    avpicture_fill(reinterpret_cast<AVPicture*>(pRDst), pRBuffer, _format, pRCodecCtx->width, pRCodecCtx->height);

    _ySize = pRCodecCtx->width * pRCodecCtx->height;
    pRPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    av_new_packet(pRPacket, _ySize);

    _frameWidth = pRCodecCtx->width;
    _frameHeight = pRCodecCtx->height;
    Size frameSize(_frameWidth, _frameHeight);
    _fps = pRCodecCtx->framerate;  //pFormatCtx->streams[videoindex]->r_frame_rate;
    _nFrames = 0;
    double videoFPS = av_q2d(_fps);
    _pWriter = new VideoWriter(string(strOutputFile), VideoWriter::fourcc('M', 'J', 'P', 'G'), av_q2d(_fps), frameSize);

    av_dump_format(pRFormatCtx, 0, strInputFile, 0);

    concurrency::task<void> t = concurrency::create_task([videoindex, this]()//, &frame
        {
            _bLoop = true;
            _thr_ai = thread(&CAVCapture::AIThread, this);

            while (av_read_frame(pRFormatCtx, pRPacket) >= 0)
            {
                if (pRPacket->stream_index == videoindex)// read a compressed data
                {
                    int success = 0;
                    int ret = avcodec_decode_video2(pRCodecCtx, pRFrame, &success, pRPacket);// decode a compressed data
                    if (ret < 0)
                        break;
                    if (success)
                    {
                        // to cut just above the width, in order to better display
                        SwsContext* pSwsCtx = sws_getContext(pRCodecCtx->width, pRCodecCtx->height, pRCodecCtx->pix_fmt, pRCodecCtx->width, pRCodecCtx->height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
                        sws_scale(pSwsCtx, (const uint8_t* const*)reinterpret_cast<AVPicture*>(pRFrame)->data, reinterpret_cast<AVPicture*>(pRFrame)->linesize, 0, pRFrame->height, reinterpret_cast<AVPicture*>(pRDst)->data, reinterpret_cast<AVPicture*>(pRDst)->linesize);
                        pushFrame(pRCodecCtx->height, pRCodecCtx->width, pRBuffer);
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        av_free_packet(pRPacket);
                        sws_freeContext(pSwsCtx);

                    }
                    else
                        break;
                }
                else
                {
                    av_free_packet(pRPacket);
                    av_init_packet(pRPacket);
                    break;
                }
            }
            waitForFinish();

            if (_bLoop) {
                _bLoop = false;
                if (_thr_ai.joinable())
                    _thr_ai.join();
            }

            //if (_pDnn) {
            //    delete _pDnn;
            //    _pDnn = nullptr;
            //}


            // close the file and release memory
            if (pRFormatCtx != nullptr)
            {
                //sws_freeContext(pSwsCtx);
                av_frame_free(&pRFrame);
                av_frame_free(&pRDst);
                av_free(pRBuffer);
                avcodec_close(pRCodecCtx);
                avformat_close_input(&pRFormatCtx);
                _pWriter->release();
            }

            _isRun = false;
            //dataReady = 1;
            //Concurrency::wait(10);
            _cond.notify_all();

            return;
        });

    return 0;
}
