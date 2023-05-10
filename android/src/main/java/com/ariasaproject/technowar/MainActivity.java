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
    		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            final View decorView = getWindow().getDecorView();
            try {
                DisplayCutout displayCutout = decorView.getRootWindowInsets().getDisplayCutout();
                if (displayCutout != null) {
                    setInsets(
                      displayCutout.getSafeInsetLeft(),
                      displayCutout.getSafeInsetTop(),
                      displayCutout.getSafeInsetRight(),
                      displayCutout.getSafeInsetBottom()
                    );
                }
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
            } catch (UnsupportedOperationException e) {
                //Gdx.app.log("AndroidGraphics", "Unable to get safe area insets");
            }
    		}
    }
    
}
