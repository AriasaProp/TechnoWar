package com.ariasaproject.technowar;

import android.os.Build;
import android.app.NativeActivity;
import android.os.Bundle;
import android.view.View;
import android.view.DisplayCutout;
import android.view.WindowInsets;

public class MainActivity extends NativeActivity implements  View.OnApplyWindowInsetsListener {
    static native void setInsets(int left, int top, int right, int bottom);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().getDecorView().setOnApplyWindowInsetsListener(this);
    }
    private DisplayCutout mDisplayCutout;
    @Override
    public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
        if (Build.VERSION.SDK_INT < 28) return;
        mDisplayCutout = insets.getDisplayCutout();
        setInsets(
          mDisplayCutout.getSafeInsetLeft(),
          mDisplayCutout.getSafeInsetTop(),
          mDisplayCutout.getSafeInsetRight(),
          mDisplayCutout.getSafeInsetBottom());
        return insets;
    }
    
    
}
