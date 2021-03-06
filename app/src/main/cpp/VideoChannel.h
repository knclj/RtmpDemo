//
// Created by cailianjun on 7/10/21.
//

#ifndef RTMPDEMO_VIDEOCHANNEL_H
#define RTMPDEMO_VIDEOCHANNEL_H
#include <pthread.h>
#include <x264.h>
#include <rtmp.h>
#include "util.h"


class VideoChannel {



public:

    typedef void (*VideoCallback)(RTMPPacket * pk);

    VideoChannel();
    ~VideoChannel();

private:
    pthread_mutex_t mutex;
    VideoCallback callback;
    int width;
    int height;
    int fps;
    int bitrate;
    int y_len;
    int uv_len;
    x264_t *videoEncoder;
    x264_picture_t * picture;
public:

    void  setVideoCallback(VideoCallback callback);
   void initVideoEncoder(int width,int heigh,int fps,int bitrate);

   void encodeData(signed char* data);

    void sendSpsPps(uint8_t sps[100], uint8_t pps[100], int sps_len, int pps_len);

    void sendFrame(int type, int payload, uint8_t *pPayload);
};


#endif //RTMPDEMO_VIDEOCHANNEL_H
