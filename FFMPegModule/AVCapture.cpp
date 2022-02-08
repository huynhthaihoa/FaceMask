#include "stdafx.h"
#include "AVCapture.h"

extern std::atomic<bool> _isRun;


CAVCapture::CAVCapture()
{
    _pDnn = new CAIDnn("obj.names", "obj.cfg", "obj.weights");// , 0.5f);
    _bLoop = false;
    
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();
}

CAVCapture::~CAVCapture()
{

    if (_pDnn) {
        delete _pDnn;
        _pDnn = nullptr;
    }

    if (_isRun == true)
    {
        //closeReading();
        //closeWriting();
        _isRun = false;
        _bLoop = false;
    }
}

void CAVCapture::AIThread()
{
    while (_bLoop) {
        unique_lock<mutex> lock(_mtx);

        if (_frames.size() <= 0)
            _cond.wait(lock);

        if (_frames.size() <= 0)
            break;

        Mat frame = _frames.front();
        _frames.pop();

        if (_isRun && _pDnn)
        {
            Mat res = _pDnn->analysis(frame);
            if (writeFrame(res) != 0)
                _isRun = false;
        }

        lock.unlock();
    }
}

void CAVCapture::pushFrame(int rows, int cols, uint8_t* buffer)
{
    bool bNotify = false;
    unique_lock<mutex> lock(_mtx);

    if (_frames.size() <= 0)
        bNotify = true;

    cv::Mat frame(rows, cols, CV_8UC3, buffer);
    _frames.push(frame);

    if (bNotify)
        _cond.notify_one();
}

void CAVCapture::waitForFinish()
{
    while (1) {
        unique_lock<mutex> lock(_mtx);

        if (!_isRun || _frames.size() <= 0) {
            break;
        }
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    _cond.notify_one();
}

int CAVCapture::writeFrame(Mat frame)
{
    if (_videoDuration != 0 && _nFrames == 0)
    {
        //1st step - close writing on current file
        if (_idx != -1)
            closeWriting();

        //2nd step - open writing on new file
        ++_idx;
        int64_t totalTime = _idx * _videoDuration;
        int64_t second = totalTime % 60;
        int64_t minute = totalTime / 60;
        int64_t hour = minute / 60;
        minute %= 60;

        string strTime = string_format("%dsec_%dh%02dm%02ds", _videoDuration, hour, minute, second);
        string strOutputFile = _strOutputName + "_" + strTime + _strOutputExt;
        if (openWriting(strOutputFile.c_str()) != 0)
            return 1;
    }

    //1st step - convert Mat into AvFrame
    const int kStide[] = { (int)frame.step[0] };
    sws_scale(pWSwsCtx, &frame.data, kStide, 0, frame.rows, pWFrame->data, pWFrame->linesize);
    pWFrame->pts = _nFrames++;
    
    if (_videoDuration != 0 && _nFrames >= _frameDuration)
        _nFrames = 0;

    //2nd step - encode frame and send to video
    if (avcodec_send_frame(pWCodecCtx, pWFrame) < 0)
        return 2;

    if (flushPackets())
        return 0;

    return 3;
}

int CAVCapture::openReading(const char* strInputFile)
{
    pRFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pRFormatCtx, strInputFile, NULL, NULL) != 0)
        return -1;

    // Get video file information
    if (avformat_find_stream_info(pRFormatCtx, NULL) < 0)
    {
        avformat_close_input(&pRFormatCtx);
        pRFormatCtx = nullptr;
        return -2;
    }

    _videoIndex = -1;

    for (int i = 0; i < pRFormatCtx->nb_streams; i++) {
        if (pRFormatCtx->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            _videoIndex = i;
            break;
        }
    }
    if (_videoIndex < 0)
    {
        avformat_close_input(&pRFormatCtx);
        pRFormatCtx = nullptr;
        return -3;
    }

    pRCodecCtx = pRFormatCtx->streams[_videoIndex]->codec;
    pRCodec = avcodec_find_decoder(pRCodecCtx->codec_id);
    if (pRCodec == nullptr)
        return -4;
    if (avcodec_open2(pRCodecCtx, pRCodec, nullptr) < 0)
        return -5;


    if (!_pDnn)
    {
        avcodec_close(pRCodecCtx);
        avformat_close_input(&pRFormatCtx);
        return -6;
    }

    pRSwsCtx = sws_getContext(pRCodecCtx->width, pRCodecCtx->height, pRCodecCtx->pix_fmt, pRCodecCtx->width, pRCodecCtx->height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    if (!pRSwsCtx)
        return -7;

    pRFrame = av_frame_alloc();
    _format = AVPixelFormat::AV_PIX_FMT_RGB24;
    _nBytes = av_image_get_buffer_size(AVPixelFormat::AV_PIX_FMT_RGB24, pRCodecCtx->width, pRCodecCtx->height, 32);
    pRBuffer = static_cast<uint8_t*>(av_malloc(_nBytes * sizeof(uint8_t)));
    avpicture_fill(reinterpret_cast<AVPicture*>(pRFrame), pRBuffer, AVPixelFormat::AV_PIX_FMT_BGR24, pRCodecCtx->width, pRCodecCtx->height);

    pRDst = av_frame_alloc();
    av_image_fill_arrays(reinterpret_cast<AVPicture*>(pRDst)->data, reinterpret_cast<AVPicture*>(pRDst)->linesize, pRBuffer, _format, pRCodecCtx->width, pRCodecCtx->height, 1);

    _ySize = pRCodecCtx->width * pRCodecCtx->height;
    pRPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    av_new_packet(pRPacket, _ySize);

    return 0;
}

int CAVCapture::openWriting(const char* strOutputFile)
{
    //guess the output format from the output file name
    pWOutputFmt = av_guess_format(nullptr, strOutputFile, nullptr);
    if (!pWOutputFmt)
        return 1;

    //allocate AVFormatContext
    avformat_alloc_output_context2(&pWFormatCtx, nullptr, nullptr, strOutputFile);
    if (!pWFormatCtx)
        return 2;

    //find a registered encoder
    pCodec = avcodec_find_encoder(pWOutputFmt->video_codec);
    if (!pCodec)
        return 3;

    //add a new stream to media file
    pStream = avformat_new_stream(pWFormatCtx, nullptr);
    if (!pStream)
        return 4;

    pStream->id = (int)(pWFormatCtx->nb_streams - 1);

    //create codec context
    pWCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pWCodecCtx)
        return 5;

    //pWPacket = av_packet_alloc();
    //if (!pWPacket)
    //    return 5;

    /* put sample parameters */
    //pWCodecCtx->bit_rate = 1000000;
    /* resolution must be a multiple of two */
    pWCodecCtx->width = pRCodecCtx->width;
    pWCodecCtx->height = pRCodecCtx->height;
    pWCodecCtx->codec_id = pWOutputFmt->video_codec;
    pWCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    /* frames per second */
    //pWCodecCtx->time_base = pRCodecCtx->time_base;
    //pWCodecCtx->framerate = pRCodecCtx->framerate;
    //pWCodecCtx->time_base.num = pWCodecCtx->framerate.den = 1;// (AVRational) { 1, 30 };
    //pWCodecCtx->time_base.den = pWCodecCtx->framerate.num = 30;// (AVRational) { 1, 30 };
    //pWCodecCtx->framerate = (AVRational){ 30, 1 };
    pWCodecCtx->bit_rate = _bitRates;
    pWCodecCtx->gop_size = pRCodecCtx->gop_size;
    pWCodecCtx->max_b_frames = pRCodecCtx->max_b_frames;
    //pWCodecCtx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;
    pWCodecCtx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;// pRCodecCtx->pix_fmt;

    pStream->time_base = av_d2q(1 / _fps, 120);
    pWCodecCtx->time_base = pStream->time_base;

    if (pCodec->id == AV_CODEC_ID_H264) {
        av_opt_set(pWCodecCtx, "preset", "superfast", 0);
        
        av_opt_set(pWCodecCtx, "tune", "zerolatency", 0);
    }
    else if (pCodec->id == AV_CODEC_ID_H265)
    {
        av_opt_set(pWCodecCtx, "preset", "ultrafast", 0);
    }
   

    /* open it */
    if (avcodec_open2(pWCodecCtx, pCodec, nullptr) < 0)
        return 6;

    pWFrame = av_frame_alloc();
    if (!pWFrame)
        return 7;

    pWFrame->format = (int)pWCodecCtx->pix_fmt;
    pWFrame->width = pWCodecCtx->width;
    pWFrame->height = pWCodecCtx->height;

    //allocate buffer for new video data
    if (av_frame_get_buffer(pWFrame, 32) < 0)
        return 8;

    if (avcodec_parameters_from_context(pStream->codecpar, pWCodecCtx) < 0)
        return 9;

    pWSwsCtx = sws_getContext(pWCodecCtx->width,
        pWCodecCtx->height,
        AVPixelFormat::AV_PIX_FMT_BGR24,
        pWCodecCtx->width,
        pWCodecCtx->height,
        pWCodecCtx->pix_fmt //AV_PIX_FMT_YUV420p
        , SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    if (!pWSwsCtx)
        return 10;

    av_dump_format(pWFormatCtx, 0, strOutputFile, 1);

    if (avio_open(&pWFormatCtx->pb, strOutputFile, AVIO_FLAG_WRITE) != 0)
        return 11;
    
    if (avformat_write_header(pWFormatCtx, nullptr) < 0)
    {
        if (avio_close(pWFormatCtx->pb) != 0)
            return 12;
    }

    return 0;
}

void CAVCapture::closeReading()
{
    if (pRFormatCtx != nullptr)
    {
        sws_freeContext(pRSwsCtx);
        av_frame_free(&pRFrame);
        av_frame_free(&pRDst);
        av_free(pRBuffer);
        avcodec_close(pRCodecCtx);
        avformat_close_input(&pRFormatCtx);
    }
}

void CAVCapture::closeWriting()
{
    if (pWCodecCtx != nullptr)
    {
        avcodec_send_frame(pWCodecCtx, nullptr);

        flushPackets();

        av_write_trailer(pWFormatCtx);
        avio_close(pWFormatCtx->pb);
        sws_freeContext(pWSwsCtx);
        av_frame_free(&pWFrame);
        avcodec_free_context(&pWCodecCtx);
        avcodec_close(pWCodecCtx);
        avformat_free_context(pWFormatCtx);
    }
}

bool CAVCapture::flushPackets()
{
    int ret;
    do
    {
        AVPacket packet = { 0 };

        ret = avcodec_receive_packet(pWCodecCtx, &packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

        if (ret < 0)
        {
            //std::cout << "error encoding a frame: " << ret << std::endl;
            return false;
        }

        av_packet_rescale_ts(&packet, pWCodecCtx->time_base, pStream->time_base);
        packet.stream_index = pStream->index;

        ret = av_interleaved_write_frame(pWFormatCtx, &packet);
        av_packet_unref(&packet);
        if (ret < 0)
        {
            //std::cout << "error while writing output packet: " << ret << std::endl;
            return false;
        }
    } while (ret >= 0);

    return true;
}

int CAVCapture::doReadWrite(const char* strInputFile, const char* strOutputFile, int64_t bitRates, float fps, int64_t duration)
{

    //int videoindex = -1;
    int ret = openReading(strInputFile);
    if (ret != 0)
        return ret;
    
    _bitRates = bitRates;
    _fps = fps;
    if (duration != 0)
    {
        _videoDuration = duration;
        _frameDuration = _videoDuration * _fps;
        _strOutputName = strOutputFile;
        int idx = _strOutputName.find_last_of(".");
        _strOutputExt = _strOutputName.substr(idx);
        _strOutputName = _strOutputName.substr(0, idx);

    }
    else
    {
        _videoDuration = _frameDuration = 0;
        ret = openWriting(strOutputFile);
        if (ret != 0)
            return ret;
    }

    concurrency::task<void> t = concurrency::create_task([this]()
        {
            _nFrames = 0;
            _idx = -1;
            _bLoop = true;
            _thr_ai = thread(&CAVCapture::AIThread, this);

            while (_isRun && av_read_frame(pRFormatCtx, pRPacket) >= 0)
            {
                if (pRPacket->stream_index == _videoIndex)// read a compressed data
                {
                    int success = 0;

                    int ret = avcodec_send_packet(pRCodecCtx, pRPacket);
                    if (ret < 0)
                        break;

                    while (ret >= 0)
                    {
                        ret = avcodec_receive_frame(pRCodecCtx, pRFrame);
                        if (ret <= 0)
                        {
                            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                                success = 0;
                            else
                                success = 1;
                            break;
                        }
                    }

                    if (success)
                    {
                        // to cut just above the width, in order to better display
                        //SwsContext* pSwsCtx = sws_getContext(pRCodecCtx->width, pRCodecCtx->height, pRCodecCtx->pix_fmt, pRCodecCtx->width, pRCodecCtx->height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
                        //sws_scale(pSwsCtx, (const uint8_t* const*)reinterpret_cast<AVPicture*>(pRFrame)->data, reinterpret_cast<AVPicture*>(pRFrame)->linesize, 0, pRFrame->height, reinterpret_cast<AVPicture*>(pRDst)->data, reinterpret_cast<AVPicture*>(pRDst)->linesize);
                        sws_scale(pRSwsCtx, (const uint8_t* const*)reinterpret_cast<AVPicture*>(pRFrame)->data, reinterpret_cast<AVPicture*>(pRFrame)->linesize, 0, pRFrame->height, reinterpret_cast<AVPicture*>(pRDst)->data, reinterpret_cast<AVPicture*>(pRDst)->linesize);
                        pushFrame(pRCodecCtx->height, pRCodecCtx->width, pRBuffer);

                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        av_free_packet(pRPacket);
                        //sws_freeContext(pSwsCtx);
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

            closeReading();

            closeWriting();

            _isRun = false;
            Concurrency::wait(10);
            _cond.notify_all();
        });

    return 0;
}