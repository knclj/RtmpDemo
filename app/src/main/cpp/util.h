//
// Created by cailianjun on 6/6/21.
//

#ifndef FFMPEGLEARNDEMO_UTIL_H
#define FFMPEGLEARNDEMO_UTIL_H

#include <android/log.h>
#include <queue>
#include <pthread.h>


#define TAG "NativePush"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG,  __VA_ARGS__);
#define DELETE(object) if (object) { delete object; object = 0;}



#endif //FFMPEGLEARNDEMO_UTIL_H
