#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <cerrno>
#include <cassert>


#include <EGL/egl.h>
#include <GLES3/gl3.h>


#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};
enum {
	TERM_EGL_SURFACE = 1,
	TERM_EGL_CONTEXT = 2,
	TERM_EGL_DISPLAY = 4
};
struct engine {
    bool animating;
    unsigned int eglTermReq;
    int32_t width;
    int32_t height;
    android_app* app;
    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    saved_state state;
};

static void engine_draw_frame(engine* eng) {
    if (!eng->display) {
        return;
    }
    glClearColor(((float)eng->state.x)/eng->width, eng->state.angle, ((float)eng->state.y)/eng->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}
static int32_t poll_input(android_app* app, AInputEvent* event) {
    engine* eng = (engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        eng->animating = true;
        eng->state.x = AMotionEvent_getX(event, 0);
        eng->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

static void poll_cmd(android_app* app, int32_t cmd) {
    engine* eng = (engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            assert(app->window);
            break;
        case APP_CMD_GAINED_FOCUS:
            if (eng->accelerometerSensor) {
                ASensorEventQueue_enableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
                ASensorEventQueue_setEventRate(eng->sensorEventQueue,eng->accelerometerSensor,(1000L/60)*1000);
            }
            break;
            
		    case APP_CMD_WINDOW_RESIZED:
			    	eng->resize = true;
			    	break;
        case APP_CMD_SAVE_STATE:
            app->savedState = malloc(sizeof(saved_state));
            *((saved_state*)app->savedState) = eng->state;
            app->savedStateSize = sizeof(saved_state);
            break;
        case APP_CMD_LOST_FOCUS:
            if (eng->accelerometerSensor) {
                ASensorEventQueue_disableSensor(eng->sensorEventQueue,eng->accelerometerSensor);
            }
            eng->animating = false;
            break;
        case APP_CMD_TERM_WINDOW:
        		eng->eglTermReq |= TERM_EGL_SURFACE;
            break;
        case APP_CMD_DESTROY:
        		eng->eglTermReq |= TERM_EGL_DISPLAY;
            break;
        default:
            break;
    }
}
void android_main(android_app* app) {
    engine eng{};
    memset(&eng, 0, sizeof(eng));
    app->userData = &eng;
    app->onAppCmd = poll_cmd;
    app->onInputEvent = poll_input;
    eng.app = app;
    eng.sensorManager = ASensorManager_getInstance();
    eng.accelerometerSensor = ASensorManager_getDefaultSensor(eng.sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    eng.sensorEventQueue = ASensorManager_createEventQueue(eng.sensorManager,app->looper, LOOPER_ID_USER,nullptr, nullptr);
    if (app->savedState) {
        eng.state = *(saved_state*)app->savedState;
    }
    for (;;) {
        int ident;
        int events;
        android_poll_source* source;
        while ((ident=ALooper_pollAll(eng.animating ? 0 : -1, nullptr, &events,(void**)&source)) >= 0) {
            if (source) {
                source->process(app, source);
            }
            if (ident == LOOPER_ID_USER) {
                if (eng.accelerometerSensor) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(eng.sensorEventQueue,&event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",event.acceleration.x, event.acceleration.y,event.acceleration.z);
                    }
                }
            }
				    //destroy egl req
				    if (eng.eglTermReq) {
				    	if (eng.display) {
				    		eglMakeCurrent(eng.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
				    		if (eng.context && (eng.eglTermReq & (TERM_EGL_CONTEXT|TERM_EGL_DISPLAY))) {
						    	eglDestroyContext(eng.display, eng.context);
						    	eng.context = EGL_NO_CONTEXT;
						    }
						    if (eng.surface && (eng.eglTermReq & (TERM_EGL_SURFACE|TERM_EGL_DISPLAY))) {
						      eglDestroySurface(eng.display, eng.surface);
						    	eng.surface = EGL_NO_SURFACE;
						    }
						    if (eng.eglTermReq & TERM_EGL_DISPLAY) {
					    		eglTerminate(eng.display);
						    	eng.display = EGL_NO_DISPLAY;
						    }
				    	}
				    	eng.eglTermReq = 0;
				    }
            if (app->destroyRequested) {
                return;
            }
        }
        //init egl
        if (!app->window) continue;
		    if (!eng.display || !eng.context || !eng.surface) {
		    	if (!eng.display) {
			    	eng.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
			    	eglInitialize(eng.display, nullptr, nullptr);
			    	eng.eConfig = nullptr;
		    	}
		    	if (!eng.eConfig) {
					  EGLint temp;
					  const EGLint configAttr[] = {
					    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
					    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
					    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
					    EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
					    EGL_ALPHA_SIZE, 0,
					    EGL_NONE
					  };
					  eglChooseConfig(eng.display, configAttr, nullptr,0, &temp);
					  assert(temp);
					  EGLConfig *configs = (EGLConfig*) alloca(temp*sizeof(EGLConfig));
					  assert(configs);
					  eglChooseConfig(eng.display, configAttr, configs, temp, &temp);
					  assert(temp);
					  eng.eConfig = configs[0];
					  for (unsigned int i = 0, j = temp, k = 0, l; i < j; i++) {
					    EGLConfig& cfg = configs[i];
					    eglGetConfigAttrib(eng.display, cfg, EGL_BUFFER_SIZE, &temp);
					    l = temp;
					    eglGetConfigAttrib(eng.display, cfg, EGL_DEPTH_SIZE, &temp);
					    l += temp;
					    eglGetConfigAttrib(eng.display, cfg, EGL_STENCIL_SIZE, &temp);
					    l += temp;
					    if (l > k) {
					      k = l;
					      eng.eConfig = cfg;
					    }
					  }
		    	}
		    	if (!eng.context) {
		    		const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
		    		eng.context = eglCreateContext(eng.display, eng.eConfig, nullptr, ctxAttr);
		    	}
		    	if (!eng.surface) {
						eng.surface = eglCreateWindowSurface(eng.display, eng.eConfig, app->window, nullptr);
		    	}
		    	eglMakeCurrent(eng.display, eng.surface, eng.surface, eng.context);
		    	eglQuerySurface(eng.display, eng.surface, EGL_WIDTH, &eng.width);
		    	eglQuerySurface(eng.display, eng.surface, EGL_HEIGHT, &eng.height);
		    	
		    	eng.resize = false;
		    }
		    if (eng.resize) {
		    	eglMakeCurrent(eng.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		    	eglMakeCurrent(eng.display, eng.surface, eng.surface, eng.context);
		    	eglQuerySurface(eng.display, eng.surface, EGL_WIDTH, &eng.width);
		    	eglQuerySurface(eng.display, eng.surface, EGL_HEIGHT, &eng.height);
		    	eng.resize = false;
		    }
        if (!eng.animating) continue;
        eng.state.angle += .01f;
        if (eng.state.angle > 1) {
            eng.state.angle = 0;
        }
        engine_draw_frame(&eng);
    		if (!eglSwapBuffers(eng.display, eng.surface)) {
	    			switch (eglGetError()) {
				    		case EGL_BAD_SURFACE:
				    		case EGL_BAD_NATIVE_WINDOW:
				    		case EGL_BAD_CURRENT_SURFACE:
					    			eng.eglTermReq |= TERM_EGL_SURFACE;
					    			break;
				    		case EGL_BAD_CONTEXT:
				    		case EGL_CONTEXT_LOST:
					    			eng.eglTermReq |= TERM_EGL_CONTEXT;
					    			break;
				    		case EGL_NOT_INITIALIZED:
				    		case EGL_BAD_DISPLAY:
					    			eng.eglTermReq |= TERM_EGL_DISPLAY;
					    			break;
				    		default:
				    				break;
	    			}
    		}
    }
}