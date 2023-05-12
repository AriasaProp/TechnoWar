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
        final View decorView = getWindow().getDecorView();
        decorView.setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                    DisplayCutout displayCutout = insets.getDisplayCutout();
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
        //decorView.requestApplyInsets();
    }
}
