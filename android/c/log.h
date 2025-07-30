#ifndef Included_LOG
#define Included_LOG

#include <android/log.h>

#define LOG_TAG "TechnoWar Activity"

#ifdef _DEBUG
extern void toastMessage(const char*, ...);
extern void finish();
#define LOGD(...) ((void)__android_log_print (ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print (ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print (ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)), \
    toastMessage(__VA_ARGS__)
#define LOGE(...) ((void)__android_log_print (ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)), \
    toastMessage(__VA_ARGS__), finish()
#else
#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...) ((void)__android_log_print (ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif // _DEBUG

#endif // Included_LOG