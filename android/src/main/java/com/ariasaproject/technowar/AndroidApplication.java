package com.ariasaproject.technowar;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.content.res.AssetFileDescriptor;
import android.content.res.Configuration;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.GLES30;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnSystemUiVisibilityChangeListener;
import android.widget.Toast;

import com.ariasaproject.technowar.ApplicationListener;
import com.ariasaproject.technowar.Application;

import java.io.FileDescriptor;
import java.io.IOException;
import java.lang.Exception;
import java.lang.Throwable;
import java.io.FileOutputStream;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

//AndroidApplication include Graphics, Audio and Application
public class AndroidApplication extends Activity implements Application, Runnable, Callback {
    public static final String TAG = "MainActivity";
    
    static {
	    	try {
	    			System.loadLibrary("ext");
	    	} catch (Exception e) {
	    			Log.e(TAG, "failed load library!.")
	    	}
    }
    
    final int uiHide = 5382;//hide all system ui as possible
    int mayorV, minorV;
    volatile boolean resume = false, pause = false, destroy = false, resize = false, rendered = false, hasFocus = true,
            mExited = false;
    long frameStart = System.currentTimeMillis(), lastFrameTime = System.currentTimeMillis();
    int frames, fps, width = 0, height = 0;
    float deltaTime = 0;
    Thread mainTGFThread;
    // graphics params
    private SurfaceHolder holder = null;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(this);
        super.onCreate(savedInstanceState);
        final View d = getWindow().getDecorView();
        d.setSystemUiVisibility(uiHide);
        d.setOnSystemUiVisibilityChangeListener(new OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int ui) {
                if (ui != uiHide)
                    d.setSystemUiVisibility(uiHide);
            }
        });
        setContentView(R.layout.main);
        SurfaceView view = findViewById(R.id.root);
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
        this.mayorV = (short) (configurationInfo.reqGlEsVersion >> 16);
        this.minorV = (short) (configurationInfo.reqGlEsVersion & 0x0000ffff);
        // for graphics loop
        mainTGFThread = new Thread(this, "GLThread");
        mainTGFThread.start();
        view.getHolder().addCallback(this);
    }

    @Override
    protected void onStart() {
    		super.onStart();
    }
    @Override
    protected synchronized void onResume() {
        super.onResume();
        resume = true;
        notifyAll();
    }

    @Override
    public synchronized void restart() {
        runOnUiThread(new Runnable() {
          @Override
          public void run() {
            finish();
            try {
                while(!mExited)
                  wait();
            } catch(Throwable ignore) {}
            startActivity(getIntent());
          }
        });
    }

    @Override
    public synchronized void exit() {
        runOnUiThread(new Runnable() {
          @Override
          public void run() {
            finish();
          }
        });
    }

    @Override
    public synchronized void onWindowFocusChanged(final boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
    }

    @Override
    protected synchronized void onPause() {
        // graphics
        pause = true;
        rendered = true;
        notifyAll();
        while (!mExited && rendered) {
            try {
                wait();
            } catch (InterruptedException ex) {
                Thread.currentThread().interrupt();
            }
        }
        if (isFinishing()) {
            destroy = true;
            notifyAll();
            while (!mExited) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    Thread.currentThread().interrupt();
                }
            }
        }
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }
    
    @Override
    protected synchronized void onDestroy() {
        //net.destroy();
        super.onDestroy();
    }


    @Override
    public void onConfigurationChanged(Configuration config) {
        super.onConfigurationChanged(config);
        //input.setKeyboardAvailable(config.hardKeyboardHidden == Configuration.HARDKEYBOARDHIDDEN_NO);
    }

    @Override
    public void debug(String tag, String message) {
        Log.d(tag, message);
    }

    @Override
    public void debug(String tag, String message, Throwable exception) {
        Log.d(tag, message, exception);
    }

    @Override
    public void log(String tag, String message) {
        Log.i(tag, message);
    }

    @Override
    public void log(String tag, String message, Throwable exception) {
        Log.i(tag, message, exception);
    }

    @Override
    public void error(String tag, String message) {
        Log.e(tag, message);
    }

    @Override
    public void error(String tag, String message, Throwable exception) {
        Log.e(tag, message, exception);

    }

    @Override
    public synchronized void surfaceCreated(SurfaceHolder holder) {
        // fall thru surfaceChanged
        this.holder = holder;
        notifyAll();
    }

    @Override
    public synchronized void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        if (this.holder == null) this.holder = holder;
        width = w;
        height = h;
        resize = true;
    }

    @Override
    public synchronized void surfaceDestroyed(SurfaceHolder holder) {
        this.holder = null;
        notifyAll();
    }
    
		private native void create();
		private native void resume();
		private native void resize(int w, int h);
		private native void render(float d);
		private native void pause();
		private native void destroy();

    // main loop
    @Override
    public void run() {
        EGLDisplay mEglDisplay = null;
        EGLSurface mEglSurface = null;
        EGLConfig mEglConfig = null;
        EGLContext mEglContext = null;
        try {
				    final int[] configsEGL = new int[]{
				  			EGL14.EGL_COLOR_BUFFER_TYPE, EGL14.EGL_RGB_BUFFER, EGL14.EGL_NONE, //EGLConfig offset 0
				  			EGL14.EGL_CONTEXT_CLIENT_VERSION, mayorV, EGL14.EGL_NONE, //EGLContext offset 3
				  			EGL14.EGL_NONE, EGL14.EGL_NONE, //NULL EGL Value offset 6
				  			0
				    };
            byte eglDestroyRequest = 0;// to destroy egl surface, egl contex, egl display, ?....
            boolean wantRender = false, newContext = true, // indicator
                    created = false, lrunning = true, lresume = false, lpause = false;// on running state
            while (!destroy) {
                synchronized (this) {
                    // render notify
                    if (wantRender) {
                        rendered = false;
                        wantRender = false;
                        notifyAll();
                    }
                    if (rendered)
                        wantRender = true;
                    // egl destroy request
                    if (mEglSurface != null && (eglDestroyRequest > 0 || (holder == null))) {
                        EGL14.eglMakeCurrent(mEglDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT);
                        if (!EGL14.eglDestroySurface(mEglDisplay, mEglSurface))
                            throw new RuntimeException("eglDestroySurface failed: " + Integer.toHexString(EGL14.eglGetError()));
                        mEglSurface = null;
                        if (mEglContext != null && (eglDestroyRequest > 1)) {
                            if (!EGL14.eglDestroyContext(mEglDisplay, mEglContext))
                                throw new RuntimeException("eglDestroyContext failed: " + Integer.toHexString(EGL14.eglGetError()));
                            mEglContext = null;
                            newContext = true;
                            if (mEglDisplay != null && (eglDestroyRequest > 3)) {
                                EGL14.eglTerminate(mEglDisplay);
                                mEglDisplay = null;
                            }
                        }
                        eglDestroyRequest = 0;
                    }
                    // end destroy request
                    if (lpause)
                        lrunning = false;
                    lpause = pause;
                    if (pause)
                        pause = false;
                    lresume = resume;
                    if (resume) {
                        resume = false;
                        lrunning = true;
                    }
                    // Ready to draw?
                    if (!lrunning || (holder == null)) {
                        wait();
                        continue;
                    }
                }

                if (mEglDisplay == null) {
                    final int[] temp = new int[2]; // for chaching value output

                    mEglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
                    if (mEglDisplay == EGL14.EGL_NO_DISPLAY || mEglDisplay == null) {
                        mEglDisplay = null;
                        throw new RuntimeException("eglGetDisplay failed " + Integer.toHexString(EGL14.eglGetError()));
                    }
                    if (EGL14.eglInitialize(mEglDisplay, temp, 0, temp, 1))
                        log(TAG, "version EGL " + temp[0] + "." + temp[1]);
                    else
                        throw new RuntimeException("eglInitialize failed " + Integer.toHexString(EGL14.eglGetError()));

                    if (mEglConfig == null) {
                        // choose best config
                        EGL14.eglChooseConfig(mEglDisplay, configsEGL, 0, null, 0, 0, temp, 0);
                        if (temp[0] <= 0)
                            throw new IllegalArgumentException("No configs match with configSpec");
                        EGLConfig[] configs = new EGLConfig[temp[0]];
                        EGL14.eglChooseConfig(mEglDisplay, configsEGL, 0, configs, 0, configs.length, temp, 0);
                        int lastSc = -1, curSc;
                        mEglConfig = configs[0];
                        for (EGLConfig config : configs) {
                            temp[0] = -1;
                            // alpha should 0
                            // choose higher depth, stencil, color buffer(rgba)
                            curSc = -1;
                            for (int attr : new int[]{EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_BUFFER_SIZE, EGL14.EGL_ALPHA_SIZE, EGL14.EGL_DEPTH_SIZE, EGL14.EGL_STENCIL_SIZE}) {
                                if (EGL14.eglGetConfigAttrib(mEglDisplay, config, attr, temp, 0)) {
                                    if (attr == EGL14.EGL_ALPHA_SIZE)
                                        temp[0] *= -1;
                                    curSc += temp[0];
                                } else {
                                    int error;
                                    while ((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS)
                                        error(TAG, String.format("EglConfigAttribute : EGL error: 0x%x", error));
                                }
                            }
                            if (curSc > lastSc) {
                                lastSc = curSc;
                                mEglConfig = config;
                            }
                        }
                    }
                }
                if (newContext || mEglSurface == null) {
                    if (newContext) {
                        mEglContext = EGL14.eglCreateContext(mEglDisplay, mEglConfig, EGL14.EGL_NO_CONTEXT, configsEGL, 3);
                        if (mEglContext == null || mEglContext == EGL14.EGL_NO_CONTEXT) {
                            mEglContext = null;
                            throw new RuntimeException("createContext failed: " + Integer.toHexString(EGL14.eglGetError()));
                        }
                    }
                    mEglSurface = EGL14.eglCreateWindowSurface(mEglDisplay, mEglConfig, holder, configsEGL, 6);
                    if (mEglSurface == null || mEglSurface == EGL14.EGL_NO_SURFACE) {
                        mEglSurface = null;
                        throw new RuntimeException("Create EGL Surface failed: " + Integer.toHexString(EGL14.eglGetError()));
                    }
                    if (!EGL14.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
                        throw new RuntimeException("Make EGL failed: " + Integer.toHexString(EGL14.eglGetError()));

                    if (newContext) {
                        if (!created) {
                            create();
                            created = true;
                    		}
				                synchronized (this) {
						                if (resize) {
                        				resize(width, height);
                        				resize = false;
						                }
				                }
                        lastFrameTime = System.currentTimeMillis();
                        newContext = false;
                    }
                }
                synchronized (this) {
		                if (resize) {
		                    EGL14.eglMakeCurrent(mEglDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT);
		                    EGL14.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext);
		                    resize(width, height);
		                    resize = false;
		                }
                }
                long time = System.currentTimeMillis();
                if (lresume) {
                    resume();
                    time = frameStart = lastFrameTime = 0;
                }
                if (time - frameStart > 1000l) {
                    fps = frames;
                    frames = 0;
                    frameStart = time;
                }
                deltaTime = (time - lastFrameTime) / 1000f;
                lastFrameTime = time;
                /*
                GLES30.glClearColor(1, 1, 0, 1);
                GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT|GLES30.GL_DEPTH_BUFFER_BIT|GLES30.GL_STENCIL_BUFFER_BIT);
                */
                render(deltaTime);
                
                if (lpause) {
                    pause();
                    boolean limitGles = GLES30.glGetString(GLES30.GL_RENDERER).startsWith("Adreno");
                    eglDestroyRequest |= (limitGles ? 2 : 1);
                }
                if (!EGL14.eglSwapBuffers(mEglDisplay, mEglSurface)) {
                    int error = EGL14.eglGetError();
                    switch (error) {
                        case EGL14.EGL_BAD_DISPLAY:
                            eglDestroyRequest |= 4;
                            break;
                        case 0x300E:
                        case EGL14.EGL_BAD_CONTEXT:
                            eglDestroyRequest |= 2;
                            break;
                        case EGL14.EGL_BAD_SURFACE:
                            eglDestroyRequest |= 1;
                            break;
                        case EGL14.EGL_BAD_NATIVE_WINDOW:
                            error(TAG, "eglSwapBuffers returned EGL_BAD_NATIVE_WINDOW. tid=" + Thread.currentThread().getId());
                            break;
                        default:
                            error(TAG, "eglSwapBuffers failed: " + Integer.toHexString(error));
                    }
                }
                frames++;
            }
        } catch (Throwable e) {
            // fall thru and exit normally
            error(TAG, "error", e);
        }
        // dispose all resources
        destroy();
        if (mEglSurface != null) {
            EGL14.eglMakeCurrent(mEglDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT);
            EGL14.eglDestroySurface(mEglDisplay, mEglSurface);
            mEglSurface = null;
        }
        if (mEglContext != null) {
            EGL14.eglDestroyContext(mEglDisplay, mEglContext);
            mEglContext = null;
        }
        if (mEglDisplay != null) {
            EGL14.eglTerminate(mEglDisplay);
            mEglDisplay = null;
        }
        // end thread
        synchronized (this) {
            mExited = true;
            notifyAll();
        }
        if (!destroy) finish();
    }
}
