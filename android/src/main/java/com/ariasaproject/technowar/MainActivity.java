package com.ariasaproject.technowar;

import java.lang.Math;

import android.os.Build;
import android.app.NativeActivity;
import android.os.Bundle;
import android.view.View;
import android.view.DisplayCutout;
import android.view.WindowInsets;
import android.view.WindowInsets.RoundedCorner;
import android.view.SurfaceHolder;

public class MainActivity extends NativeActivity {
    static {
        System.loadLibrary("ext");
    }
    static native void setInsets(int left, int top, int right, int bottom);
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().getDecorView().setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            private DisplayCutout displayCutout;
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                int insetsL, insetsT, insetsR, insetsB;
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                        int tl = insets.getRoundedCorner(RoundedCorner.POSITION_TOP_LEFT).getRadius();
                        int bl = insets.getRoundedCorner(RoundedCorner.POSITION_BOTTOM_LEFT).getRadius();
                        int tr = insets.getRoundedCorner(RoundedCorner.POSITION_TOP_RIGHT).getRadius();
                        int br = insets.getRoundedCorner(RoundedCorner.POSITION_BOTTOM_RIGHT).getRadius();
                        insetsL = Math.max(tl, bl)/3;
                        insetsT = Math.max(tl, tr)/3;
                        insetsR = Math.max(tr, br)/3;
                        insetsB = Math.max(bl, br)/3;
                    }
                    displayCutout = insets.getDisplayCutout();
                    if (displayCutout != null) {
                        insetsL = Math.max(insetsL, displayCutout.getSafeInsetLeft());
                        insetsT = Math.max(insetsT, displayCutout.getSafeInsetTop());
                        insetsR = Math.max(insetsR, displayCutout.getSafeInsetRight());
                        insetsB = Math.max(insetsB, displayCutout.getSafeInsetBottom());
                    }
                }
                setInsets(insetsL, insetsT, insetsR, insetsB);
                return insets;
            }
        });
    }
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        getWindow().getDecorView().requestApplyInsets();
        super.surfaceChanged(holder, format, width, height);
    }
    @Override
    public void onGlobalLayout() {
        getWindow().getDecorView().requestApplyInsets();
        super.onGlobalLayout();
    }
}
