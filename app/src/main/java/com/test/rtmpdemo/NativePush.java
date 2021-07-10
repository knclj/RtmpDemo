package com.test.rtmpdemo;

/**
 * 推送与C++交互类
 */
public class NativePush {

    private int cameraId;
    private int width;
    private int height;
    private int fps;
    private int bitare;


    public NativePush(int camareId,int width,int height,int fps,int bitare){
        this.cameraId = camareId;
        this.width = width;
        this.height = height;
        this.fps = fps;
        this.bitare = bitare;
    }

    public native void native_init();

    /**
     * 直播地址
     * @param path
     */
    public native void native_start(String path);

    public native void native_stop();

    public native void native_release();

    //视频直播推送
    public native void native_initVideoEncoder(int width,int height,int fps,int bitrate);
    public native void native_pushVideo(byte[] bytes);
}
