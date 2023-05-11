package com.ariasaproject.technowar;

import android.os.Build;
import android.app.NativeActivity;
import android.os.Bundle;
import android.view.View;
import android.view.DisplayCutout;
import android.view.WindowInsets;

public class MainActivity extends NativeActivity {
    static native void setInsets(int left, int top, int right, int bottom);
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Build.VERSION.SDK_INT >= 28) {
            getWindow().getDecorView().setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
                    private DisplayCutout mDisplayCutout;
                    @Override
                    public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                        mDisplayCutout = insets.getDisplayCutout();
                        setInsets(
                          mDisplayCutout.getSafeInsetLeft(),
                          mDisplayCutout.getSafeInsetTop(),
                          mDisplayCutout.getSafeInsetRight(),
                          mDisplayCutout.getSafeInsetBottom());
                        return insets;
                    }
                });
        }
    }
}
