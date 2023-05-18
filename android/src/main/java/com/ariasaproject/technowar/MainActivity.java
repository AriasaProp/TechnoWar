package com.ariasaproject.technowar;

import java.lang.Math;

import android.os.Build;
import android.app.NativeActivity;
import android.os.Bundle;
import android.view.View;
import android.view.WindowInsets;
import android.view.SurfaceHolder;
import android.widget.Toast;

public class MainActivity extends NativeActivity {
    static {
        System.loadLibrary("ext");
    }
    static native void setInsets(int left, int top, int right, int bottom);
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().getDecorView().setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                int insetsL = 0;
                int insetsT = 0;
                int insetsR = 0;
                int insetsB = 0;
                try {
                    if ((insets != null) && (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)) {
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                            int tl = insets.getRoundedCorner(android.view.RoundedCorner.POSITION_TOP_LEFT).getRadius();
                            int bl = insets.getRoundedCorner(android.view.RoundedCorner.POSITION_BOTTOM_LEFT).getRadius();
                            insetsL = Math.max(tl, bl)/3;
                            int tr = insets.getRoundedCorner(android.view.RoundedCorner.POSITION_TOP_RIGHT).getRadius();
                            insetsT = Math.max(tl, tr)/3;
                            int br = insets.getRoundedCorner(android.view.RoundedCorner.POSITION_BOTTOM_RIGHT).getRadius();
                            insetsR = Math.max(tr, br)/3;
                            insetsB = Math.max(bl, br)/3;
                        }
                        final android.view.DisplayCutout displayCutout = insets.getDisplayCutout();
                        if (displayCutout != null) {
                            insetsL = Math.max(insetsL, displayCutout.getSafeInsetLeft());
                            insetsT = Math.max(insetsT, displayCutout.getSafeInsetTop());
                            insetsR = Math.max(insetsR, displayCutout.getSafeInsetRight());
                            insetsB = Math.max(insetsB, displayCutout.getSafeInsetBottom());
                        }
                    }
                } catch (Exception e) {
                    //unsuported ?
                    // cause floating window
                }
                setInsets(insetsL, insetsT, insetsR, insetsB);
                callToast("request Insets!");
                return insets;
            }
        });
        callToast("Create done!");
    }
    @Override
    protected void onStart() {
        callToast("On Start");
        super.onStart();
    }
    @Override
    protected void onResume() {
        callToast("On Resume");
        super.onResume();
    }
    @Override
    protected void onPause() {
        callToast("On Pause");
        super.onPause();
    }
    @Override
    protected void onStop() {
        callToast("On Stop");
        super.onStop();
    }
    @Override
    protected void onDestroy() {
        callToast("On Destroy");
        super.onDestroy();
    }
    
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        callToast("Surface Created");
        super.surfaceCreated(holder);
    }
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        callToast("on Surface Changed!");
        getWindow().getDecorView().requestApplyInsets();
        super.surfaceChanged(holder, format, width, height);
    }
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        callToast("Surface Destroyed");
        super.surfaceDestroyed(holder);
    }
    
    void callToast (CharSequence msg) {
        Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
    }
}
