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
    sprintf(version,"librtmp version:%x",RTMP_LibVersion());
    x264_picture_t * picture = new x264_picture_t ;
    return env->NewStringUTF(version);
}extern "C"

JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1init(JNIEnv *env, jobject thiz) {
    // TODO: implement native_init()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1start(JNIEnv *env, jobject thiz, jstring path) {
    // TODO: implement native_start()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1stop(JNIEnv *env, jobject thiz) {
    // TODO: implement native_stop()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1release(JNIEnv *env, jobject thiz) {
    // TODO: implement native_release()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1initVideoEncoder(JNIEnv *env, jobject thiz, jint width,
                                                           jint height, jint fps, jint bitrate) {
    // TODO: implement native_initVideoEncoder()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_test_rtmpdemo_NativePush_native_1pushVideo(JNIEnv *env, jobject thiz, jbyteArray bytes) {
    // TODO: implement native_pushVideo()
}