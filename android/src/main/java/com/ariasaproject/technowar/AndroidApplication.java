package com.ariasaproject.technowar;

import android.compat.annotation.UnsupportedAppUsage;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Bundle;
import android.os.Looper;
import android.os.MessageQueue;
import android.util.AttributeSet;
import android.view.InputQueue;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

import java.io.File;

public class NativeActivity extends Activity implements SurfaceHolder.Callback, InputQueue.Callback, OnGlobalLayoutListener {

    static {
        try {
            System.loadLibrary("ext");
        } catch (Exception e) {
            System.out.println("failed to load library : ext");
            System.exit(0);
        }
    }
    private static final String KEY_NATIVE_DATA_STATE = "android:native_state"
    
    private InputMethodManager mIMM;
    private View mRootView;
    private InputQueue mCurInputQueue;
    private SurfaceHolder mCurSurfaceHolder;
    protected long nativeHandleId;
    
    final int[] mLocation = new int[2];
    int mLastContentX;
    int mLastContentY;
    int mLastContentWidth;
    int mLastContentHeight;
    private boolean mDispatchingUnhandledKey;
    private boolean mDestroyed;
    
    private native long onCreateNative(byte[] dat, int len);
    private native void onStartNative(long nativeHandle);
    private native void onResumeNative(long nativeHandle);
    private native byte[] onSaveInstanceStateNative(long nativeHandle);
    private native void onPauseNative(long nativeHandle, boolean finish);
    private native void onStopNative(long nativeHandle);
    private native void onConfigurationChangedNative(long nativeHandle);
    private native void onLowMemoryNative(long nativeHandle);
    private native void onWindowFocusChangedNative(long nativeHandle, boolean focused);
    private native void onSurfaceSetNative(long nativeHandle, Surface surface);
    private native void onInputQueueSetNative(long nativeHandle, long inputQueue);
    private native void onSurfaceChangedNative(long nativeHandle, int format, int width, int height);
    private native void onContentRectChangedNative(long nativeHandle, int x, int y, int w, int h);
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mIMM = getSystemService(InputMethodManager.class);
        getWindow().takeSurface(this);
        getWindow().takeInputQueue(this);
        getWindow().setFormat(PixelFormat.RGB_565);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_UNSPECIFIED | WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        mRootView = findViewById(R.id.root);
        mRootView.requestFocus();
        mRootView.getViewTreeObserver().addOnGlobalLayoutListener(this);
        byte[] data_state = savedInstanceState != null ? savedInstanceState.getByteArray(KEY_NATIVE_DATA_STATE) : null;
        nativeHandleId = onCreateNative(data_state,data_state.length);
    }
    @Override
    protected void onDestroy() {
        mDestroyed = true;
        if (mCurSurfaceHolder != null) {
            onSurfaceSetNative(nativeHandleId, null);
            mCurSurfaceHolder = null;
        }
        if (mCurInputQueue != null) {
            onInputQueueSetNative(nativeHandleId, null);
            mCurInputQueue = null;
        }
        super.onDestroy();
    }
    @Override
    protected void onPause() {
        onPauseNative(nativeHandleId, isFinishing());
        super.onPause();
    }
    @Override
    protected void onResume() {
        super.onResume();
        onResumeNative(nativeHandleId);
    }
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        byte[] state = onSaveInstanceStateNative(nativeHandleId);
        if (state != null) {
            outState.putByteArray(KEY_NATIVE_DATA_STATE, state);
        }
    }
    @Override
    protected void onStart() {
        super.onStart();
        onStartNative(nativeHandleId);
    }
    @Override
    protected void onStop() {
        super.onStop();
        onStopNative(nativeHandleId);
    }
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (!mDestroyed) {
            onConfigurationChangedNative(nativeHandleId);
        }
    }
    @Override
    public void onLowMemory() {
        super.onLowMemory();
        if (!mDestroyed) {
            onLowMemoryNative(nativeHandleId);
        }
    }
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (!mDestroyed) {
            onWindowFocusChangedNative(nativeHandleId, hasFocus);
        }
    }
    
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (!mDestroyed) {
            mCurSurfaceHolder = holder;
            onSurfaceSetNative(nativeHandleId, holder.getSurface());
        }
    }
    
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (!mDestroyed) {
            onSurfaceChangedNative(nativeHandleId, format, width, height);
        }
    }
    
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mCurSurfaceHolder = null;
        if (!mDestroyed) {
            onSurfaceSetNative(nativeHandleId, null);
        }
    }
    
    @Override
    public void onInputQueueCreated(InputQueue queue) {
        if (!mDestroyed) {
            mCurInputQueue = queue;
            onInputQueueSetNative(nativeHandleId, queue.getNativePtr());
        }
    }
    @Override
    public void onInputQueueDestroyed(InputQueue queue) {
        if (!mDestroyed) {
            onInputQueueSetNative(nativeHandleId, null);
            mCurInputQueue = null;
        }
    }
    
    public void onGlobalLayout() {
        mRootView.getLocationInWindow(mLocation);
        int w = mRootView.getWidth();
        int h = mRootView.getHeight();
        if (mLocation[0] != mLastContentX || mLocation[1] != mLastContentY || w != mLastContentWidth || h != mLastContentHeight) {
            mLastContentX = mLocation[0];
            mLastContentY = mLocation[1];
            mLastContentWidth = w;
            mLastContentHeight = h;
            if (!mDestroyed) {
                onContentRectChangedNative(nativeHandleId, mLastContentX, mLastContentY, mLastContentWidth, mLastContentHeight);
            }
        }
    }
}