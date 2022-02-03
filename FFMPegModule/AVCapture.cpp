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
        closeReading();
        closeWriting();
        _isRun = false;
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

        pair<Mat, int64_t> p = _frames.front();
        Mat frame = p.first;
        int64_t pts = p.second;
        _frames.pop();

        //lock.unlock();

        if (_pDnn)
        {
            pair<Mat, int64_t> r = _pDnn->analysis(frame, pts);
            writeFrame(r.first, r.second);
        }

        lock.unlock();
    }
}

void CAVCapture::pushFrame(int rows, int cols, uint8_t* buffer, int64_t pts)
{
    bool bNotify = false;
    unique_lock<mutex> lock(_mtx);

    if (_frames.size() <= 0)
        bNotify = true;

    cv::Mat frame(rows, cols, CV_8UC3, buffer);
    pair<Mat, int64_t> p;
    p.first = frame;
    p.second = pts;
    _frames.push(p);

    if (bNotify)
        _cond.notify_one();
}

void CAVCapture::waitForFinish()
{
    while (1) {
        unique_lock<mutex> lock(_mtx);
        if (_frames.size() <= 0) {
            break;
        }
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    _cond.notify_one();
}

void CAVCapture::writeFrame(Mat frame, int64_t pts)
{
    //1st step - convert Mat into AvFrame
    struct SwsContext* sws_ctx_bgr_yuv = NULL;
    sws_ctx_bgr_yuv = sws_getContext(pWCodecCtx->width,
        pWCodecCtx->height,
        AVPixelFormat::AV_PIX_FMT_BGR24,
        pWCodecCtx->width,
        pWCodecCtx->height,
        pWCodecCtx->pix_fmt //AV_PIX_FMT_YUV420p
        , SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    const int kStide[] = { (int)frame.step[0] };
    sws_scale(sws_ctx_bgr_yuv, &frame.data, kStide, 0, frame.rows, pWFrame->data, pWFrame->linesize);
    pWFrame->pts = pts;

    sws_freeContext(sws_ctx_bgr_yuv);

    //2nd step - encode frame and send to video
    if (avcodec_send_frame(pWCodecCtx, pWFrame) < 0)
        exit(1);

    flushPackets();
}

int CAVCapture::openReading(const char* strInputFile, int &videoindex)
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

    //int videoindex = -1;
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
        return -3;
    }

    pRCodecCtx = pRFormatCtx->streams[videoindex]->codec;
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

    _nFrames = 0;

    return 0;
}

int CAVCapture::openWriting(const char* strOutputFile)
{
    avformat_alloc_output_context2(&pWFormatCtx, nullptr, nullptr, strOutputFile);
    if (!pWFormatCtx)
        return 1;

    //create codec
    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pCodec)
        return 2;

    pStream = avformat_new_stream(pWFormatCtx, nullptr);
    if (!pStream)
        return 3;

    pStream->id = (int)(pWFormatCtx->nb_streams - 1);

    //create codec context
    pWCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pWCodecCtx)
        return 4;

    //pWPacket = av_packet_alloc();
    //if (!pWPacket)
    //    return 5;

    /* put sample parameters */
    //pWCodecCtx->bit_rate = 1000000;
    /* resolution must be a multiple of two */
    pWCodecCtx->width = pRCodecCtx->width;
    pWCodecCtx->height = pRCodecCtx->height;
    /* frames per second */
    //pWCodecCtx->time_base = pRCodecCtx->time_base;
    //pWCodecCtx->framerate = pRCodecCtx->framerate;
    //pWCodecCtx->time_base.num = pWCodecCtx->framerate.den = 1;// (AVRational) { 1, 30 };
    //pWCodecCtx->time_base.den = pWCodecCtx->framerate.num = 30;// (AVRational) { 1, 30 };
    //pWCodecCtx->framerate = (AVRational){ 30, 1 };
    
    pWCodecCtx->gop_size = pRCodecCtx->gop_size;
    pWCodecCtx->max_b_frames = pRCodecCtx->max_b_frames;
    //pWCodecCtx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;
    pWCodecCtx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;// pRCodecCtx->pix_fmt;

    pStream->time_base = av_d2q(1 / 30.0, 120);
    pWCodecCtx->time_base = pStream->time_base;

    if (pCodec->id == AV_CODEC_ID_H264) {
        av_opt_set(pWCodecCtx, "preset", "superfast", 0);
        
        av_opt_set(pWCodecCtx, "tune", "zerolatency", 0);
    }

   

    /* open it */
    if (avcodec_open2(pWCodecCtx, pCodec, nullptr) < 0)
        return 5;

    //open output file
    //pFile = fopen(strOutputFile, "wb");
    //if (!pFile)
    //    return 7;

    pWFrame = av_frame_alloc();
    if (!pWFrame)
        return 6;

    pWFrame->format = (int)pWCodecCtx->pix_fmt;
    pWFrame->width = pWCodecCtx->width;
    pWFrame->height = pWCodecCtx->height;

    if (av_frame_get_buffer(pWFrame, 32) < 0)
        return 7;

    //ret = avcodec_parameters_from_context(mContext.stream->codecpar, mContext.codec_context);
    if (avcodec_parameters_from_context(pStream->codecpar, pWCodecCtx) < 0)
        return 8;
    //{
    //    std::cout << "could not copy the stream parameters" << std::endl;
    //    break;
    //}

    //mContext.sws_context = sws_getContext(
    //    mContext.codec_context->width, mContext.codec_context->height, params.src_format,   // src
    //    mContext.codec_context->width, mContext.codec_context->height, params.dst_format, // dst
    //    SWS_BICUBIC, nullptr, nullptr, nullptr
    //);
    //if (!mContext.sws_context)
    //{
    //    std::cout << "could not initialize the conversion context" << std::endl;
    //    break;
    //}

    av_dump_format(pWFormatCtx, 0, strOutputFile, 1);

    avio_open(&pWFormatCtx->pb, strOutputFile, AVIO_FLAG_WRITE);
    if (avio_open(&pWFormatCtx->pb, strOutputFile, AVIO_FLAG_WRITE) != 0)
        return 9;
    //{
    //    std::cout << "could not open " << filename << std::endl;
    //    break;
    //}

    int ret = avformat_write_header(pWFormatCtx, nullptr);
    if (ret < 0)
    {
        //std::cout << "could not write" << std::endl;
        ret = avio_close(pWFormatCtx->pb);
        if (ret != 0)
            return 10;
        //    std::cout << "failed to close file" << std::endl;
        //break;
    }



    return 0;
}

void CAVCapture::closeReading()
{
    if (pRFormatCtx != nullptr)
    {
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
        avformat_free_context(pWFormatCtx);
        avcodec_free_context(&pWCodecCtx);
        avcodec_close(pWCodecCtx);
        av_frame_free(&pWFrame);
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

int CAVCapture::doReadWrite(const char* strInputFile, const char* strOutputFile)
{
    int videoindex = -1;
    int ret = openReading(strInputFile, videoindex);
    if (ret != 0)
        return ret;
    
    ret = openWriting(strOutputFile);
    if (ret != 0)
        return ret;

    concurrency::task<void> t = concurrency::create_task([videoindex, this]()
        {
            _bLoop = true;
            _thr_ai = thread(&CAVCapture::AIThread, this);

            while (av_read_frame(pRFormatCtx, pRPacket) >= 0 && _isRun)
            {
                if (pRPacket->stream_index == videoindex)// read a compressed data
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
                        SwsContext* pSwsCtx = sws_getContext(pRCodecCtx->width, pRCodecCtx->height, pRCodecCtx->pix_fmt, pRCodecCtx->width, pRCodecCtx->height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
                        sws_scale(pSwsCtx, (const uint8_t* const*)reinterpret_cast<AVPicture*>(pRFrame)->data, reinterpret_cast<AVPicture*>(pRFrame)->linesize, 0, pRFrame->height, reinterpret_cast<AVPicture*>(pRDst)->data, reinterpret_cast<AVPicture*>(pRDst)->linesize);
                        pushFrame(pRCodecCtx->height, pRCodecCtx->width, pRBuffer, _nFrames++);
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

            closeReading();

            closeWriting();

            _isRun = false;
            Concurrency::wait(10);
            _cond.notify_all();
        });

    return 0;
}