#include <jni.h>
#include <string>
//extern "C" rtmp 内部库已兼容
#include <rtmp.h>
#include <x264.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_test_rtmpdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    char version[50];
    sprintf(version,"librtmp version:%d",RTMP_LibVersion());
    x264_picture_t * picture = new x264_picture_t ;
    return env->NewStringUTF(version);
}