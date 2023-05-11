package com.ariasaproject.technowar;

import android.os.Build;
import android.app.NativeActivity;
import android.os.Bundle;
import android.view.View;
import android.view.DisplayCutout;
import android.view.WindowInsets;
import android.view.SurfaceHolder;

public class MainActivity extends NativeActivity {
    static {
        System.loadLibrary("ext");
    }
    static native void setInsets(int left, int top, int right, int bottom);
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        updateSafeArea();
        super.surfaceCreated(holder);
    }
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        updateSafeArea();
        super.surfaceChanged(holder, format, width, height);
    }
    void updateSafeArea() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            DisplayCutout displayCutout = getWindow().getDecorView().getRootWindowInsets().getDisplayCutout();
            if (displayCutout != null) {
                setInsets( 50, 5, 50, 5
                  /*
                  displayCutout.getSafeInsetLeft(),
                  displayCutout.getSafeInsetTop(),
                  displayCutout.getSafeInsetRight(),
                  displayCutout.getSafeInsetBottom()
                  */
                );
            }
        }
    }
}
