#ifndef Included_LOG
#define Included_LOG

#include <SDL2/SDL.h>

#define LOGD(...) SDL_LogDebug(0, __VA_ARGS__)
#define LOGI(...) SDL_LogInfo(0, __VA_ARGS__)
#define LOGW(...) SDL_LogWarn(0, __VA_ARGS__)
#define LOGE(...) SDL_LogError(0, __VA_ARGS__)

#endif // Included_LOG