package com.test.rtmpdemo;

/**
 * 推送与C++交互类
 */
public class NativePush {

    private int width;
    private int height;
    private int fps;
    private int bitare;


    public NativePush(int width,int height,int fps,int bitare){
        this.width = width;
        this.height = height;
        this.fps = fps;
        this.bitare = bitare;
    }

    public void init(){
        native_init();
    }
    /**
     * rtmp 地址
     * @param path
     */
    public void startPush(String path){
        native_start(path);
    }

    public void stopPush(){
        native_stop();
    }

    public void release(){
        native_release();
    }

    public void videoEncoderInit(){
        native_initVideoEncoder(this.width,this.height,this.fps,this.bitare);
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
