package com.test.rtmpdemo;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.Camera;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.graphics.ImageFormat;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.util.Size;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.google.common.util.concurrent.ListenableFuture;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private ListenableFuture<ProcessCameraProvider> cameraProviderFuture;
    private ProcessCameraProvider cameraProvider;
    private int CAMERA_INDEX = CameraSelector.LENS_FACING_FRONT;
    private PreviewView previewView;
    private NativePush nativePush;


    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        this.requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.CAMERA},100);
        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        previewView = findViewById(R.id.previewView);
        cameraProviderFuture = ProcessCameraProvider.getInstance(this);
        nativePush = new NativePush(640,480,25,800000);
        cameraProviderFuture.addListener(new Runnable() {
            @Override
            public void run() {
                try {
                   cameraProvider  = cameraProviderFuture.get();
                    bindPreview();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }, ContextCompat.getMainExecutor(this));
    }

    private void bindPreview() {
        if(cameraProvider == null){
            return;
        }
        if(nativePush != null){
            nativePush.videoEncoderInit();
        }
        cameraProvider.unbindAll();
        Preview preview = new  Preview.Builder().build();
        if(CAMERA_INDEX == CameraSelector.LENS_FACING_FRONT){
            CAMERA_INDEX = CameraSelector.LENS_FACING_BACK;
        }else{
            CAMERA_INDEX = CameraSelector.LENS_FACING_FRONT;
        }
        CameraSelector cameraSelector = new CameraSelector.Builder()
                .requireLensFacing(CAMERA_INDEX)
                .build();
        preview.setSurfaceProvider(previewView.getSurfaceProvider());
        ImageAnalysis imageAnalysis =
                new ImageAnalysis.Builder()
                        .setTargetResolution(new Size(640, 480))
                        .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                        .build();

        imageAnalysis.setAnalyzer(ContextCompat.getMainExecutor(this), new ImageAnalysis.Analyzer() {
            @Override
            public void analyze(@NonNull ImageProxy image) {
              int rotationDegress =  image.getImageInfo().getRotationDegrees();
              Log.d(TAG,"analyze");
              if(isPush && nativePush != null){
                  int format = image.getFormat();
                  Log.i("NativePush","the format:"+format);
                  if(format != ImageFormat.YUV_420_888){

                      return;
                  }
                  ImageProxy.PlaneProxy[] planes =  image.getPlanes();
                  //y
                 ByteBuffer yByteBuffer =  planes[0].getBuffer();
                 ByteBuffer uvByteBuffer = planes[2].getBuffer();
                 int ySize = yByteBuffer.remaining();
                 int uvSize = uvByteBuffer.remaining();
                 byte[] data = new byte[ySize+uvSize];
                 yByteBuffer.get(data,0,ySize);
                 uvByteBuffer.get(data,ySize,uvSize);
                    nativePush.native_pushVideo(data);
              }
            }
        });

        cameraProvider.bindToLifecycle(this, cameraSelector, imageAnalysis, preview);
        Camera camera = cameraProvider.bindToLifecycle(this,cameraSelector,preview);

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    private boolean isPush = false;
    public void onBtnPush(View view) {
        isPush = true;
        if(nativePush != null){
            nativePush.startPush("rtmp://sendtc3a.douyu.com/live/9981301raTDrkBPJ?wsSecret=c117137ef445c0ab6df5b515a7f7dec8&wsTime=60f2e76c&wsSeek=off&wm=0&tw=0&roirecognition=0&record=flv&origin=tct");
        }
    }

    public void onBtnSwitchCamera(View view) {
        bindPreview();
    }

    public void onBtnStop(View view) {
        if(nativePush != null){
            nativePush.stopPush();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(nativePush != null){
            nativePush.release();
        }
    }
}