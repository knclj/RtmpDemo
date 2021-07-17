#include "VideoChannel.h"
/**
 * 封装视频数据格式并push到工作队列中
 */
VideoChannel::VideoChannel() {
    pthread_mutex_init(&mutex,0);
}

VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&mutex);
}

void VideoChannel::setVideoCallback(VideoChannel::VideoCallback callback) {
    this->callback = callback;
}

void VideoChannel::initVideoEncoder(int width, int heigh, int fps, int bitrate) {

    pthread_mutex_lock(&mutex);

    this->width = width;
    this->height = heigh;
    this->fps = fps;
    this->bitrate = bitrate;
    if(!videoEncoder){
        x264_encoder_close(videoEncoder);
        videoEncoder = nullptr;
    }
    if(picture){
        x264_picture_clean(picture);
        DELETE(picture);
    }
    x264_param_t  param;
    x264_param_default_preset(&param,"ultrafast","zerolatency");
    //// 3.2 中等偏上的规格  自动用 码率，模糊程度，分辨率
    param.i_level_idc = 32;
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height =heigh;

    //不需要封装B帧,封装B导致编码效率降低
    param.i_bframe = 0;

    // 码率控制方式。CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.i_bitrate = bitrate/1000;
    param.rc.i_vbv_max_bitrate = bitrate/1000*1.2;
    // 设置了i_vbv_max_bitrate就必须设置buffer大小，码率控制区大小，单位Kb/s
    param.rc.i_vbv_buffer_size = bitrate/1000;
    //码率控制不是通过 timebase 和 timestamp，码率的控制，完全不用时间搓
    // ，而是通过 fps 来控制 码率（根据你的fps来自动控制）
    param.b_vfr_input = 0;

    param.i_fps_num = fps;
    param.i_fps_den = 1;

    // 1/25 为时间基
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_timebase_den;

    //没两个帧一次关键帧
    param.i_keyint_max = 2*fps;

    param.b_repeat_headers = 1;

    param.i_threads = 1;

    x264_param_apply_profile(&param,"baseline");

    picture = new x264_picture_t ;
    x264_picture_alloc(picture,param.i_csp,param.i_width,param.i_height);
    videoEncoder = x264_encoder_open(&param);
    if(videoEncoder){
        LOGI("编码器打开成功")
    }
    pthread_mutex_unlock(&mutex);
}

