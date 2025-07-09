#ifndef Included_LOG
#define Included_LOG

#include <android/log.h>
#include <stdio.h>

extern void toast_message(const char *, ...);
extern void finish_activity();

#define LOG_TAG "TechnoWar Activity"

#ifdef _DEBUG
#define LOGD(...) ((void)__android_log_print (ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)), toast_message (__VA_ARGS__)
#define LOGI(...) ((void)__android_log_print (ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)), toast_message (__VA_ARGS__)
#else
#define LOGD(...) ((void)__android_log_print (ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print (ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#endif

#define LOGW(...) ((void)__android_log_print (ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)), toast_message (__VA_ARGS__)
#define LOGE(...) ((void)__android_log_print (ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)), \
  toast_message (__VA_ARGS__), finish_activity ()

#endif // Included_LOG