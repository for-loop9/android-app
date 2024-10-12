#ifndef IG_LOG_H
#define IG_LOG_H

#ifdef IG_ANDROID
#include <android/log.h>

#define IG_LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "iglog", __VA_ARGS__))
#endif

#endif