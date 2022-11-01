#include <jni.h>
#include <thread>
#include <mutex>
#include <native_window.h>

#include "mainListener.h"
#include "translated_opengles.h"


#define JEx(R, M) extern "C" JNIEXPORT R JNICALL Java_com_ariasaproject_technowar_AndroidApplication_##M

JEx(void, native_onCreate) (JNIEnv *e, jobject o) {
	
}

JEx(void, native_onStart) (JNIEnv *e, jobject o) {
	
}

JEx(void, native_onResume) (JNIEnv *e, jobject o) {
	
}

JEx(void, native_onPause) (JNIEnv *e, jobject o, jboolean finished) {
	
}


//surface callback
ANativeWindow n_window = nullptr;
unsigned int width = 0, height = 0;

JEx(jboolean, native_surfaceCreated) (JNIEnv *e, jobject o, jobject surface) {
	n_window = ANativeWindow_fromSurface(surface);
	return (n_window != nullptr);
}

JEx(jboolean, native_surfaceChanged) (JNIEnv *e, jobject o, jobject surface, jint w, jint h) {
	width = w;
	height = h;
	return (n_window != nullptr);
}

JEx(jboolean, native_surfaceDestroyed) (JNIEnv *e, jobject o, jobject surface) {
	
	return (n_window != nullptr);
}


JEx(void, create) (JNIEnv *e, jobject o) {
	tgf = new tgf_gles();
	Main::create();
}

JEx(void, resume) (JNIEnv *e, jobject o) {
	Main::resume();
}

JEx(void, resize) (JNIEnv *e, jobject o, jint w, jint h) {
	Main::resize(w, h);
}

JEx(void, render) (JNIEnv *e, jobject o, jfloat d) {
	Main::render(d);
}

JEx(void, pause) (JNIEnv *e, jobject o) {
	Main::pause();
}

JEx(void, destroy) (JNIEnv *e, jobject o) {
	Main::destroy();
}

JEx(jboolean, limitRenderer) (JNIEnv *e, jobject o) {
	return ((bool)strstr(tgf->renderer(), "adreno")) || ((bool)strstr(tgf->renderer(), "Adreno"));
}



