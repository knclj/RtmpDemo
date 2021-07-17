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
    this->width = width;
    this->height = heigh;
    this->fps = fps;
    this->bitrate = bitrate;
    if(!videoEncoder){
        x264_encoder_close(videoEncoder);
        videoEncoder = nullptr;
    }


}

