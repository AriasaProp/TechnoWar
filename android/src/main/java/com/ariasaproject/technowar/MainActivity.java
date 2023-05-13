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
        getWindow().getDecorView().setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            private DisplayCutout displayCutout;
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                    displayCutout = insets.getDisplayCutout();
                    if (displayCutout != null) {
                        setInsets(
                          displayCutout.getSafeInsetLeft(),
                          displayCutout.getSafeInsetTop(),
                          displayCutout.getSafeInsetRight(),
                          displayCutout.getSafeInsetBottom()
                        );
                    }
                }
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
