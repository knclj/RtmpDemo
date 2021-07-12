#include <jni.h>
#include <string>
//extern "C" rtmp 内部库已兼容
#include <rtmp.h>
#include <x264.h>
#include "VideoChannel.h"
#include "util.h"
#include "SafeQueue.h"


VideoChannel * videoChannel;
SafeQueue<RTMPPacket *> packets;
bool isStart;
uint32_t start_time;
pthread_t pid_start;
bool realyPush;


void releasePackets(RTMPPacket **pkt){
    if(pkt){
        RTMPPacket_Free(*pkt);
        delete pkt;
        pkt = nullptr;
    }
}

void callback(RTMPPacket * pkt){
    if(pkt){
        if(pkt->m_nTimeStamp == -1){
            pkt->m_nTimeStamp = RTMP_GetTime() - start_time;
        }
        packets.insertToQueue(pkt);
    }
}
/**
 * 在子线程中，将包从队列中取出来并发送
 * @param param
 * @return
 */
void* task_start(void * param){
    char* url = static_cast<char *>(param);
    RTMP *rtmp = nullptr;
    int ret;
    do{
        //第一步alloc rtmp结构体
       rtmp = RTMP_Alloc();
       if(!rtmp){
           LOGE("RTMP_Alloc failed")
           break;
       }
       //分配内存
       RTMP_Init(rtmp);
       //5s timeout
       rtmp->Link.timeout = 5;
       //设置地址
       ret = RTMP_SetupURL(rtmp,url);
       if(!ret){
           LOGE("RTMP_SetupURL failed")
           break;
       }
       //开启输出模式
       RTMP_EnableWrite(rtmp);

       //建立连接

      ret =  RTMP_Connect(rtmp, nullptr);
      if(!ret){
         LOGE("RTMP_Connect failed");
          break;
      }

     ret = RTMP_ConnectStream(rtmp,0);
      if(!ret){
          LOGE("RTMP_ConnectStream failed");
          break;
      }
      start_time = RTMP_GetTime();
      //推流开始
      realyPush = true;
      packets.setWork(1);
      RTMPPacket * packet;
      while(realyPush){
         ret = packets.getQueueAndDel(packet);
         if(!ret){
             LOGE("getQueueAndDel failed")
             break;
         }
         if(!realyPush){
             LOGE("realyPush false")
             break;
         }
         if(!packet){
             continue;
         }
         packet->m_nInfoField2 = rtmp->m_stream_id;
         ret = RTMP_SendPacket(rtmp,packet,1);
         releasePackets(&packet);
          if(!ret){
              LOGE("RTMP_SendPacket failed")
              break;
          }
      }
      releasePackets(&packet);

    }while (false);
    isStart = false;
    realyPush = false;
    packets.setWork(0);
    packets.clear();
    if(rtmp){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    DELETE(url);
    return nullptr;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_test_rtmpdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    char version[50];
    sprintf(version,"librtmp version:%x",RTMP_LibVersion());
    x264_picture_t * picture = new x264_picture_t ;
    return env->NewStringUTF(version);
}extern "C"

JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel = new VideoChannel();
    videoChannel->setVideoCallback(callback);
    packets.setReleaseCallback(releasePackets);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1start(JNIEnv *env, jobject thiz, jstring path) {
    if(isStart){
        return;
    }
    isStart = true;
    const char* path_ = env->GetStringUTFChars(path, nullptr);
    char* url = new char(strlen(path_)+1);
    strcpy(url,path_);
    pthread_create(&pid_start, nullptr,task_start,url);
    env->ReleaseStringUTFChars(path,path_);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1stop(JNIEnv *env, jobject thiz) {
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1release(JNIEnv *env, jobject thiz) {
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1initVideoEncoder(JNIEnv *env, jobject thiz, jint width,
                                                           jint height, jint fps, jint bitrate) {
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray bytes) {
}