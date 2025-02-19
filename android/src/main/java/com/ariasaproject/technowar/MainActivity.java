package com.ariasaproject.technowar;

import android.app.NativeActivity;
import android.os.Build;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowInsets;
import android.widget.Toast;

public class MainActivity extends NativeActivity implements View.OnApplyWindowInsetsListener {
    static {
        System.loadLibrary("ext");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().getDecorView().setOnApplyWindowInsetsListener(this);
    }

    @Override
    public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
        int insetsL = 0, insetsT = 0, insetsR = 0, insetsB = 0;
        try {
            if ((Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) && (insets != null)) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    int tl =
                            insets.getRoundedCorner(android.view.RoundedCorner.POSITION_TOP_LEFT)
                                    .getRadius();
                    int bl =
                            insets.getRoundedCorner(android.view.RoundedCorner.POSITION_BOTTOM_LEFT)
                                    .getRadius();
                    insetsL = Math.max(tl, bl) / 3;
                    int tr =
                            insets.getRoundedCorner(android.view.RoundedCorner.POSITION_TOP_RIGHT)
                                    .getRadius();
                    insetsT = Math.max(tl, tr) / 3;
                    int br =
                            insets.getRoundedCorner(
                                            android.view.RoundedCorner.POSITION_BOTTOM_RIGHT)
                                    .getRadius();
                    insetsR = Math.max(tr, br) / 3;
                    insetsB = Math.max(bl, br) / 3;
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
            // unsuported ?
            // cause floating window
        }
        insetNative(insetsL, insetsT, insetsR, insetsB);
        return insets;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        getWindow().getDecorView().requestApplyInsets();
        super.surfaceCreated(holder);
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

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
    
    @Keep
    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    // c implementation
    native void insetNative(int left, int top, int right, int bottom);
}
