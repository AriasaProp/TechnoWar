package com.ariasaproject.technowar;

import android.app.NativeActivity;
import android.os.Bundle;
import android.view.View;
import android.view.DisplayCutout;
import android.view.WindowInsets;
import android.view.WindowInsets.Type;
import android.view.WindowInsetsController;
import android.view.WindowInsetsController.Appearance;


public class MainActivity extends NativeActivity {
    static native void setInsets(int left, int top, int right, int bottom);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //get insets
        final View decorView = getWindow().getDecorView();
        decorView.setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            private DisplayCutout mDisplayCutout;
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                mDisplayCutout = insets.getDisplayCutout();
                setInsets(
                  displayCutout.getSafeInsetLeft(),
                  displayCutout.getSafeInsetTop(),
                  displayCutout.getSafeInsetRight(),
                  displayCutout.getSafeInsetBottom());
                return insets;
            }
        });
        
        super.onCreate(savedInstanceState);
    }
    
}
