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

    y_len = width*heigh;
    uv_len = y_len/4;

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

void VideoChannel::encodeData(signed char *data) {
    LOGI("encodeData");
    pthread_mutex_lock(&mutex);
    //拷贝数据 Y分量数据
    memcpy(picture->img.plane[0],data,y_len);
    //UV分量数据
    for (int i = 0; i < uv_len; ++i) {
        *(picture->img.plane[1]+i) = *(data+y_len+i*2+1);
        *(picture->img.plane[2]+i) = *(data+y_len+i*2);
    }

    x264_nal_t *nal = nullptr;
    int pi_nal;
    //(编码后数据)
    x264_picture_t pic_out;

    //调用编码
   int ret =  x264_encoder_encode(videoEncoder,&nal,&pi_nal,picture,&pic_out);
   if(ret < 0){
       LOGE("x264编码失败");
       pthread_mutex_unlock(&mutex);
       return;
   }

   int sps_len,pp_len;
   uint8_t sps[100];
   uint8_t pps[100];
   picture->i_pts+1;

   //进行头部sps和pps包
    for (int i = 0; i < pi_nal; ++i) {
        if(nal[i].i_type == NAL_SPS){
            //SPS数据
            sps_len = nal[i].i_payload-4;
            memcpy(sps,nal[i].p_payload+4,sps_len);
        }else if(nal[i].i_type == NAL_PPS){
            //PPS 数据
            pp_len = nal[i].i_payload-4;
            memcpy(pps,nal[i].p_payload+4,pp_len);
            sendSpsPps(sps,pps,sps_len,pp_len);
        }else{
            //I,P帧数据
            sendFrame(nal[i].i_type,nal[i].i_payload,nal[i].p_payload);
        }
    }

    pthread_mutex_unlock(&mutex);

}

void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    /**
     * 5:头部占位信息
     * 8:固定信息
     * sps_len:
     * 3:
     */
    int body_size = 5+8+sps_len+3+pps_len;

    RTMPPacket*  headerPkt = new RTMPPacket ;
    RTMPPacket_Alloc(headerPkt,body_size);
    int i = 0;
    //根据协议封装
    headerPkt->m_body[i++] = 0x17;

    headerPkt->m_body[i++] = 0x00;
    headerPkt->m_body[i++] = 0x00;
    headerPkt->m_body[i++] = 0x00;
    headerPkt->m_body[i++] = 0x00;

    //版本
    headerPkt->m_body[i++] = 0x01;

    headerPkt->m_body[i++] = sps[1];
    headerPkt->m_body[i++] = sps[2];
    headerPkt->m_body[i++] = sps[3];

    headerPkt->m_body[i++] = 0xFF;
    headerPkt->m_body[i++] = 0xE1;
    //sps长度
    //高两位（将长度存到两个位中）
    headerPkt->m_body[i++] = (sps_len>>8)&0xFF;
    headerPkt->m_body[i++] = sps_len&0xFF;

    memcpy(&headerPkt->m_body[i],sps,sps_len);
    i+=sps_len;

    headerPkt->m_body[i++] = 0x01;
    headerPkt->m_body[i++] = (pps_len>>8)&0xFF;
    headerPkt->m_body[i++] = pps_len&0XFF;
    memcpy(&headerPkt->m_body[i],pps,pps_len);

    i+= pps_len;

    //包数据结构处理
    headerPkt->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    headerPkt->m_nBodySize = body_size;
    //不重复就好
    headerPkt->m_nChannel = 10;
    headerPkt->m_nTimeStamp = 0;
    headerPkt->m_hasAbsTimestamp = 0;
    headerPkt->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    callback(headerPkt);
}

void VideoChannel::sendFrame(int type, int payload, uint8_t *pPayload) {
    LOGI("send Frame data")
    if(pPayload[2] == 0x00){
        pPayload+=4;
        payload-=4;
    }else if(pPayload[2] == 0x01){
        pPayload+=3;
        payload-=3;
    }

    int body_size = 5+4+payload;
    RTMPPacket * framePkt = new RTMPPacket ;
    RTMPPacket_Alloc(framePkt,body_size);

    framePkt->m_body[0] = 0x27;
    //关键帧
    if(type == NAL_SLICE_IDR){
        framePkt->m_body[0] = 0x17;
    }
    framePkt->m_body[1] = 0x01; // 重点是此字节 如果是1 帧类型（关键帧 非关键帧）， 如果是0一定是 sps pps
    framePkt->m_body[2] = 0x00;
    framePkt->m_body[3] = 0x00;
    framePkt->m_body[4] = 0x00;

    framePkt->m_body[5] = (payload>>24)&0xFF;
    framePkt->m_body[6] = (payload>>16)&0xFF;
    framePkt->m_body[7] = (payload>>8)&0xFF;
    framePkt->m_body[8] = (payload)&0xFF;
    memcpy(&framePkt->m_body[9],pPayload,payload);
    framePkt->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    framePkt->m_nChannel = 10;
    framePkt->m_nBodySize = body_size;
    framePkt->m_nTimeStamp = -1;
    framePkt->m_hasAbsTimestamp= 0;
    framePkt->m_headerType = RTMP_PACKET_SIZE_LARGE;
    callback(framePkt);
}

