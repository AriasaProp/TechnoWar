#ifndef Included_LOG
#define Included_LOG

#include <android/log.h>
#include <assert.h>

#define LOG_TAG "TechnoWar Activity"

#define LOGD(...) ((void)__android_log_print (ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print (ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print (ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print (ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)), assert (false)

#endif // Included_LOG