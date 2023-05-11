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
        //get insets
    		if (Build.VERSION.SDK_INT >= 28) {
            final View decorView = getWindow().getDecorView();
            decorView.setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
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
    @Override
    protected void onStart() {
    		if (Build.VERSION.SDK_INT >= 28) {
            final View decorView = getWindow().getDecorView();
            final DisplayCutout displayCutout = decorView.getRootWindowInsets().getDisplayCutout();
            if (displayCutout != null) {
                setInsets(
                  displayCutout.getSafeInsetLeft(),
                  displayCutout.getSafeInsetTop(),
                  displayCutout.getSafeInsetRight(),
                  displayCutout.getSafeInsetBottom()
                );
            }
    		}
    		super.onStart();
    }
    
}
