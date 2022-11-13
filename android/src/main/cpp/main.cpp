#include <jni.h>
#include <thread>
#include <mutex>

#include "mainListener.h"
#include "translated_opengles.h"

#define JEx(R, M) extern "C" JNIEXPORT R JNICALL Java_com_ariasaproject_technowar_AndroidApplication_##M

JEx(void, create) (JNIEnv *e, jobject o, jint w, jint h) {
	tgf = new tgf_gles();
	Main::create(w, h);
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
	delete tgf;
	tgf = 0;
}

JEx(jboolean, limitRenderer) (JNIEnv *e, jobject o) {
	const char *r = tgf->renderer();
	return ((bool)strstr(r, "adreno")) || ((bool)strstr(r, "Adreno"));
}
#include <android/native_window.h>
/*
#include <EGL/egl.h>
#include <android/native_window_jni.h>
*/
