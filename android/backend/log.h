#ifndef Included_LOG
#define Included_LOG

#include <android/log.h>

#define LOG_TAG "TechnoWar Activity"

#define LOGD(...) ((void)__android_log_print (ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print (ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print (ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))

#ifdef _DEBUG
#include <stdio.h>
extern char error_pack[2048];
#define LOGE(...) \
    if (!error_pack[0])                              \
      snprintf (error_pack, 2048, __VA_ARGS__);         \
		((void)__android_log_print (ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#else
#define LOGE(...) ((void)__android_log_print (ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#endif // Included_LOG