#include <jni.h>
#include <thread>
#include <mutex>

#include "mainListener.h"
#include "translated_opengles.h"


#define JEx(R, M) extern "C" JNIEXPORT R JNICALL Java_com_ariasaproject_technowar_AndroidApplication_##M


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



